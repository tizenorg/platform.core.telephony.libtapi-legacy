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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tapi_common.h"
#include "TapiUtility.h"
#include "TelSs.h"

#include "common.h"
#include "tapi_log.h"
#include "ITapiSs.h"

#include <tel_ss.h>
#include <tapi_ss.h>
#include <tel_return.h>

#define MAP_SS_BARRING_TYPE(src, dest) \
{ \
	switch (src) { \
		case TAPI_SS_CB_TYPE_BAOC : \
			dest = TEL_SS_CB_TYPE_BAOC; \
			break; \
		case TAPI_SS_CB_TYPE_BOIC : \
			dest = TEL_SS_CB_TYPE_BOIC; \
			break; \
		case TAPI_SS_CB_TYPE_BOIC_NOT_HC : \
			dest = TEL_SS_CB_TYPE_BOIC_NOT_HC; \
			break; \
		case TAPI_SS_CB_TYPE_BAIC : \
			dest = TEL_SS_CB_TYPE_BAIC; \
			break; \
		case TAPI_SS_CB_TYPE_BIC_ROAM : \
			dest = TEL_SS_CB_TYPE_BIC_ROAM; \
			break; \
		case TAPI_SS_CB_TYPE_AB : \
			dest = TEL_SS_CB_TYPE_AB; \
			break; \
		case TAPI_SS_CB_TYPE_AOB : \
			dest = TEL_SS_CB_TYPE_AOB; \
			break; \
		case TAPI_SS_CB_TYPE_BIC_NOT_SIM : \
			dest = TEL_SS_CB_TYPE_NS; \
			break; \
		case TAPI_SS_CB_TYPE_AIB: \
			dest = TEL_SS_CB_TYPE_AIB; \
			break; \
		default : \
			dest = 9999; /*To handle enums that have been removed in 3.0.*/ \
	} \
}

#define MAP_SS_BARRING_TYPE_RESP(src, dest) \
{ \
	switch (src) { \
		case TEL_SS_CB_TYPE_BAOC : \
			dest = TAPI_SS_CB_TYPE_BAOC; \
			break; \
		case TEL_SS_CB_TYPE_BOIC : \
			dest = TAPI_SS_CB_TYPE_BOIC; \
			break; \
		case TEL_SS_CB_TYPE_BOIC_NOT_HC : \
			dest = TAPI_SS_CB_TYPE_BOIC_NOT_HC; \
			break; \
		case TEL_SS_CB_TYPE_BAIC : \
			dest = TAPI_SS_CB_TYPE_BAIC; \
			break; \
		case TEL_SS_CB_TYPE_BIC_ROAM : \
			dest = TAPI_SS_CB_TYPE_BIC_ROAM; \
			break; \
		case TEL_SS_CB_TYPE_AB : \
			dest = TAPI_SS_CB_TYPE_AB; \
			break; \
		case TEL_SS_CB_TYPE_AOB : \
			dest = TAPI_SS_CB_TYPE_AOB; \
			break; \
		case TEL_SS_CB_TYPE_NS : \
			dest = TAPI_SS_CB_TYPE_BIC_NOT_SIM; \
			break; \
		case TEL_SS_CB_TYPE_AIB: \
			dest = TAPI_SS_CB_TYPE_AIB; \
			break; \
	} \
}

#define MAP_SS_FWD_CONDITION(src, dest) \
{ \
	switch(src) { \
		case TAPI_SS_CF_WHEN_CFU: \
			dest = TEL_SS_CF_COND_CFU; \
			break; \
		case TAPI_SS_CF_WHEN_CFB: \
			dest = TEL_SS_CF_COND_CFB; \
			break; \
		case TAPI_SS_CF_WHEN_CFNRy: \
			dest = TEL_SS_CF_COND_CFNRY; \
			break; \
		case TAPI_SS_CF_WHEN_CFNRc: \
			dest = TEL_SS_CF_COND_CFNRC; \
			break; \
		case TAPI_SS_CF_WHEN_CFC: \
			dest = TEL_SS_CF_COND_ALL_CFC; \
			break; \
		case TAPI_SS_CF_WHEN_CF_ALL: \
			dest = TEL_SS_CF_COND_ALL; \
		default : \
			dest = 9999; /*To handle enums that have been removed in 3.0.*/ \
	} \
}

#define MAP_SS_FWD_CONDITION_RESP(src, dest) \
{ \
	switch(src) { \
		case TEL_SS_CF_COND_CFU: \
			dest = TAPI_SS_CF_WHEN_CFU; \
			break; \
		case TEL_SS_CF_COND_CFB: \
			dest = TAPI_SS_CF_WHEN_CFB; \
			break; \
		case TEL_SS_CF_COND_CFNRY: \
			dest = TAPI_SS_CF_WHEN_CFNRy; \
			break; \
		case TEL_SS_CF_COND_CFNRC: \
			dest = TAPI_SS_CF_WHEN_CFNRc; \
			break; \
		case TEL_SS_CF_COND_ALL_CFC: \
			dest = TAPI_SS_CF_WHEN_CFC; \
			break; \
		case TEL_SS_CF_COND_ALL: \
			dest = TAPI_SS_CF_WHEN_CF_ALL; \
			break; \
	} \
}

#define MAP_SS_USSD_TYPE(src, dest) \
{ \
	switch(src) { \
		case TAPI_SS_USSD_TYPE_USER_INIT: \
			dest = TEL_SS_USSD_TYPE_USER_INIT; \
			break; \
		case TAPI_SS_USSD_TYPE_USER_RSP: \
			dest = TEL_SS_USSD_TYPE_USER_RSP; \
			break; \
		case TAPI_SS_USSD_TYPE_USER_REL: \
			dest = TEL_SS_USSD_TYPE_USER_REL; \
			break; \
	} \
}

#define MAP_SS_USSD_TYPE_RESP(src, dest) \
{ \
	switch(src) { \
		case TEL_SS_USSD_TYPE_USER_INIT: \
			dest = TAPI_SS_USSD_TYPE_USER_INIT; \
			break; \
		case TEL_SS_USSD_TYPE_USER_RSP: \
			dest = TAPI_SS_USSD_TYPE_USER_RSP; \
			break; \
		case TEL_SS_USSD_TYPE_USER_REL: \
			dest = TAPI_SS_USSD_TYPE_USER_REL; \
			break; \
	} \
}

#define MAP_SS_USSD_STATUS_RESP(src, dest) \
{ \
	switch(src) { \
		case TEL_SS_USSD_STATUS_NO_ACTION_REQUIRED: \
			dest = TAPI_SS_USSD_STATUS_NO_ACTION_REQUIRE; \
			break; \
		case TEL_SS_USSD_STATUS_ACTION_REQUIRED: \
			dest = TAPI_SS_USSD_STATUS_ACTION_REQUIRE; \
			break; \
		case TEL_SS_USSD_STATUS_TERMINATED_BY_NETWORK: \
			dest = TAPI_SS_USSD_STATUS_TERMINATED_BY_NET; \
			break; \
		case TEL_SS_USSD_STATUS_OTHER_CLIENT: \
			dest = TAPI_SS_USSD_STATUS_OTHER_CLIENT; \
			break; \
		case TEL_SS_USSD_STATUS_TIME_OUT: \
			dest = TAPI_SS_USSD_STATUS_TIME_OUT; \
			break; \
		default : \
			dest = TAPI_SS_USSD_STATUS_NOT_SUPPORT; \
	} \
}

static TelSsCause_t __map_ss_result(int val)
{
	switch(val) {
		case TEL_SS_RESULT_SUCCESS :
			return TAPI_SS_SUCCESS;
		case TEL_SS_RESULT_INVALID_PARAMETER:
			return TAPI_SS_UNKNOWNALPHABET;
		case TEL_SS_RESULT_OPERATION_NOT_SUPPORTED :
			return TAPI_SS_ILLEGALSSOPERATION;
		case TEL_SS_RESULT_SERVICE_NOT_AVAILABLE :
			return TAPI_SS_NOTAVAILABLE;
		case TEL_SS_RESULT_OPERATION_NOT_ALLOWED :
			return TAPI_SS_ILLEGALSSOPERATION;
		case TEL_SS_RESULT_INVALID_PASSWORD :
			return TAPI_SS_NEGATIVEPWCHECK;
		case TEL_SS_RESULT_PASSWORD_BLOCKED :
			return TAPI_SS_NUMBEROFPWATTEMPTSVIOLATION;
		default :
			return TAPI_SS_UNKNOWNERROR;
	}
}

static TelSsClass __map_ss_class( TelSsClass_t src)
{
	switch (src) {
		case TAPI_SS_CLASS_ALL_TELE :
			return TEL_SS_CLASS_ALL_TELE;
		case TAPI_SS_CLASS_VOICE :
			return TEL_SS_CLASS_VOICE;
		case TAPI_SS_CLASS_ALL_DATA_TELE :
			return TEL_SS_CLASS_ALL_DATA_TELE;
		case TAPI_SS_CLASS_FAX :
			return TEL_SS_CLASS_FAX;
		case TAPI_SS_CLASS_SMS :
			return TEL_SS_CLASS_SMS;
		case TAPI_SS_CLASS_VGCS :
			return TEL_SS_CLASS_VGCS;
		case TAPI_SS_CLASS_VBS :
			return TEL_SS_CLASS_VBS;
		case TAPI_SS_CLASS_ALL_TELE_EXPT_SMS :
			return TEL_SS_CLASS_ALL_TELE_EXPT_SMS;
		case TAPI_SS_CLASS_ALL_BEARER :
			return TEL_SS_CLASS_ALL_BEARER;
		case TAPI_SS_CLASS_ALL_ASYNC :
			return TEL_SS_CLASS_ALL_ASYNC;
		case TAPI_SS_CLASS_ALL_SYNC :
			return TEL_SS_CLASS_ALL_SYNC;
		case TAPI_SS_CLASS_ALL_CS_SYNC :
			return TEL_SS_CLASS_ALL_CS_SYNC;
		case TAPI_SS_CLASS_ALL_CS_ASYNC :
			return TEL_SS_CLASS_ALL_CS_ASYNC;
		case TAPI_SS_CLASS_ALL_DEDI_PS :
			return TEL_SS_CLASS_ALL_DEDI_PS;
		case TAPI_SS_CLASS_ALL_DEDI_PAD :
			return TEL_SS_CLASS_ALL_DEDI_PAD;
		case TAPI_SS_CLASS_ALL_DATA_CDA :
			return TEL_SS_CLASS_ALL_DATA_CDA;
		case TAPI_SS_CLASS_AUX_VOICE :
			return TEL_SS_CLASS_AUX_VOICE;
		case TAPI_SS_CLASS_ALL_GPRS_BEARER :
			return TEL_SS_CLASS_ALL_GPRS_BEARER;
		case TAPI_SS_CLASS_ALL_TELE_BEARER:
			return TEL_SS_CLASS_ALL_TELE_BEARER;
		default :
			return TEL_SS_CLASS_ALL_TELE;
	}
}

static TelSsClass_t __map_ss_class_resp (TelSsClass src)
{
	switch (src) {
		case TEL_SS_CLASS_ALL_TELE:
			return TAPI_SS_CLASS_ALL_TELE;
		case TEL_SS_CLASS_VOICE :
			return TAPI_SS_CLASS_VOICE;
		case TEL_SS_CLASS_ALL_DATA_TELE :
			return TAPI_SS_CLASS_ALL_DATA_TELE;
		case TEL_SS_CLASS_FAX :
			return TAPI_SS_CLASS_FAX;
		case TEL_SS_CLASS_SMS :
			return TAPI_SS_CLASS_SMS;
		case TEL_SS_CLASS_VGCS :
			return TAPI_SS_CLASS_VGCS;
		case TEL_SS_CLASS_VBS :
			return TAPI_SS_CLASS_VBS;
		case TEL_SS_CLASS_ALL_TELE_EXPT_SMS :
			return TAPI_SS_CLASS_ALL_TELE_EXPT_SMS;
		case TEL_SS_CLASS_ALL_BEARER :
			return TAPI_SS_CLASS_ALL_BEARER;
		case TEL_SS_CLASS_ALL_ASYNC :
			return TAPI_SS_CLASS_ALL_ASYNC;
		case TEL_SS_CLASS_ALL_SYNC :
			return TAPI_SS_CLASS_ALL_SYNC;
		case TEL_SS_CLASS_ALL_CS_SYNC :
			return TAPI_SS_CLASS_ALL_CS_SYNC;
		case TEL_SS_CLASS_ALL_CS_ASYNC :
			return TAPI_SS_CLASS_ALL_CS_ASYNC;
		case TEL_SS_CLASS_ALL_DEDI_PS :
			return TAPI_SS_CLASS_ALL_DEDI_PS;
		case TEL_SS_CLASS_ALL_DEDI_PAD :
			return TAPI_SS_CLASS_ALL_DEDI_PAD;
		case TEL_SS_CLASS_ALL_DATA_CDA :
			return TAPI_SS_CLASS_ALL_DATA_CDA;
		case TEL_SS_CLASS_AUX_VOICE :
			return TAPI_SS_CLASS_AUX_VOICE;
		case TEL_SS_CLASS_ALL_GPRS_BEARER :
			return TAPI_SS_CLASS_ALL_GPRS_BEARER;
		case TEL_SS_CLASS_ALL_TELE_BEARER:
			return TAPI_SS_CLASS_ALL_TELE_BEARER;
		default :
			return TAPI_SS_CLASS_ALL_TELE;
	}
}

static TelSsCliStatus_t __map_ss_cli_status_resp(TelSsCliNetworkStatus net_status, TelSsCliDeviceStatus dev_status)
{
	switch (net_status) {
	case TEL_SS_CLI_PROVISIONED :
		if (dev_status == TEL_SS_CLI_ENABLE) {
			return TAPI_CLI_STATUS_ACTIVATED;
		} else {
			return TAPI_CLI_STATUS_PROVISIONED;
		}
	case TEL_SS_CLI_NOT_PROVISIONED :
		return TAPI_CLI_STATUS_NOT_PROVISONED;
	default :
		return TAPI_CLI_STATUS_UNKOWN;
	}
}

static TelSsCliStatus_t __map_ss_clir_status_resp(TelSsClirNetworkStatus net_status, TelSsClirDeviceStatus dev_status)
{
	switch(net_status) {
		case TEL_CLIR_STATUS_PROVISIONED:
			if (dev_status == TEL_CLIR_STATUS_INVOCATION) {
				return TAPI_CLI_STATUS_ACTIVATED;
			} else {
				return TAPI_CLI_STATUS_PROVISIONED;
			}
		case TEL_CLIR_STATUS_NOT_PROVISIONED :
			return TAPI_CLI_STATUS_NOT_PROVISONED;
		case TEL_CLIR_STATUS_TEMP_RESTRICTED:
			return TAPI_CLI_STATUS_TEMP_RESTRICTED;
		case TEL_CLIR_STATUS_TEMP_ALLOWED:
			return TAPI_CLI_STATUS_TEMP_ALLOWED;
		default :
			return TAPI_CLI_STATUS_UNKOWN;
	}
}

static void on_response_get_ss_barring_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (res == TAPI_SS_SUCCESS) {
		int i;
		TelSsBarringResp_t resp;
		TelSsBarringResp *resp_buf =  (TelSsBarringResp *)data;

		resp.record_num = (int) resp_buf->record_num;
		for(i = 0; i < resp.record_num; i++) {

			resp.record[i].Class = __map_ss_class_resp(resp_buf->records[i].class);
			MAP_SS_BARRING_TYPE_RESP(resp_buf->records[i].type, resp.record[i].Flavour);
			resp.record[i].Status = (TelSsStatus_t)((resp_buf->records[i].enable) ? TAPI_SS_STATUS_ACTIVE : TAPI_SS_STATUS_QUIESCENT);
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data->legacy_user_data);
	g_free (evt_cb_data);
}

static void on_response_set_ss_barring_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (res == TAPI_SS_SUCCESS) {
		TelSsBarringInfo_t *resp_buf = (TelSsBarringInfo_t *)evt_cb_data->legacy_user_data;
		TelSsBarringResp_t resp;
		resp.record_num = 1;
		resp.record[0].Class = resp_buf->Class;
		resp.record[0].Flavour = resp_buf->Type;
		if (resp_buf->Mode == TAPI_SS_CB_MODE_ACTIVATE)
			resp.record[0].Status = TAPI_SS_STATUS_ACTIVE;
		else
			resp.record[0].Status = TAPI_SS_STATUS_QUIESCENT;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
			}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_change_ss_barring_password(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

static void on_response_get_ss_forward_status(TelHandle *handle, int result,  void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (res == TAPI_SS_SUCCESS) {
		int i;
		TelSsForwardResp_t resp;
		TelSsForwardingResp *resp_buf = (TelSsForwardingResp *)data;

		resp.record_num = (int)resp_buf->record_num;
		for(i = 0; i < resp.record_num; i++) {
			resp.record[i].Class = __map_ss_class_resp(resp_buf->records[i].class);
			resp.record[i].Status = ((resp_buf->records[i].enable) ? TAPI_SS_STATUS_ACTIVE : TAPI_SS_STATUS_QUIESCENT);
			MAP_SS_FWD_CONDITION_RESP(resp_buf->records[i].condition, resp.record[i].ForwardCondition);
			if (resp_buf->records[i].number[0]) {
				g_strlcpy((gchar *)resp.record[i].szCallForwardingNumber, resp_buf->records[i].number, TEL_SS_NUMBER_LEN_MAX + 1);
				resp.record[i].bCallForwardingNumberPresent = 1;
			} else {
				resp.record[i].bCallForwardingNumberPresent =0;
			}
			resp.record[i].NoReplyWaitTime = resp_buf->records[i].wait_time;
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free (evt_cb_data);
}

static void on_response_set_ss_forward_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (res == TAPI_SS_SUCCESS) {
		TelSsForwardInfo_t *resp_buf = (TelSsForwardInfo_t *)evt_cb_data->legacy_user_data;
		TelSsForwardResp_t resp;

		resp.record_num = 1;
		resp.record[0].Class = resp_buf->Class;
		resp.record[0].ForwardCondition = resp_buf->Condition;
		if (resp_buf->szPhoneNumber[0]) {
			g_strlcpy((gchar *)resp.record[0].szCallForwardingNumber, resp_buf->szPhoneNumber, TAPI_CALL_DIALDIGIT_LEN_MAX + 1);
			resp.record[0].bCallForwardingNumberPresent = 1;
		} else {
			resp.record[0].bCallForwardingNumberPresent = 0;
		}
		resp.record[0].NoReplyWaitTime = (unsigned char)resp_buf->NoReplyConditionTimer;

		if (resp_buf->Mode == TAPI_SS_CF_MODE_ENABLE_EV || resp_buf->Mode == TAPI_SS_CF_MODE_REGISTRATION_EV) {
			resp.record[0].Status = TAPI_SS_STATUS_ACTIVE;
		} else if (resp_buf->Mode == TAPI_SS_CF_MODE_ERASURE_EV) {
			resp.record[0].Status = TAPI_SS_STATUS_PROVISIONED;
		} else {
			resp.record[0].Status = TAPI_SS_STATUS_QUIESCENT;
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
				evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_get_ss_waiting_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (res == TAPI_SS_SUCCESS) {
		int i;
		TelSsWaitingResp_t resp;
		TelSsWaitingResp *resp_buf =  (TelSsWaitingResp *)data;

		resp.record_num = resp_buf->record_num;
		for(i = 0; i < resp.record_num; i++) {
			resp.record[i].Class = __map_ss_class_resp(resp_buf->records[i].class);
			resp.record[i].Status = ((resp_buf->records[i].enable) ? TAPI_SS_STATUS_ACTIVE : TAPI_SS_STATUS_QUIESCENT);
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free (evt_cb_data);
}

static void on_response_set_ss_waiting_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (res == TAPI_SS_SUCCESS) {
		TelSsWaitingInfo_t *resp_buf = (TelSsWaitingInfo_t *)evt_cb_data->legacy_user_data;
		TelSsWaitingResp_t resp;
		resp.record_num = 1;
		resp.record[0].Class = resp_buf->Class;
		if (resp_buf->Mode == TAPI_SS_CW_MODE_ACTIVATE) {
			resp.record[0].Status = TAPI_SS_STATUS_ACTIVE;
		} else {
			resp.record[0].Status = TAPI_SS_STATUS_QUIESCENT;
		}

		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
				evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_get_ss_cli_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (res == TAPI_SS_SUCCESS) {
		TelSsCliResp_t resp;
		TelSsCliResp *resp_buf = (TelSsCliResp *)data;

		resp.Type = resp_buf->type;
		switch(resp_buf->type) {
			case TEL_SS_CLI_CLIP :
				resp.Status = __map_ss_cli_status_resp(resp_buf->status.clip.net_status, resp_buf->status.clip.dev_status);
				break;
			case TEL_SS_CLI_CLIR :
				resp.Status = __map_ss_clir_status_resp(resp_buf->status.clir.net_status, resp_buf->status.clir.dev_status);
				break;
			case TEL_SS_CLI_COLP :
				resp.Status = __map_ss_cli_status_resp(resp_buf->status.colp.net_status, resp_buf->status.colp.dev_status);
				break;
			case TEL_SS_CLI_COLR :
				resp.Status = __map_ss_cli_status_resp(resp_buf->status.colr.net_status, resp_buf->status.colr.dev_status);
				break;
			case TEL_SS_CLI_CDIP :
				resp.Status = __map_ss_cli_status_resp(resp_buf->status.cdip.net_status, resp_buf->status.cdip.dev_status);
				break;
			case TEL_SS_CLI_CNAP :
				resp.Status = __map_ss_cli_status_resp(resp_buf->status.cnap.net_status, resp_buf->status.cnap.dev_status);
				break;
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free (evt_cb_data);
}

static void on_response_send_ss_ussd_request(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSsCause_t res = __map_ss_result(result);

	if (res == TAPI_SS_SUCCESS) {
		TelSsUssdResp_t resp;
		TelSsUssdResp *resp_buf = (TelSsUssdResp *)data;

		MAP_SS_USSD_TYPE_RESP(resp_buf->type, resp.Type);
		MAP_SS_USSD_STATUS_RESP(resp_buf->status, resp.Status);
		if (resp_buf->str) {
				resp.Length = strlen((const char *)resp_buf->str);
				g_strlcpy(resp.szString, (gchar *)resp_buf->str, TAPI_SS_USSD_DATA_SIZE_MAX + 1);
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free (evt_cb_data);
}

EXPORT_API int tel_set_ss_barring(TapiHandle *handle, TelSsBarringInfo_t *info, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsBarringInfo info_buf = {0,};

	dbg("Func Entrance");

	info_buf.class = __map_ss_class(info->Class);
	info_buf.enable = ((info->Mode == TAPI_SS_CB_MODE_ACTIVATE) ? TRUE: FALSE);
	MAP_SS_BARRING_TYPE(info->Type, info_buf.type);
	if (info->szPassword[0])
		g_strlcpy(info_buf.pwd, info->szPassword, TAPI_SS_GSM_BARR_PW_LEN_MAX + 1);

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, info, sizeof(TelSsBarringInfo_t));

	ret = tapi_ss_set_barring((TelHandle *)handle, &info_buf, on_response_set_ss_barring_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_get_ss_barring_status(TapiHandle *handle, TelSsClass_t class, TelSsBarringType_t type, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsBarringGetInfo get_barring_info_buf = {0,};

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	get_barring_info_buf.class = __map_ss_class(class);
	MAP_SS_BARRING_TYPE(type, get_barring_info_buf.type);

	ret = tapi_ss_get_barring_status((TelHandle *)handle, &get_barring_info_buf, on_response_get_ss_barring_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_change_ss_barring_password(TapiHandle *handle,
		const char *old_password,
		const char *new_password,
		const char *new_password_again,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsBarringPwdInfo pwd_info_buf = {{0,}, };

	dbg("Func Entrance ");

	if(g_strcmp0(new_password, new_password_again))
		return TAPI_API_INVALID_INPUT;

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	if (old_password)
		g_strlcpy(pwd_info_buf.old_pwd, old_password, TEL_SS_BARRING_PASSWORD_LEN_MAX + 1);
	if (new_password)
		g_strlcpy(pwd_info_buf.new_pwd, new_password, TEL_SS_BARRING_PASSWORD_LEN_MAX + 1);

	/*new_password_again not needed by TAPI 3.0*/

	ret = tapi_ss_change_barring_password((TelHandle *)handle, &pwd_info_buf, on_response_change_ss_barring_password, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_ss_forward(TapiHandle *handle, const TelSsForwardInfo_t *info, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsForwardInfo info_buf = {0,};

	dbg("Func Entrance ");

	info_buf.class = __map_ss_class(info->Class);
	info_buf.mode = info->Mode;
	MAP_SS_FWD_CONDITION(info->Condition, info_buf.condition);
	if (info->szPhoneNumber[0])
		g_strlcpy(info_buf.number, info->szPhoneNumber, TAPI_CALL_DIALDIGIT_LEN_MAX + 1);
	info_buf.wait_time = info->NoReplyConditionTimer;

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, info, sizeof(TelSsForwardInfo_t));

	ret = tapi_ss_set_forwarding((TelHandle *)handle, &info_buf, on_response_set_ss_forward_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_ss_forward_status(TapiHandle *handle, TelSsClass_t class, TelSsForwardWhen_t condition, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsForwardGetInfo info_buf = {0,};

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	info_buf.class = __map_ss_class(class);
	MAP_SS_FWD_CONDITION(condition, info_buf.condition);

	ret = tapi_ss_get_forwarding_status((TelHandle *)handle, &info_buf, on_response_get_ss_forward_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_ss_waiting(TapiHandle *handle, const TelSsWaitingInfo_t *info, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsWaitingInfo info_buf = {0,};

	dbg("Func Entrance ");

	info_buf.class = __map_ss_class(info->Class);
	info_buf.enable= ((info->Mode == TAPI_SS_CW_MODE_ACTIVATE) ? TRUE: FALSE);

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, info, sizeof(TelSsWaitingInfo_t));

	ret = tapi_ss_set_waiting((TelHandle *)handle, &info_buf, on_response_set_ss_waiting_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_ss_waiting_status(TapiHandle *handle, const TelSsClass_t class, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsClass buf_class;

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	buf_class = __map_ss_class(class);

	ret = tapi_ss_get_waiting_status((TelHandle *)handle, buf_class, on_response_get_ss_waiting_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_ss_cli_status(TapiHandle *handle, TelSsCliType_t type, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsCliType type_buf = 0;

	dbg("Func Entrance ");

	switch (type) {
		case TAPI_SS_CLI_CLIP:
			type_buf = TEL_SS_CLI_CLIP;
			break;
		case TAPI_SS_CLI_CLIR:
			type_buf = TEL_SS_CLI_CLIR;
			break;
		case TAPI_SS_CLI_COLP:
			type_buf = TEL_SS_CLI_COLP;
			break;
		case TAPI_SS_CLI_COLR:
			type_buf = TEL_SS_CLI_COLR;
			break;
		case TAPI_SS_CLI_CDIP:
			type_buf = TEL_SS_CLI_CDIP;
			break;
		case TAPI_SS_CLI_CNAP:
			type_buf = TEL_SS_CLI_CNAP;
			break;
	}
	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_ss_get_cli_status((TelHandle *)handle, type_buf, on_response_get_ss_cli_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_send_ss_ussd_request(TapiHandle *handle, const TelSsUssdMsgInfo_t *info, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSsUssdInfo info_buf = {0,};

	dbg("Func Entrance ");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	MAP_SS_USSD_TYPE (info->Type, info_buf.type);
	if (info->szString[0]) {

		info_buf.str = g_malloc0(TAPI_SS_USSD_DATA_SIZE_MAX + 1);
		g_strlcpy((gchar *)info_buf.str, info->szString, TAPI_SS_USSD_DATA_SIZE_MAX + 1);
	}

	dbg("ussd string [%s]", info_buf.str);
	ret = tapi_ss_send_ussd_request((TelHandle *)handle, &info_buf, on_response_send_ss_ussd_request, (void *)cb_data);
	g_free(info_buf.str);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_ss_aoc_info(TapiHandle *handle, TelSsAocType_t AocType, tapi_response_cb callback, void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}

EXPORT_API int tel_set_ss_aoc_info(TapiHandle *handle, TelCallAocInfo_t *AocSetInfo, tapi_response_cb callback, void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}

/*	EOF  */
