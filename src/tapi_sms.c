/*
 * libslp-tapi
 *
 * Copyright (c) 2013 Samsung Electronics Co. Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @ingroup	TAPI
 * @defgroup	SMS
 * @{
 *
 * SMS APIs allow a client to accomplish the following features: @n
 * - Send, Recieve, Save, Delete, Read short messages  @n
 * - Set and Get information like Service Center Address, Cell Broadcast configuration,Preferred Bearer, SMS parameters @n
 * - Retrieve information like Current Memory selected, NetTextCount related to the messages @n
 * - Set delivery report @n
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tapi_common.h"
#include "TapiUtility.h"
#include "TelSms.h"

#include "common.h"
#include "tapi_log.h"
#include "ITapiNetText.h"

#include <tapi_sms.h>
#include <tel_return.h>

/* Utilities */
static void _hex_dump(char *pad, int size, const void *data)
{
	char buf[255] = {0, };
	char hex[4] = {0, };
	int i;
	unsigned char *p;

	if (size <= 0) {
		msg("%sno data", pad);
		return;
	}

	p = (unsigned char *)data;
	snprintf(buf, 255, "%s%04X: ", pad, 0);
	for (i = 0; i<size; i++) {
		snprintf(hex, 4, "%02X ", p[i]);
		strcat(buf, hex);

		if ((i + 1) % 8 == 0) {
			if ((i + 1) % 16 == 0) {
				msg("%s", buf);
				memset(buf, 0, 255);
				snprintf(buf, 255, "%s%04X: ", pad, i + 1);
			} else {
				strcat(buf, "  ");
			}
		}
	}

	msg("%s", buf);
}

static TelSmsResponse_t __map_sms_result (int x)
{
	switch(x) {
		case TEL_SMS_RESULT_SUCCESS :
			return TAPI_NETTEXT_SENDSMS_SUCCESS;
		case TEL_SMS_RESULT_INVALID_PARAMETER:
			return TAPI_NETTEXT_INVALID_PARAMETER;
		case TEL_SMS_RESULT_INVALID_MANDATORY_INFO :
			return TAPI_NETTEXT_INVALID_MANDATORY_INFO;
		case TEL_SMS_RESULT_NETWORK_FAILURE :
			return TAPI_NETTEXT_NETWORK_OUTOFORDER;
		case TEL_SMS_RESULT_NETWORK_CONGESTION :
			return TAPI_NETTEXT_CONGESTION;
		case TEL_SMS_RESULT_OPERATION_NOT_SUPPORTED:
			return TAPI_NETTEXT_FACILITY_NOT_IMPLEMENTED;
		default :
			return TAPI_NETTEXT_DEVICE_FAILURE;
	}
}

static gboolean __sms_hexstring_to_asciistring(const char *hex_string, unsigned char *ascii_string, unsigned int hex_len)
{
	unsigned int i, ascii_len;

	if (ascii_string == NULL || hex_string == NULL)
		return FALSE;

	if (hex_string[hex_len - 1] >> 4 == 0x0f) {
		ascii_len = (hex_len * 2) - 1;
	} else {
		ascii_len = (hex_len * 2);
	}

	for (i = 0; i < ascii_len; i++) {
		if (i % 2 == 0) {
			ascii_string[i] = (hex_string[i/2] & 0x0f) + '0';
		} else {
			ascii_string[i] = ((hex_string[i/2] >> 4 )) + '0';
		}
	}
	ascii_string[ascii_len] = '\0';

	return TRUE;
}

static char __sms_hexchar_to_int(char ch)
{
	if (ch >= '0' && ch <= '9')
		return (ch - '0');
	else if (ch >= 'A' && ch <= 'F')
		return (ch - 'A' + 10);
	else if (ch >= 'a' && ch <= 'f')
		return (ch - 'a' + 10);
	else {
		err("Invalid Character");
		return -1;
	}
}

static gboolean __sms_asciistring_to_hex(const char *ascii_string, unsigned char *hex_string, unsigned int ascii_len, unsigned char *hex_len)
{
	unsigned int i;

	if (ascii_string == NULL || hex_string == NULL)
		return FALSE;

	if (ascii_len % 2 == 0) { /* Make sure ASCII len is even */
		*hex_len = ascii_len/2;
		for(i = 0; i < ascii_len; i += 2) {
			hex_string[i/2] =
				(char)(((__sms_hexchar_to_int(ascii_string[i + 1]) << 4) |
				__sms_hexchar_to_int(ascii_string[i])));
		}
	} else { /* Odd ASCII len */
		*hex_len = ascii_len/2 + 1;
		for (i = 0; i < ascii_len; i += 2) {
			if (i == ascii_len - 1) {
				hex_string[i / 2] = (char)(0xF0 | __sms_hexchar_to_int(ascii_string[i]));
			} else {
				hex_string[i / 2] = (char)(((__sms_hexchar_to_int(ascii_string[i + 1]) << 4) |
						__sms_hexchar_to_int(ascii_string[i])));
			}
		}
	}
	return TRUE;
}

static void on_response_default(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSmsResponse_t res =__map_sms_result(result);

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

static void on_response_read_msg(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;


	TelSmsResponse_t res =__map_sms_result(result);

	if (res == TAPI_NETTEXT_SENDSMS_SUCCESS) {
		TelSmsData_t resp;

		TelSmsSimDataInfo *resp_buf = (TelSmsSimDataInfo *)data;

		resp.MsgStatus = resp_buf->status;
		resp.SmsData.MsgLength = resp_buf->data.tpdu_length;
		if (resp_buf->data.tpdu[0])
			memcpy((gchar *)resp.SmsData.szData,
				(gchar *)resp_buf->data.tpdu, TAPI_NETTEXT_SMDATA_SIZE_MAX);

		resp.SmsData.Sca[0] = strlen(resp_buf->data.sca.number);
		if (resp.SmsData.Sca[0] != 0) {
			unsigned char sca_length = 0;
			resp.SmsData.Sca[1] =
				((resp_buf->data.sca.ton << 4) & 0xF0) | (resp_buf->data.sca.npi & 0x0F);

			if (__sms_asciistring_to_hex(
					(const char *)resp_buf->data.sca.number,
					(unsigned char *)&resp.SmsData.Sca[2],
					resp.SmsData.Sca[0],
					&sca_length) != TRUE) {
				err("sca received is null");
				goto failure;
			}
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
		g_free(evt_cb_data);
		return;
	}
failure :
	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_write_msg(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSmsResponse_t res =__map_sms_result(result);
	gint *index = NULL;

	if (res == TAPI_NETTEXT_SENDSMS_SUCCESS) {
		index = (int *)data;
	}
	if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, index, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

static void on_response_delete_msg(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	gint *index = NULL;

	TelSmsResponse_t res =__map_sms_result(result);

	if (res == TAPI_NETTEXT_SENDSMS_SUCCESS) {
		index = (int *)evt_cb_data->legacy_user_data;
	}
	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, index, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

static void on_response_get_msg_count(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSmsResponse_t res =__map_sms_result(result);

	if (res == TAPI_NETTEXT_SENDSMS_SUCCESS) {
		TelSmsStoredMsgCountInfo_t resp;
		unsigned int i;

		TelSmsStoredMsgCountInfo *resp_buf = (TelSmsStoredMsgCountInfo *)data;

		resp.TotalCount = resp_buf->total_count;
		resp.UsedCount = resp_buf->used_count;
		for (i = 0; i < resp.TotalCount; i++) {
			resp.IndexList[i] = resp_buf->index_list[i];
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_get_sca(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSmsResponse_t res =__map_sms_result(result);

	if (res == TAPI_NETTEXT_SENDSMS_SUCCESS) {
		TelSmsAddressInfo_t resp;
		TelSmsSca *resp_buf = (TelSmsSca *)data;

		resp.Npi = (TelSimNumberingPlanIdentity_t)resp_buf->npi;
		resp.Ton = (TelSimTypeOfNum_t)resp_buf->ton;
		dbg("SCA number [%s] len [%d]", resp_buf->number, strlen(resp_buf->number));
		if (resp_buf->number[0] != '\0') {
			char *num;
			if (resp_buf->number[0] == '+') {
				/* copy '+' into first byte */
				num = &(resp_buf->number[1]);
			} else
				num = &(resp_buf->number[0]);

			if (__sms_asciistring_to_hex((const char *)num,
				(unsigned char *)&(resp.szDiallingNum[1]),
				strlen(num),
				(unsigned char *)&resp.DialNumLen) != TRUE) {
				err("sca received is null");
				goto failure;
			}

			/* First byte should be sca length */
			resp.szDiallingNum[0] = strlen(num);

			/* DialNumLen should be length of sca number  + length of first byte */
			resp.DialNumLen++;
			_hex_dump("    ", resp.DialNumLen, resp.szDiallingNum);
		}

		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
		g_free(evt_cb_data);

		return;
	}
failure :
	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_get_cb_config(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSmsResponse_t res =__map_sms_result(result);

	if (res == TAPI_NETTEXT_SENDSMS_SUCCESS) {
		TelSmsCbConfig_t resp;

		TelSmsCbConfigInfo *resp_buf = (TelSmsCbConfigInfo *)data;

		resp.CBEnabled = resp_buf->cb_enabled;
		resp.MsgIdRangeCount = resp_buf->msg_id_range_cnt;
		if (resp_buf->msg_ids)
			memcpy(resp.MsgIDs, resp_buf->msg_ids, TEL_SMS_GSM_CBMI_LIST_SIZE_MAX + 1);
		resp.Net3gppType = 1; /*Value removed in 3.0. Assigning default. */
		resp.MsgIdMaxCount = '\0';
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_get_sms_params(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSmsResponse_t res =__map_sms_result(result);

	if (res == TAPI_NETTEXT_SENDSMS_SUCCESS) {
		TelSmsParams_t resp = {0, };
		TelSmsParamsInfoList *resp_buf = (TelSmsParamsInfoList *)data;

		if (resp_buf->count) {
			resp.RecordIndex = resp_buf->params->index;
			resp.TpValidityPeriod = resp_buf->params->vp;

			dbg("SCA number [%s] len [%d]", resp_buf->params->sca.number,
				strlen(resp_buf->params->sca.number));

			if (resp_buf->params->sca.number[0] != '\0') {
				resp.TpSvcCntrAddr.Npi = resp_buf->params->sca.npi;
				resp.TpSvcCntrAddr.Ton = resp_buf->params->sca.ton;
				if (__sms_asciistring_to_hex((const char *)resp_buf->params->sca.number,
						(unsigned char *)resp.TpSvcCntrAddr.szDiallingNum,
						strlen(resp_buf->params->sca.number),
						(unsigned char *)&resp.TpSvcCntrAddr.DialNumLen) != TRUE) {
					err("sca received is null");
					goto failure;
				}
			}
		}

		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
		g_free(evt_cb_data);
		return;
	}
failure :
	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_get_sms_params_count (TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSmsResponse_t res =__map_sms_result(result);

	if (res == TAPI_NETTEXT_SENDSMS_SUCCESS) {
		TelSmsParamsInfoList *resp_buf = (TelSmsParamsInfoList *)data;
		int resp = (int)resp_buf->count;

		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}
	g_free(evt_cb_data);
}

EXPORT_API int tel_send_sms(TapiHandle *handle,
		const TelSmsDatapackageInfo_t *pDataPackage,
		int bMoreMsgToSend,
		tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSmsSendInfo info_buf = {0,};

	dbg("Func Entrance ");

	info_buf.more_msgs = ((bMoreMsgToSend) ? TRUE : FALSE);
	if (pDataPackage->Sca[0] != 0) {
		unsigned int hex_length = ((pDataPackage->Sca[0]) + 1)/2;

		info_buf.send_data.sca.ton = (pDataPackage->Sca[1] & 0x70) >> 4;
		info_buf.send_data.sca.npi = pDataPackage->Sca[1] & 0x0F;

		if (__sms_hexstring_to_asciistring(
				(const char *)&pDataPackage->Sca[2],
				(unsigned char *)&info_buf.send_data.sca.number[0],
				hex_length) != TRUE) {
			err("Invalid input");
			return TAPI_API_INVALID_INPUT;
		}
	}
	if (pDataPackage->szData[0])
		memcpy((gchar *)info_buf.send_data.tpdu,
			(gchar *)pDataPackage->szData,
			TEL_SMS_SMDATA_SIZE_MAX);

	info_buf.send_data.tpdu_length = pDataPackage->MsgLength;

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_send((TelHandle *)handle, &info_buf, on_response_default, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_read_sms_in_sim(TapiHandle *handle, int index, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_read_sms_in_sim((TelHandle *)handle, (guint)index, on_response_read_msg, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_write_sms_in_sim(TapiHandle *handle, const TelSmsData_t *pWriteData, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSmsSimDataInfo info_buf = {0,};

	dbg("Func Entrance ");

	info_buf.status = pWriteData->MsgStatus;
	if (pWriteData->SmsData.Sca[0] != 0) {
		unsigned int hex_length = ((pWriteData->SmsData.Sca[0]) + 1) / 2;

		info_buf.data.sca.ton = (pWriteData->SmsData.Sca[1] & 0x70) >> 4;
		info_buf.data.sca.npi = pWriteData->SmsData.Sca[1] & 0x0F;

		if (__sms_hexstring_to_asciistring((const char *)&pWriteData->SmsData.Sca[2],
				(unsigned char *)info_buf.data.sca.number,
				hex_length) != TRUE) {
			err("Invalid input");
			return TAPI_API_INVALID_INPUT;
		}
	}
	if (pWriteData->SmsData.szData[0])
		g_strlcpy((gchar *)info_buf.data.tpdu,
			(gchar *)pWriteData->SmsData.szData,
			TEL_SMS_SMDATA_SIZE_MAX + 1);
	info_buf.data.tpdu_length = pWriteData->SmsData.MsgLength;

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_write_sms_in_sim((TelHandle *)handle, &info_buf, on_response_write_msg, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_delete_sms_in_sim(TapiHandle *handle, int index, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, &index, sizeof(index));

	ret = tapi_sms_delete_sms_in_sim((TelHandle *)handle, (guint)index, on_response_delete_msg, (void *)cb_data);

	CHECK_RETURN (ret, cb_data);
}

EXPORT_API int tel_get_sms_count(TapiHandle *handle, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_get_count((TelHandle *)handle, on_response_get_msg_count, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sms_sca(TapiHandle *handle, int index, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_get_sca((TelHandle *)handle, on_response_get_sca, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sms_sca(TapiHandle *handle, const TelSmsAddressInfo_t *pSCA, int index, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSmsSca info_buf = {0,};

	dbg("Func Entrance ");

	if (pSCA->DialNumLen != 0) {
		info_buf.npi = (unsigned char)pSCA->Npi;
		info_buf.ton = (unsigned char)pSCA->Ton;

		if (__sms_hexstring_to_asciistring((const char *)pSCA->szDiallingNum, (unsigned char *)info_buf.number,
				pSCA->DialNumLen) != TRUE) {
			err("Invalid input");
			return TAPI_API_INVALID_INPUT;
		}
	}

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_set_sca((TelHandle *)handle, &info_buf, on_response_default, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sms_cb_config(TapiHandle *handle, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_get_cb_config((TelHandle *)handle, on_response_get_cb_config, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sms_cb_config(TapiHandle *handle, const TelSmsCbConfig_t *pCBConfig, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSmsCbConfigInfo info_buf = {0,};

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	info_buf.cb_enabled = ((pCBConfig->CBEnabled) ? TRUE : FALSE);
	info_buf.msg_id_range_cnt = pCBConfig->MsgIdRangeCount;
	if (pCBConfig->MsgIDs)
		memcpy(info_buf.msg_ids, pCBConfig->MsgIDs, TEL_SMS_GSM_CBMI_LIST_SIZE_MAX + 1);

	ret = tapi_sms_set_cb_config((TelHandle *)handle, &info_buf, on_response_get_cb_config, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sms_memory_status(TapiHandle *handle, int memoryStatus, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_set_memory_status((TelHandle *)handle, (gboolean) memoryStatus, on_response_default, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sms_preferred_bearer(TapiHandle *handle, tapi_response_cb callback, void* user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
	/*Functionality not supported by TAPI 3.0*/
}

EXPORT_API int tel_set_sms_preferred_bearer(TapiHandle *handle, TelSmsBearerType_t BearerType, tapi_response_cb callback, void* user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
	/*Functionality not supported by TAPI 3.0*/
}


EXPORT_API int tel_send_sms_deliver_report(TapiHandle *handle,
		const TelSmsDatapackageInfo_t *pDataPackage,
		TelSmsResponse_t RPCause,
		tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSmsDeliverReportInfo info_buf = {0,};

	dbg("Func Entrance ");

	switch (RPCause) {
		case TAPI_NETTEXT_SENDSMS_SUCCESS :
			info_buf.report = TEL_SMS_DELIVERY_REPORT_SUCCESS;
		case TAPI_NETTEXT_SIM_FULL :
		case TAPI_NETTEXT_ME_FULL :
		case TAPI_NETTEXT_MEMORY_CAPACITY_EXCEEDED :
			info_buf.report = TEL_SMS_DELIVERY_REPORT_MEMORY_ERROR;
		default :
			info_buf.report = TEL_SMS_DELIVERY_REPORT_UNSPECIFIED_ERROR;
	}

	if (pDataPackage->Sca[0] != 0) {
		unsigned int hex_length = ((pDataPackage->Sca[0]) + 1) / 2;

		info_buf.data.sca.ton = (pDataPackage->Sca[1] & 0x70) >> 4;
		info_buf.data.sca.npi = pDataPackage->Sca[1] & 0x0F;

		if (__sms_hexstring_to_asciistring((const char *)&pDataPackage->Sca[2], (unsigned char *)info_buf.data.sca.number,
			hex_length) != TRUE) {
			err("Invalid input");
			return TAPI_API_INVALID_INPUT;
		}
	}
	info_buf.data.tpdu_length = pDataPackage->MsgLength;
	if (pDataPackage->szData[0])
		g_strlcpy((gchar *)info_buf.data.tpdu, (gchar *)pDataPackage->szData, pDataPackage->MsgLength + 1);

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_send_deliver_report((TelHandle *)handle, &info_buf, on_response_default, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sms_message_status(TapiHandle *handle, int index, TelSmsMsgStatus_t msgStatus, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSmsStatusInfo info_buf = {0, };

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	info_buf.index = (unsigned int)index;
	info_buf.status = msgStatus;

	ret = tapi_sms_set_message_status((TelHandle *)handle, &info_buf, on_response_default, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sms_parameters(TapiHandle *handle, int index, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_get_parameters((TelHandle *)handle, on_response_get_sms_params, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sms_parameters(TapiHandle *handle, const TelSmsParams_t *pSmsSetParameters, tapi_response_cb cb, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSmsParamsInfo info_buf = {0, };

	dbg("Func Entrance ");

	info_buf.index = (unsigned int)pSmsSetParameters->RecordIndex;
	info_buf.vp = pSmsSetParameters->TpValidityPeriod;
	if (pSmsSetParameters->TpSvcCntrAddr.DialNumLen != 0) {
		info_buf.sca.npi = pSmsSetParameters->TpSvcCntrAddr.Npi;
		info_buf.sca.ton = pSmsSetParameters->TpSvcCntrAddr.Ton;

		if (__sms_hexstring_to_asciistring(
				(const char *)pSmsSetParameters->TpSvcCntrAddr.szDiallingNum,
				(unsigned char *)info_buf.sca.number,
				pSmsSetParameters->TpSvcCntrAddr.DialNumLen) != TRUE) {
			err("Invalid input");
			return TAPI_API_INVALID_INPUT;
		}
	}

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sms_set_parameters((TelHandle *)handle, &info_buf, on_response_get_sms_params, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sms_parameter_count(TapiHandle *handle, tapi_response_cb callback, void* user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_sms_get_parameters((TelHandle *)handle, on_response_get_sms_params_count, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_check_sms_device_status(TapiHandle *handle, int *pReadyStatus)
{
	TelReturn ret;
	gboolean info_buf;

	dbg("Func Entrance");

	ret = tapi_sms_get_init_status((TelHandle *)handle, &info_buf);

	*pReadyStatus = (int)info_buf;

	CHECK_RETURN(ret, NULL);
}

/*EOF*/
