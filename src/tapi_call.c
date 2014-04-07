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

#include "TelCall.h"
#include "tapi_common.h"
#include "TapiUtility.h"

#include "common.h"
#include "tapi_log.h"
#include "ITapiCall.h"

#include <tel_return.h>
#include <tapi_call.h>

static TelCallCause_t __map_call_result(int result)
{
	switch(result) {
		case TEL_CALL_RESULT_SUCCESS :
			return TAPI_CAUSE_SUCCESS;
		case TEL_CALL_RESULT_MEMORY_FAILURE :
			return TAPI_CAUSE_MEMCAPEXCEEDED;
		case TEL_CALL_RESULT_OPERATION_NOT_SUPPORTED :
		case TEL_CALL_RESULT_OPERATION_NOT_PERMITTED :
			return TAPI_CAUSE_OPERATIONUNAVAIL;
		case TEL_CALL_RESULT_NETWORK_BUSY :
			return TAPI_CAUSE_BUSY;
		case TEL_CALL_RESULT_SERVICE_NOT_ALLOWED :
			return TAPI_CAUSE_SERVICE_UNAVAIL;
		case TEL_CALL_RESULT_FDN_RESTRICTED :
			return TAPI_CAUSE_FIXED_DIALING_NUMBER_ONLY;
		default :
			return TAPI_CAUSE_UNKNOWN;
	}
}

static void on_response_call_dial(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_call_answer(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelCallAnswerCnf_t call_answer_resp;

	call_answer_resp.id = *(unsigned int *)evt_cb_data->legacy_user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), &call_answer_resp, evt_cb_data->user_data);
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_call_end(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result),
				(TelCallEndCnf_t *)evt_cb_data->legacy_user_data, evt_cb_data->user_data);
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_call_hold(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelCallHoldCnf_t call_hold_resp;

	call_hold_resp.id = *(unsigned int *)evt_cb_data->legacy_user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), &call_hold_resp, evt_cb_data->user_data);
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_call_active(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelCallActiveCnf_t call_active_resp;

	call_active_resp.id = *(unsigned int *)evt_cb_data->legacy_user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), &call_active_resp, evt_cb_data->user_data);
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_call_swap(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelCallSwapCnf_t call_swap_resp;

	call_swap_resp.id = *(unsigned int *)evt_cb_data->legacy_user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), &call_swap_resp, evt_cb_data->user_data);
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_call_join(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelCallJoinCnf_t call_join;

	call_join.id = *(unsigned int *)evt_cb_data->legacy_user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), &call_join, evt_cb_data->user_data);
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_call_split(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelCallSplitCnf_t call_split_resp;

	call_split_resp.id = *(unsigned int *)evt_cb_data->legacy_user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), &call_split_resp, evt_cb_data->user_data);
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_call_transfer(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelCallTransferCnf_t call_transfer_resp;

	call_transfer_resp.id = *(unsigned int *)evt_cb_data->legacy_user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), &call_transfer_resp, evt_cb_data->user_data);
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_call_dtmf(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_call_deflect(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_call_get_active_line(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result),
				(TelCallActiveLine_t *)data, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_call_set_active_line(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_get_call_volume_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelCallCause_t call_result;

	call_result = __map_call_result(result);
	dbg("call_result[%d]", call_result);

	if (call_result == TAPI_CAUSE_SUCCESS) {
		TelCallGetVolumeInfoResp_t vol_info_resp = {0, };

		vol_info_resp = *(TelCallGetVolumeInfoResp_t *)evt_cb_data->legacy_user_data;
		vol_info_resp.record_num = 1;
		vol_info_resp.record[0].volume = *(TelSoundVolume_t *)data;

		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, call_result, &vol_info_resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, call_result, NULL, evt_cb_data->user_data);
		}
	}
	g_free(evt_cb_data->legacy_user_data);
	g_free(evt_cb_data);
}

static void on_response_set_call_volume_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_set_call_sound_path(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_set_call_mute_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb)
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	g_free(evt_cb_data);
}

static void on_response_get_call_mute_status(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb)
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result),
				(TelSoundMuteStatus_t *)data, evt_cb_data->user_data);

	g_free(evt_cb_data);
}

static void on_response_set_call_sound_recording(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_set_call_sound_equalization(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_call_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

EXPORT_API int tel_dial_call(TapiHandle *handle, const TelCallDial_t *pParams, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelCallDial call_dial_info_buf = {0, };

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	call_dial_info_buf.call_type = pParams->CallType;
	g_strlcpy(call_dial_info_buf.number, pParams->szNumber, TEL_CALL_CALLING_NUMBER_LEN_MAX +1);

	ret = tapi_call_dial((TelHandle *)handle, (const TelCallDial *)&call_dial_info_buf, on_response_call_dial, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_answer_call(TapiHandle *handle, unsigned int CallHandle, TelCallAnswerType_t AnsType, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &CallHandle, sizeof(unsigned int));

	ret = tapi_call_answer((TelHandle *)handle, (TelCallAnswerType)AnsType, on_response_call_answer, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_end_call(TapiHandle *handle, unsigned int CallHandle, TelCallEndType_t EndType, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelCallEnd call_end_info_buf = {0, };
	TelCallEndCnf_t legacy_usr_data = {0, };

	dbg("Func Entrance");

	legacy_usr_data.id = CallHandle;
	legacy_usr_data.type = EndType;

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &legacy_usr_data, sizeof(TelCallEndCnf_t));

	call_end_info_buf.call_id = CallHandle;
	call_end_info_buf.end_type = EndType;

	ret = tapi_call_end((TelHandle *)handle, (const TelCallEnd *)&call_end_info_buf, on_response_call_end, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_hold_call(TapiHandle *handle, unsigned int CallHandle, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &CallHandle, sizeof(unsigned int));

	ret = tapi_call_hold((TelHandle *)handle, on_response_call_hold, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_active_call(TapiHandle *handle, unsigned int CallHandle, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &CallHandle, sizeof(unsigned int));

	ret = tapi_call_active((TelHandle *)handle, on_response_call_active, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_swap_call(TapiHandle *handle, unsigned int CallHandle1, unsigned int CallHandle2, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &CallHandle1, sizeof(unsigned int));

	ret = tapi_call_swap((TelHandle *)handle, on_response_call_swap, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_join_call(TapiHandle *handle, unsigned int CallHandle1, unsigned int CallHandle2, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &CallHandle1, sizeof(unsigned int));

	ret = tapi_call_join((TelHandle *)handle, on_response_call_join, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_split_call(TapiHandle *handle, unsigned int CallHandle, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &CallHandle, sizeof(unsigned int));

	ret = tapi_call_split((TelHandle *)handle, CallHandle, on_response_call_split, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_transfer_call(TapiHandle *handle, unsigned int CallHandle, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &CallHandle, sizeof(unsigned int));

	ret = tapi_call_transfer((TelHandle *)handle, on_response_call_transfer, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_activate_call_ccbs(TapiHandle *handle, unsigned int CallHandle, tapi_response_cb callback, void *user_data )
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_call_dtmf(TapiHandle *handle, const char *pDtmfString, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_send_dtmf((TelHandle *)handle, pDtmfString, on_response_call_dtmf, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_deflect_call(TapiHandle *handle, unsigned int CallHandle, const TelCallDeflectDstInfo_t *pInfo, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_deflect((TelHandle *)handle, (const char *)pInfo->number, on_response_call_deflect, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_call_status(TapiHandle *handle, int call_id, TelCallStatus_t *out )
{
	TelReturn ret;
	TelCallStatus call_status_buf = {0,};

	dbg("Func Entrance");

	ret = tapi_call_get_status((TelHandle *)handle, (unsigned int)call_id, &call_status_buf);

	if (ret == TEL_RETURN_SUCCESS) {
		out->CallHandle = (int)call_status_buf.call_id;
		out->CallType = call_status_buf.call_type;
		out->bMoCall = call_status_buf.mo_call;
		out->CallState = call_status_buf.call_state;
		out->bConferenceState = call_status_buf.mpty;
		g_strlcpy(out->pNumber, call_status_buf.number, TAPI_CALL_DIALDIGIT_LEN_MAX+1);
	}
	CHECK_RETURN(ret, NULL);
}

EXPORT_API int tel_get_call_status_all( TapiHandle *handle, TelCallStatusCallback cb, void *user_data )
{
	TelReturn ret;
	TelCallStatusList call_status_list_buf = {0, };
	unsigned int i = 0;

	dbg("Func Entrance");

	ret = tapi_call_get_status_all((TelHandle *)handle, &call_status_list_buf);

	if (ret == TEL_RETURN_SUCCESS) {
		while(i < call_status_list_buf.count) {
			TelCallStatus_t call_status = {0, };

			call_status.CallHandle = (int)call_status_list_buf.status[i].call_id;
			call_status.CallType = call_status_list_buf.status[i].call_type;
			call_status.bMoCall = call_status_list_buf.status[i].mo_call;
			call_status.CallState = call_status_list_buf.status[i].call_state;
			call_status.bConferenceState = call_status_list_buf.status[i].mpty;
			g_strlcpy(call_status.pNumber, call_status_list_buf.status[i].number, TAPI_CALL_DIALDIGIT_LEN_MAX+1);

			cb(&call_status, user_data);
			i++;
		}
	}
	CHECK_RETURN(ret, NULL);
}

EXPORT_API int tel_get_call_act_line(TapiHandle *handle, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_get_active_line((TelHandle *)handle, on_response_call_get_active_line, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_call_act_line(TapiHandle *handle, TelCallActiveLine_t active_line, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_set_active_line((TelHandle *)handle, (TelCallActiveLine)active_line, on_response_call_set_active_line, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

/*================================================================================================*/

EXPORT_API int tel_get_call_duration(TapiHandle *handle, unsigned int CallHandle, unsigned int *pDurationInSecs)
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_get_call_conf_list(TapiHandle *handle, unsigned int CallHandle, unsigned int *pCallList, int *pNoOfCalls)
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_get_call_privacy_mode(TapiHandle *handle, TelCallPrivacyType_t PrivacyType, tapi_response_cb callback, void *user_data )
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_set_call_privacy_mode(TapiHandle *handle, TelCallVoicePrivacyInfo_t PrivacyInfo, tapi_response_cb callback, void *user_data )
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_exe_call_flash_info(TapiHandle *handle, const char *pDialNumber, tapi_response_cb callback, void *user_data )
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_exit_call_emergency_mode(TapiHandle *handle, tapi_response_cb callback, void *user_data )
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_get_call_time(TapiHandle *handle, unsigned short req_mask, tapi_response_cb callback, void *user_data )
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_get_call_volume_info(TapiHandle *handle, TelSoundDevice_t device, TelSoundType_t type, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelCallGetVolumeInfoResp_t legacy_usr_data = {0, };

	dbg("Func Entrance");

	legacy_usr_data.record[0].device = device;
	legacy_usr_data.record[0].type = type;

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, &legacy_usr_data, sizeof(TelCallGetVolumeInfoResp_t));

	ret = tapi_call_get_volume_info((TelHandle *)handle, (TelCallSoundDevice)device, on_response_get_call_volume_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_call_volume_info(TapiHandle *handle, TelCallVolumeInfo_t *info, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelCallVolumeInfo vol_info_buf = {0, };

	dbg("Func Entrance");

	vol_info_buf.device = info->device;
	vol_info_buf.volume = info->volume * 10;

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_set_volume_info((TelHandle *)handle, &vol_info_buf,
		on_response_set_call_volume_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_call_sound_path(TapiHandle *handle, TelCallSoundPathInfo_t *info, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelCallSoundPathInfo sound_path_info_buf = {0, };

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	sound_path_info_buf.ex_volume = info->ex_volume;
	switch(info->path) {
		case TAPI_SOUND_PATH_HANDSET :
			sound_path_info_buf.path = TEL_SOUND_PATH_HANDSET;
			break;
		case TAPI_SOUND_PATH_HEADSET :
			sound_path_info_buf.path =  TEL_SOUND_PATH_HEADSET;
			break;
		case TAPI_SOUND_PATH_HANDSFREE :
			sound_path_info_buf.path = TEL_SOUND_PATH_HANDSFREE;
			break;
		case TAPI_SOUND_PATH_BLUETOOTH :
			sound_path_info_buf.path = TEL_SOUND_PATH_BLUETOOTH;
			break;
		case TAPI_SOUND_PATH_STEREO_BLUETOOTH :
			sound_path_info_buf.path = TEL_SOUND_PATH_STEREO_BLUETOOTH;
			break;
		case TAPI_SOUND_PATH_SPK_PHONE :
			sound_path_info_buf.path = TEL_SOUND_PATH_SPK_PHONE;
			break;
		case TAPI_SOUND_PATH_HEADSET_3_5PI :
			sound_path_info_buf.path = TEL_SOUND_PATH_HEADSET_3_5PI;
			break;
		case TAPI_SOUND_PATH_BT_NSEC_OFF :
			sound_path_info_buf.path = TEL_SOUND_PATH_BT_NSEC_OFF;
			break;
		case TAPI_SOUND_PATH_MIC1 :
			sound_path_info_buf.path = TEL_SOUND_PATH_MIC1;
			break;
		case TAPI_SOUND_PATH_MIC2 :
			sound_path_info_buf.path = TEL_SOUND_PATH_MIC2;
			break;
		case TAPI_SOUND_PATH_HEADSET_HAC :
			sound_path_info_buf.path = TEL_SOUND_PATH_HEADSET_HAC;
			break;
	}

	ret = tapi_call_set_sound_path((TelHandle *)handle, &sound_path_info_buf,
		on_response_set_call_sound_path, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_call_mute_status(TapiHandle *handle, TelSoundMuteStatus_t mute, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_set_mute((TelHandle *)handle, (gboolean)mute,
		on_response_set_call_mute_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_call_mute_status(TapiHandle *handle, tapi_response_cb callback, void *user_data )
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_get_mute_status((TelHandle *)handle,
		on_response_get_call_mute_status, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API  int tel_set_call_sound_recording(TapiHandle *handle, TelSoundRecording_t *on, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_set_sound_recording((TelHandle *)handle, *(TelCallSoundRecording *)on,
		on_response_set_call_sound_recording, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API  int tel_set_call_sound_equalization(TapiHandle *handle, TelCallSoundEqualization_t *eq, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelCallSoundEqualization eq_buf = {0, };
	unsigned int i;

	dbg("Func Entrance");

	eq_buf.direction = eq->direction;
	eq_buf.mode = eq->mode;
	for (i = 0; i < MAX_SOUND_EQ_PARAMETER_SIZE; i++)
		eq_buf.parameter[i] = eq->parameter[i];

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_call_set_sound_equalization((TelHandle *)handle, &eq_buf, on_response_set_call_sound_equalization, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API  int tel_set_call_sound_noise_reduction(TapiHandle *handle, TelSoundNoiseReduction_t noise, tapi_response_cb callback, void *user_data)
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}

EXPORT_API int tel_set_call_sound_clock_status(TapiHandle *handle, gboolean status, tapi_response_cb callback, void *user_data )
{
	err("Unsupported operation");
	return TAPI_API_NOT_SUPPORTED;
}

/*	EOF  */

