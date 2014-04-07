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
#include <stdlib.h>
#include <string.h>

#include "tapi_common.h"
#include "TapiUtility.h"
#include "TelSim.h"

#include "common.h"
#include "tapi_log.h"
#include "ITapiSap.h"

#include <tapi_sap.h>
#include <tel_return.h>

static TelSimSapResultCode_t __map_sap_result(int result)
{
	switch(result) {
		case TEL_SAP_RESULT_SUCCESS :
			return TAPI_SIM_SAP_RESULT_CODE_OK;
		case TEL_SAP_RESULT_FAILURE_NO_REASON :
			return TAPI_SIM_SAP_RESULT_CODE_NO_REASON;
		case TEL_SAP_RESULT_CARD_NOT_ACCESSIBLE :
			return TAPI_SIM_SAP_RESULT_CODE_CARD_NOT_ACCESSIBLE;
		case TEL_SAP_RESULT_CARD_ALREADY_POWERED_OFF :
			return TAPI_SIM_SAP_RESULT_CODE_CARD_ALREADY_POWER_OFF;
		case TEL_SAP_RESULT_CARD_REMOVED :
			return TAPI_SIM_SAP_RESULT_CODE_CARD_REMOVED;
		case TEL_SAP_RESULT_CARD_ALREADY_POWERED_ON :
			return TAPI_SIM_SAP_RESULT_CODE_CARD_ALREADY_POWER_ON;
		case TEL_SAP_RESULT_DATA_NOT_AVAILABLE :
			return TAPI_SIM_SAP_RESULT_CODE_DATA_NOT_AVAILABLE;
		default :
			return TAPI_SIM_SAP_RESULT_CODE_NOT_SUPPORT;
	}
}

static void on_response_sap_connect(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSapConnectionStatus_t conn_status = -1;

	switch(result) {
		case TEL_SAP_RESULT_SUCCESS :
			conn_status = TAPI_SIM_SAP_CONNECTION_STATUS_OK;
			break;
		case TEL_SAP_RESULT_UNABLE_TO_ESTABLISH :
			conn_status = TAPI_SIM_SAP_CONNECTION_STATUS_UNABLE_TO_ESTABLISH;
			break;
		case TEL_SAP_RESULT_NOT_SUPPORT_MAX_SIZE :
			conn_status = TAPI_SIM_SAP_CONNECTION_STATUS_NOT_SUPPORT_MAX_SIZE;
			break;
		case TEL_SAP_RESULT_TOO_SMALL_MAX_SIZE :
			conn_status = TAPI_SIM_SAP_CONNECTION_STATUS_TOO_SMALL_MAX_SIZE;
			break;
	}
	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, conn_status, data, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

EXPORT_API int tel_req_sap_connect(TapiHandle *handle, int max_msg_size, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_sap_req_connect((TelHandle *)handle, (unsigned int)max_msg_size, on_response_sap_connect, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

static void on_response_sap_disconnect(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSapResultCode_t sap_result = __map_sap_result(result);

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, sap_result, NULL, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

EXPORT_API int tel_req_sap_disconnect(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_sap_req_disconnect((TelHandle *)handle, on_response_sap_disconnect, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_req_sap_connection_status(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	/*Not Supported in Tizen 3.0*/
	err("Operation not supported");
	return TAPI_API_NOT_SUPPORTED;
}

static void on_response_sap_transfer_atr(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSapResultCode_t sap_result = __map_sap_result(result);

	if (sap_result == TAPI_SIM_SAP_RESULT_CODE_OK) {
		TelSapAtrInfo_t atr = {0, };
		TelSapAtr *atr_buf = data;

		atr.atr_len = (int)atr_buf->atr_len;
		memcpy(atr.atr_data, atr_buf->atr, atr_buf->atr_len);

		if (evt_cb_data->cb)
			evt_cb_data->cb((TapiHandle *)handle, sap_result, &atr, evt_cb_data->user_data);
	} else {
		if (evt_cb_data->cb)
			evt_cb_data->cb((TapiHandle *)handle, sap_result, NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

EXPORT_API int tel_req_sap_transfer_atr(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_sap_get_atr((TelHandle *)handle, on_response_sap_transfer_atr, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

static void on_response_sap_transfer_apdu(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSapResultCode_t sap_result = __map_sap_result(result);

	if (sap_result == TAPI_SIM_SAP_RESULT_CODE_OK) {
		TelSapApduData_t apdu = {0, };
		TelSapApduResp *apdu_buf = data;

		apdu.apdu_len = (int)apdu_buf->apdu_resp_len;
		memcpy(apdu.apdu_data, apdu_buf->apdu_resp, apdu_buf->apdu_resp_len);

		if (evt_cb_data->cb)
			evt_cb_data->cb((TapiHandle *)handle, sap_result, &apdu, evt_cb_data->user_data);
	} else {
		if (evt_cb_data->cb)
			evt_cb_data->cb((TapiHandle *)handle, sap_result, NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

EXPORT_API int tel_req_sap_transfer_apdu(TapiHandle *handle, TelSapApduData_t *apdu_data, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSapApdu apdu_data_buf = {0, };

	dbg("Func Entrance");

	if (apdu_data->apdu_len > TAPI_SIM_APDU_MAX_LEN) {
		err("Invalid input");
		return TAPI_API_INVALID_INPUT;
	}
	apdu_data_buf.apdu_len = (unsigned int)apdu_data->apdu_len;
	memcpy(apdu_data_buf.apdu, apdu_data->apdu_data, apdu_data->apdu_len);

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_sap_req_transfer_apdu((TelHandle *)handle, &apdu_data_buf, on_response_sap_transfer_apdu, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

static void on_response_sap_transport_protocol(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSapResultCode_t sap_result = __map_sap_result(result);

	if (evt_cb_data->cb)
		evt_cb_data->cb((TapiHandle *)handle, sap_result, NULL, evt_cb_data->user_data);

	g_free(evt_cb_data);
}

EXPORT_API int tel_req_sap_transport_protocol(TapiHandle *handle, TelSimSapProtocol_t protocol, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_sap_req_transport_protocol((TelHandle *)handle, (TelSimSapProtocol)protocol, on_response_sap_transport_protocol, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

static void on_response_sap_power_operation(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSapResultCode_t sap_result = __map_sap_result(result);

	if (evt_cb_data->cb)
		evt_cb_data->cb((TapiHandle *)handle, sap_result, NULL, evt_cb_data->user_data);

	g_free(evt_cb_data);
}

EXPORT_API int tel_req_sap_power_operation(TapiHandle *handle, TelSimSapPowerMode_t power_mode, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_sap_req_power_operation((TelHandle *)handle, (TelSapPowerMode)power_mode, on_response_sap_power_operation, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

static void on_response_sap_cardreader_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSapResultCode_t sap_result = __map_sap_result(result);

	if (evt_cb_data->cb)
		evt_cb_data->cb((TapiHandle *)handle, sap_result, (TelSimSapCardStatus_t *)data, evt_cb_data->user_data);

	g_free(evt_cb_data);
}

EXPORT_API int tel_req_sap_cardreader_status(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_sap_get_cardreader_status((TelHandle *)handle, on_response_sap_cardreader_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}
