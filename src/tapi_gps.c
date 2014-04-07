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

#include "common.h"
#include "tapi_log.h"
#include "ITapiGps.h"

#include <tapi_gps.h>
#include <tel_return.h>

static void on_response_set_gps_frequency_aiding(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	switch (result) {
	case TEL_GPS_RESULT_SUCCESS :
		result = TAPI_API_SUCCESS;
		break;
	case TEL_GPS_RESULT_INVALID_PARAMETER :
		result = TAPI_API_INVALID_INPUT;
		break;
	case TEL_GPS_RESULT_MEMORY_FAILURE :
		result = TAPI_API_SYSTEM_OUT_OF_MEM;
		break;
	case TEL_GPS_RESULT_OPERATION_NOT_SUPPORTED :
		result = TAPI_API_NOT_SUPPORTED;
		break;
	case TEL_GPS_RESULT_OPERATION_NOT_PERMITTED :
		result = TAPI_API_INVALID_OPERATION;
		break;
	default :
		result = TAPI_API_OPERATION_FAILED;
		break;
	}

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, result, NULL, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

EXPORT_API int tel_set_gps_frequency_aiding(TapiHandle *handle, unsigned char state, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_gps_set_frequency_aiding((TelHandle *)handle, (gboolean)state, on_response_set_gps_frequency_aiding, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_confirm_gps_measure_pos(TapiHandle *handle, unsigned char *data, unsigned int data_len)
{
	TelGpsDataInfo gps_data = {0, };
	TelReturn ret;

	dbg("Func Entrance");

	gps_data.data = data;
	gps_data.data_len = data_len;

	ret = tapi_gps_confirm_measure_pos((TelHandle *)handle, &gps_data);

	CHECK_RETURN(ret, NULL);
}

