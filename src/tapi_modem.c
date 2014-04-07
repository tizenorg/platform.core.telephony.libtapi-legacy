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

#include "TelMisc.h"
#include "TelPower.h"
#include "ITapiModem.h"
#include "ITapiPS.h"

#include "common.h"
#include "tapi_log.h"

#include <tel_return.h>
#include <tapi_modem.h>

#define MAKE_ASYNC_CB_DATA(cb_data, handle_buf, ret_buf, data_buf, data_len, user_data_buf, callback)  \
	cb_data = g_new0(call_back_data, 1); \
	cb_data->handle = handle_buf; \
	cb_data->ret = ret_buf; \
	cb_data->data = g_memdup(data_buf, data_len); \
	cb_data->user_data = user_data_buf; \
	cb_data->cb = callback

typedef struct {
	TelHandle *handle;
	TelReturn ret;
	void *data;
	void *user_data;
	tapi_response_cb cb;
}call_back_data;

static gboolean async_callback(gpointer data_ptr)
{
	call_back_data *cb_data = data_ptr;

	if (cb_data->ret == TEL_RETURN_SUCCESS) {
		cb_data->ret = TAPI_API_SUCCESS;
	}	else {
		cb_data->ret = TAPI_API_OPERATION_FAILED;
	}
	cb_data->cb(cb_data->handle, cb_data->ret, cb_data->data, cb_data->user_data);

	g_free(cb_data->data);
	g_free(cb_data);

	return FALSE;
}

static gboolean __modem_get_version(TelHandle *handle, TelMiscVersionInformation *data)
{
	TelModemVersion info_buf = {{0},};
	TelReturn ret;

	ret = tapi_modem_get_version(handle, &info_buf);
	if (ret == TEL_RETURN_SUCCESS) {
		if (info_buf.software_version[0])
			g_strlcpy((gchar *)data->szSwVersion,
				info_buf.software_version, MAX_VERSION_LEN);

		if (info_buf.hardware_version[0])
			g_strlcpy((gchar *)data->szHwVersion,
				info_buf.hardware_version, MAX_VERSION_LEN);

		if (info_buf.calibration_date[0])
			g_strlcpy((gchar *)data->szRfCalDate,
				info_buf.calibration_date, MAX_VERSION_LEN);

		if (info_buf.product_code[0])
			g_strlcpy((gchar *)data->szProductCode,
				info_buf.product_code, TAPI_MISC_PRODUCT_CODE_LEN_MAX);
		return TRUE;
	}
	return FALSE;
}

static void on_response_default_set(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (result == TEL_RETURN_SUCCESS) {
		result = TAPI_API_SUCCESS;
	} else {
		result = TAPI_API_OPERATION_FAILED;
	}

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, result, NULL, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

static void on_response_set_flight_mode(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	tapi_power_flight_mode_resp_type_t res;

	gboolean enable = *(gboolean *)evt_cb_data->legacy_user_data;

	if (result == TEL_RETURN_SUCCESS) {
		if (enable == TRUE ) {
			res = TAPI_POWER_FLIGHT_MODE_RESP_ON;
		} else {
			res = TAPI_POWER_FLIGHT_MODE_RESP_OFF;
		}
	} else {
		res = TAPI_POWER_FLIGHT_MODE_RESP_FAIL;
	}

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}

	g_free (evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

EXPORT_API int tel_process_power_command(TapiHandle *handle, tapi_power_phone_cmd_t cmd, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_modem_set_power_status((TelHandle *)handle, (TelModemPowerStatus)cmd, on_response_default_set, cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_flight_mode(TapiHandle *handle, tapi_power_flight_mode_type_t mode, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	gboolean enable = TRUE;

	dbg("Func Entrance");

	if (mode == TAPI_POWER_FLIGHT_MODE_ENTER)
		enable = TRUE;
	else if (mode == TAPI_POWER_FLIGHT_MODE_LEAVE)
		enable = FALSE;
	else {
		err("Invalid Parameter");
		return TAPI_API_INVALID_INPUT;
	}

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, &enable, sizeof(enable));

	ret = tapi_modem_set_flight_mode((TelHandle *)handle, enable, on_response_set_flight_mode, cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_flight_mode(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	gboolean mode;
	call_back_data *cb_data;

	dbg("Func Entrance");

	ret = tapi_modem_get_flight_mode((TelHandle *)handle, &mode);

	if (ret != TEL_RETURN_SUCCESS) {
		err("Operation Failed");
		return TAPI_API_OPERATION_FAILED;
	}

	MAKE_ASYNC_CB_DATA(cb_data, handle, ret, (void *)&mode,
		sizeof(gboolean), user_data, callback);

	g_idle_add(async_callback, (gpointer)cb_data);

	return TAPI_API_SUCCESS;
}

EXPORT_API int tel_get_misc_me_version(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelMiscVersionInformation data = {0,};
	gboolean ret;
	call_back_data *cb_data;

	dbg("Func Entrance");

	ret = __modem_get_version((TelHandle *)handle, &data);
	if (ret == FALSE) {
		err("Operation Failed");
		return TAPI_API_OPERATION_FAILED;
	}

	MAKE_ASYNC_CB_DATA(cb_data, handle, ret, (void *)&data, sizeof(TelMiscVersionInformation), user_data, callback);

	g_idle_add(async_callback, (gpointer)cb_data);

	return TAPI_API_SUCCESS;
}

EXPORT_API TelMiscVersionInformation *tel_get_misc_me_version_sync(TapiHandle *handle)
{
	gboolean ret;
	TelMiscVersionInformation data = {0,};

	dbg("Func Entrance");

	ret = __modem_get_version((TelHandle *)handle, &data);

	if (ret == FALSE) {
		err("Operation Failed");
		return NULL;
	}

	return (TelMiscVersionInformation *)g_memdup(&data, sizeof(TelMiscVersionInformation));
}

EXPORT_API int tel_get_misc_me_imei(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	char imei[TEL_MODEM_IMEI_LENGTH_MAX + 1];
	call_back_data *cb_data;

	dbg("Func Entrance");

	ret = tapi_modem_get_imei((TelHandle *)handle, imei);

	if (ret != TEL_RETURN_SUCCESS) {
		err("Operation Failed");
		return TAPI_API_OPERATION_FAILED;
	}

	MAKE_ASYNC_CB_DATA(cb_data, handle, ret, (void *)imei, (TEL_MODEM_IMEI_LENGTH_MAX + 1), user_data, callback);

	g_idle_add(async_callback, (gpointer)cb_data);

	return TAPI_API_SUCCESS;
}

EXPORT_API char *tel_get_misc_me_imei_sync(TapiHandle *handle)
{
	TelReturn ret;
	char imei[TEL_MODEM_IMEI_LENGTH_MAX + 1];

	dbg("Func Entrance");

	ret = tapi_modem_get_imei((TelHandle *)handle, imei);
	if (ret != TEL_RETURN_SUCCESS) {
		err("Operation Failed");
		g_free(imei);
		return NULL;
	}

	return g_memdup(imei, TEL_MODEM_IMEI_LENGTH_MAX + 1);
}

EXPORT_API int tel_get_misc_me_sn(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	/*Functionality removed in TAPI 3.0*/
	err("Deprecated!!");
	return TAPI_API_DEPRICATED;

}

EXPORT_API char *tel_get_misc_me_sn_sync(TapiHandle *handle)
{
	/*Functionality removed in TAPI 3.0*/
	err("Deprecated!!");
	return NULL;

}


EXPORT_API int tel_check_modem_power_status(TapiHandle *handle, int *result)
{
	TelReturn ret;

	dbg("Func Entrance");

	ret = tapi_modem_get_power_status((TelHandle *)handle, (TelModemPowerStatus *)result);
	if(ret == TEL_RETURN_SUCCESS) {
		switch(*result) {
		case TEL_MODEM_POWER_ON:
			*result = TAPI_PHONE_POWER_STATUS_ON;
			break;
		case TEL_MODEM_POWER_OFF:
			*result = TAPI_PHONE_POWER_STATUS_OFF;
			break;
		case TEL_MODEM_POWER_ERROR:
			*result = TAPI_PHONE_POWER_STATUS_ERROR;
			break;
		}
	}

	CHECK_RETURN(ret, NULL);
}


EXPORT_API int tel_control_modem_dun_pin_ctrl(TapiHandle *handle, tapi_ps_btdun_pincontrol *pincontrol, tapi_response_cb callback, void *user_data)
{
	/*Functionality removed in TAPI 3.0*/
	err("Deprecated!!");
	return TAPI_API_DEPRICATED;

}
