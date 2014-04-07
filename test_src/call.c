/*
 * Telephony test application
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
#include <sys/time.h>
#include <unistd.h>
#include <glib.h>
#include <glib-object.h>

#include <tapi_common.h>
#include <TelCall.h>
#include <ITapiCall.h>
#include <TapiUtility.h>

#include "menu.h"
#include "call.h"

static char data_call_type[MENU_DATA_SIZE + 1] = "0";
static char data_call_number[MENU_DATA_SIZE + 1] = "+821099777347";

static char data_answer_type[MENU_DATA_SIZE + 1] = "1";

static char data_call_id[MENU_DATA_SIZE + 1] = "1";
static char data_end_type[MENU_DATA_SIZE + 1] = "1";

static char data_dtmf_string[MENU_DATA_SIZE + 1] = "1";

static char data_call_active_line_type[MENU_DATA_SIZE + 1] = "1";

static char data_vol_device[MENU_DATA_SIZE + 1] = "0";
static char data_vol_type[MENU_DATA_SIZE + 1] = "1";
static char data_vol_level[MENU_DATA_SIZE + 1] = "1";

static char data_call_sound_path[MENU_DATA_SIZE + 1] = "0";
static char data_sound_ex_volume[MENU_DATA_SIZE + 1] = "0";

static char data_mute_status[MENU_DATA_SIZE + 1] = "0";

static char data_sound_recording_status[MENU_DATA_SIZE + 1] = "0";

static char data_sound_equalization_mode[MENU_DATA_SIZE + 1] = "0";
static char data_sound_direction[MENU_DATA_SIZE + 1] = "0";

static void on_noti_call_status_idle(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelCallStatusIdleNoti_t *noti_data = (TelCallStatusIdleNoti_t*)data;

	if (g_strcmp0(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_IDLE) == 0) {
		msg("Event (%s) received !!", TAPI_NOTI_VOICE_CALL_STATUS_IDLE);
	} else if (g_strcmp0(noti_id, TAPI_NOTI_VIDEO_CALL_STATUS_IDLE) == 0) {
		msg("Event (%s) received !!", TAPI_NOTI_VIDEO_CALL_STATUS_IDLE);
	} else {
		msg("Unregistered Event recieved!!");
		return;
	}

	if (!noti_data)
		return;

	msg("Id: [%d]", noti_data->id);
	msg("Cause: [0x%x]", noti_data->cause);
}

static void on_noti_call_status(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	int *call_id = data;

	msg("Event (%s) recieved !!", noti_id);

	if (call_id) {
		msg("Call id: [%d]", *call_id);
	} else {
		msg("Call Noti data is NULL.");
	}
}

static void on_noti_call_sound_ringback_tone(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelCallSoundRingbackToneNoti_t *noti_data = data;

	if (g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_RINGBACK_TONE) == 0) {
		msg("Event (%s) received !!", TAPI_NOTI_CALL_SOUND_RINGBACK_TONE);
	} else {
		msg("Unregistered Event recieved!!");
		return;
	}

	if (noti_data) {
		msg("Sound Ring-Back Tone: [%s]", (*noti_data == TAPI_CALL_SOUND_RINGBACK_TONE_START) ? "START" : "END");
	} else {
		msg("Call Sound Ringback Noti data is NULL");
	}
}

static void on_noti_call_sound_wbamr(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelCallSoundWbamrNoti_t *noti_data = data;

	if (g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_WBAMR) == 0) {
		msg("Event (%s) received !!", TAPI_NOTI_CALL_SOUND_WBAMR);
	} else {
		msg("Unregistered Event recieved!!");
		return;
	}

	if (noti_data) {
		msg(" AMR-WB status - %s", (*noti_data == TAPI_CALL_SOUND_WBAMR_STATUS_OFF) ? "OFF" : "ON");
 	} else {
		msg("Call Sound AMR-WB Noti data is NULL");
	}
}

static void on_noti_call_sound_equalization(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelCallSoundEqualizationNoti_t *noti_data = data;

	if (g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_EQUALIZATION) == 0) {
		msg("Event (%s) received !!", TAPI_NOTI_CALL_SOUND_EQUALIZATION);
	} else {
		msg("Unregistered Event recieved!!");
		return;
	}

	if (noti_data) {
		msg("Mode: [%d]", noti_data->mode);
		msg("Direction: [%d]", noti_data->direction);
	} else {
		msg("Call Sound Equalization Noti data is NULL");
	}
}

static void on_noti_call_sound_clock_status(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	gboolean *clock_status = data;

	if (g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_CLOCK_STATUS) == 0) {
		msg("Event (%s) received !!", TAPI_NOTI_CALL_SOUND_CLOCK_STATUS);
	} else {
		msg("Unregistered Event recieved!!");
		return;
	}

	if (clock_status) {
		msg("Clock_status[%d]", *clock_status);
	} else {
		msg("Call Sound Clock Status notification data is null");
	}
}

static void on_dial_call(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("on_dial_call() - result : 0x%x", result);
}

static int run_call_dial(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	TelCallDial_t data;
	int result;

	memset(&data, '\0', sizeof(TelCallDial_t));

	data.CallType = atoi(data_call_type);
	memcpy(data.szNumber, data_call_number, strlen(data_call_number));

	msg("call tel_dail_call()");

	result = tel_dial_call(handle, &data, on_dial_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_answer_call(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("on_answer_call() - result: [%d]", result);
	if (data) {
		TelCallAnswerCnf_t *cnf_data = (TelCallAnswerCnf_t*)data;
		msg("cnf_data->id : 0x%x", cnf_data->id);
	}
}

static int run_call_answer(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	unsigned int call_id;
	TelCallAnswerType_t answer_type;


	msg("call tel_answer_call()");

	call_id = atoi(data_call_id);
	answer_type = atoi(data_answer_type);

	result = tel_answer_call(handle, call_id, answer_type, on_answer_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_end_call(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("on_end_call() - result: [%d]", result);
	if (data) {
		TelCallEndCnf_t *cnf_data = (TelCallEndCnf_t*)data;
		msg("cnf_data->id : 0x%x", cnf_data->id);
	}
}

static int run_call_end(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	unsigned int call_id;
	TelCallEndType_t end_type;

	msg("call tel_end_call()");

	call_id = atoi(data_call_id);
	end_type = atoi(data_end_type);

	result = tel_end_call(handle, call_id, end_type, on_end_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_hold_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_hold_call() - result : 0x%x", result);
}

static int run_call_hold(MManager * mm, struct menu_data * menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	unsigned int call_id;

	msg("call tel_hold()");

	call_id = atoi(data_call_id);

	result = tel_hold_call(handle, call_id, on_hold_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_active_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_active_call() - result : 0x%x", result);
}

static int run_call_active(MManager * mm, struct menu_data * menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	unsigned int call_id;

	msg("call tel_active()");

	call_id = atoi(data_call_id);

	result = tel_active_call(handle, call_id, on_active_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_swap_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_swap_call() - result : 0x%x", result);
}

static int run_call_swap(MManager * mm, struct menu_data * menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	unsigned int call_id;

	msg("call tel_swap_call()");

	call_id = atoi(data_call_id);

	result = tel_swap_call(handle, call_id, call_id, on_swap_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_join_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_join_call() - result : 0x%x", result);
}

static int run_call_join(MManager * mm, struct menu_data * menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	unsigned int call_id;

	msg("call tel_join()");

	call_id = atoi(data_call_id);

	result = tel_join_call(handle, call_id, call_id, on_join_call, 0);
	CHECK_RESULT(result);
	return 0;
}

static void on_split_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_split_call() - result : 0x%x", result);
}

static int run_call_split(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	int call_id;

	call_id = atoi(data_call_id);

	msg("split call Id : (%d)", call_id);

	msg("call tel_split()");

	result = tel_split_call(handle, call_id, on_split_call, 0);
	CHECK_RESULT(result);
	return 0;
}

static void on_transfer_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_transfer_call() - result : 0x%x", result);
}

static int run_call_transfer(MManager * mm, struct menu_data * menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	int call_id;

	msg("call tel_transfer()");

	call_id = atoi(data_call_id);

	result = tel_transfer_call(handle, call_id, on_transfer_call, 0);
	CHECK_RESULT(result);
	return 0;
}

static void on_send_dtmf_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_send_dtmf_call() - result : 0x%x", result);
}

static int run_call_send_dtmf(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	const char *str = data_dtmf_string;

	msg("DTMF string : (%s)", str);

	msg("call tel_send_dtmf()");

	result = tel_call_dtmf(handle, str, on_send_dtmf_call, 0);
	CHECK_RESULT(result);
	return 0;
}

static void on_deflect_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_deflect_call() - result : 0x%x", result);
}

static int run_call_deflect(MManager * mm, struct menu_data * menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
 	const TelCallDeflectDstInfo_t deflect_to;
	int call_id;

	call_id = atoi(data_call_id);
	g_strlcpy((gchar *)deflect_to.number, data_call_number, MENU_DATA_SIZE + 1);

	msg("call deflect number : (%s)", deflect_to.number);

	result = tel_deflect_call(handle, call_id, &deflect_to, on_deflect_call, 0);
	CHECK_RESULT(result);
	return 0;
}

static int run_call_get_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	unsigned int call_id = 0;
	TelCallStatus_t info = {0,};

	msg("call tel_get_call_status()");

	call_id = atoi(data_call_id);

	result = tel_get_call_status(handle, call_id, &info);
	if (result != TAPI_API_SUCCESS) {
		msg("failed. (result = %d)", result);
		return 0;
	}

	msg(" - id = %d", info.CallHandle);
	msg(" - direction = %d", info.CallHandle);
	msg(" - number = %s", info.pNumber);
	msg(" - type = %d", info.CallType);
	msg(" - state = %d", info.CallState);
	msg(" - multiparty = %d", info.bConferenceState);

	return 0;
}

static void call_status_callback( TelCallStatus_t *status, void *user_data )
{
	if (status) {
		msg(" - id = %d", status->CallHandle);
		msg(" - direction = %d", status->CallHandle);
		msg(" - number = %s", status->pNumber);
		msg(" - type = %d", status->CallType);
		msg(" - state = %d", status->CallState);
		msg(" - multiparty = %d", status->bConferenceState);
	} else {
		msg("Call status is NULL");
	}
}

static int run_call_get_status_all(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result = 0;

	msg("call tel_get_call_status_all()");

	result = tel_get_call_status_all(handle, call_status_callback, 0);

	CHECK_RESULT(result);
	return 0;
}

static void on_get_active_line_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_get_active_line() - result : 0x%x", result);
	if (data) {
		TelCallActiveLine_t *act = data;
		msg("Active_line: %d", *act);
	}
}

static int run_call_get_active_line(MManager * mm, struct menu_data * menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_get_active_line()");

	result = tel_get_call_act_line(handle, on_get_active_line_call, 0);
	CHECK_RESULT(result);
	return 0;
}

static void on_set_active_line_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_set_active_line() - result : 0x%x", result);
}

static int run_call_set_active_line(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelCallActiveLine_t active_line;

	active_line = atoi(data_call_active_line_type);

	msg("active line type : (%d)", active_line);

	msg("call tel_set_active_line()");

	result = tel_set_call_act_line(handle, active_line, on_set_active_line_call, 0);
	CHECK_RESULT(result);
	return 0;
}

static void on_call_get_volume_info(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("on_call_get_volume_info() - result : 0x%x", result);

	if (data) {
		unsigned int i = 0;
		TelCallGetVolumeInfoResp_t *resp = (TelCallGetVolumeInfoResp_t *)data;

		msg("on_call_get_volume_info() - record_num : 0x%x", resp->record_num);

		for ( i=0; i<resp->record_num; i++ ) {
			msg("on_call_get_volume_info() - record[%d].device : 0x%x", (i+1), resp->record[i].device);
			msg("on_call_get_volume_info() - record[%d].type : 0x%x", (i+1), resp->record[i].device);
			msg("on_call_get_volume_info() - record[%d].volume : 0x%x", (i+1), resp->record[i].volume);

		}
	}
}

static int run_call_get_volume_info(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result = 0;
	TelSoundDevice_t device;
	TelSoundType_t type;

	msg("call tel_get_call_volume_info()");

	device = (TelSoundDevice_t)atoi(data_vol_device);
	type = (TelSoundType_t)atoi(data_vol_type);

	msg("device : (%d)", device);
	msg("type : (%d)", type);

	result = tel_get_call_volume_info(handle, device, type, on_call_get_volume_info, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_call_set_volume_info(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("on_call_set_volume_info() - result : 0x%x", result);
}

static int run_call_set_volume_info(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result = 0;
	TelCallVolumeInfo_t data;

	msg("call tel_set_call_volume_info()");

	data.device = (TelSoundDevice_t)atoi(data_vol_device);
	data.type = (TelSoundType_t)atoi(data_vol_type);
	data.volume = (TelSoundVolume_t)atoi(data_vol_level);

	msg("device : (%d)", data.device);
	msg("type : (%d)", data.type);
	msg("volume : (%d)", data.volume);

	result = tel_set_call_volume_info(handle, &data, on_call_set_volume_info, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_set_sound_path_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_set_sound_path() - result : 0x%x", result);
}

static int run_call_set_sound_path(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result = 0;
	TelCallSoundPathInfo_t sound_path_info;

	memset(&sound_path_info, '\0', sizeof(TelCallSoundPathInfo_t));

	sound_path_info.path = atoi(data_call_sound_path);
	sound_path_info.ex_volume = atoi(data_sound_ex_volume);

	msg("sound path : (%d)", sound_path_info.path);
	msg("sound ex-volume : (%d)", sound_path_info.ex_volume);

	msg("call tel_set_sound_path()");

	result = tel_set_call_sound_path(handle, &sound_path_info, on_set_sound_path_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_call_set_mute_status(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("on_call_set_mute_status() - result : 0x%x", result);
}

static int run_call_set_mute_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result = 0;
	TelSoundMuteStatus_t status = 0;

	msg("call tel_set_call_mute_status()");

	status = (TelSoundMuteStatus_t)atoi(data_mute_status);

	msg("status : (%d)", status);

	result = tel_set_call_mute_status(handle, status, on_call_set_mute_status, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_call_get_mute_status(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("on_call_get_mute_status() - result : 0x%x", result);
	if (data) {
		TelCallGetMuteStatusResp_t *resp = (TelCallGetMuteStatusResp_t *)data;
		msg("Mute Status: [%d]", resp->status);
	}
}

static int run_call_get_mute_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result = 0;

	msg("call tel_get_call_mute_status()");

	result = tel_get_call_mute_status(handle, on_call_get_mute_status, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_set_sound_recording_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_set_sound_recording() - result : 0x%x", result);
}

static int run_call_set_sound_recording(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result = 0;
	TelSoundRecording_t sound_rec;

	sound_rec = atoi(data_sound_recording_status);

	msg("sound_recording_status : (%d)", sound_rec);

	msg("call tel_set_sound_recording()");

	result = tel_set_call_sound_recording(handle, &sound_rec, on_set_sound_recording_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static void on_set_sound_equalization_call(TapiHandle * handle, int result, void *data, void *user_data)
{
	msg("on_set_sound_equalization() - result : 0x%x", result);
}

static int run_call_set_sound_equalization(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	TelCallSoundEqualization_t sound_eq;
	int result;

	memset(&sound_eq, '\0', sizeof(TelCallSoundEqualization_t));

	sound_eq.mode = atoi(data_sound_equalization_mode);
	sound_eq.direction = atoi(data_sound_direction);

	msg("sound eq mode : (%d)", sound_eq.mode);
	msg("sound eq direction : (%d)", sound_eq.direction);

	msg("call tel_set_sound_equalization()");

	result = tel_set_call_sound_equalization(handle, &sound_eq, on_set_sound_equalization_call, 0);

	CHECK_RESULT(result);

	return 0;
}

static struct menu_data menu_call_dial[] = {
	{ "1", "type (0=VOICE, 1=VIDEO, 2=Emergency)", 0, 0, data_call_type},
	{ "2", "number", 0, 0, data_call_number},
	{ "3", "run", 0, run_call_dial, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_answer[] = {
	{ "1", "id", 0, 0, data_call_id},
	{ "2", "type (0=ACCEPT, 1=REJECT, 2=REPLACE, 3=HOLD&ACCEPT)", 0, 0, data_answer_type},
	{ "3", "run", 0, run_call_answer, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_end[] = {
	{ "1", "id", 0, 0, data_call_id},
	{ "2", "type (0=END, 1=END ALL, 2=END ACTIVE ALL, 3=END HOLD ALL)", 0, 0, data_end_type},
	{ "3", "run", 0, run_call_end, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_hold[] = {
	{ "1", "run", NULL, run_call_hold, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_active[] = {
	{ "1", "run", NULL, run_call_active, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_swap[] = {
	{ "1", "run", NULL, run_call_swap, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_join[] = {
	{ "1", "run", NULL, run_call_join, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_split[] = {
	{ "1", "id", NULL, NULL, data_call_id},
	{ "2", "run", NULL, run_call_split, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_transfer[] = {
	{ "1", "run", 0, run_call_transfer, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_dtmf[] = {
	{ "1", "Dtmf Character (0-9, A-D, '*', and '#')", NULL, NULL, data_dtmf_string},
	{ "2", "run", NULL, run_call_send_dtmf, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_deflect[] = {
	{ "1", "deflect number", NULL, NULL, data_call_number},
	{ "2", "run", NULL, run_call_deflect, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_get_status[] = {
	{ "1", "id", 0, 0, data_call_id},
	{ "2", "run", 0, run_call_get_status, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_get_status_all[] = {
	{ "1", "id", 0, 0, data_call_id},
	{ "2", "run", 0, run_call_get_status_all, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_get_act_line[] = {
	{ "1", "run", NULL, run_call_get_active_line, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_set_act_line[] = {
	{ "1", "type (0=LINE1, 1=LINE2 )", NULL, NULL, data_call_active_line_type},
	{ "2", "run", NULL, run_call_set_active_line, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_get_volume_info[] = {
	{ "1", "device", 0, 0, data_vol_device},
	{ "2", "type", 0, 0, data_vol_type},
	{ "3", "run", 0, run_call_get_volume_info, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_set_volume_info[] = {
	{ "1", "device", 0, 0, data_vol_device},
	{ "2", "type", 0, 0, data_vol_type},
	{ "3", "volume", 0, 0, data_vol_level},
	{ "4", "run", 0, run_call_set_volume_info, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_set_sound_path[] = {
	{ "1", "path (0=HANDSET, 1=HEADSET, 2=HANDSFREE, 3=BLUETOOTH, 4=STEREO_BLUETOOTH, 5=SPEAKER_PHONE, 6=HEADSET_3_5PI, 7=BLUETOOTH_NSEC_OFF, 8=MIC2, 9=MIC1, 10=HEADSET_HAC)", NULL, NULL, data_call_sound_path},
	{ "2", "ex_volume (0=OFF 1=ON)", NULL, NULL, data_sound_ex_volume},
	{ "3", "run", NULL, run_call_set_sound_path, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_set_mute[] = {
	{ "1", "status", 0, 0, data_mute_status},
	{ "3", "run", 0, run_call_set_mute_status, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_get_mute_status[] = {
	{ "1", "run", 0, run_call_get_mute_status, 0},
	{ 0, 0, },
};

static struct menu_data menu_call_set_sound_recording[] = {
	{ "1", "recording (0=STOP, 1=START, 2=PAUSE)", NULL, NULL, data_sound_recording_status},
	{ "2", "run", NULL, run_call_set_sound_recording, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_call_set_sound_equalization[] = {
	{ "1", "mode (0=OFF, 1=ON, 2=FLAG_OFF, 3=FLAG_ON, 4=SOFT1, 5=SOFT2)", NULL, NULL, data_sound_equalization_mode},
	{ "2", "direction (0=LEFT, 1=RIGHT)", NULL, NULL, data_sound_direction} ,
	{ "3", "run", NULL, run_call_set_sound_equalization, NULL},
	{ NULL, NULL, },
};

struct menu_data menu_call[] = {
	{ "1", "tel_dial_call", menu_call_dial, NULL, NULL},
	{ "2", "tel_answer_call", menu_call_answer, NULL, NULL},
	{ "3", "tel_end_call", menu_call_end, NULL, NULL},
	{ "4", "tel_hold_call", menu_call_hold, NULL, NULL},
	{ "5", "tel_active_call", menu_call_active, NULL, NULL},
	{ "6", "tel_swap_call", menu_call_swap, NULL, NULL},
	{ "7", "tel_join_call", menu_call_join, NULL, NULL},
	{ "8", "tel_split_call", menu_call_split, NULL, NULL},
	{ "9", "tel_transfer_call", menu_call_transfer, NULL, NULL},
	{ "10", "tel_call_dtmf", menu_call_dtmf, NULL, NULL},
	{ "11", "tel_deflect_call", menu_call_deflect, NULL, NULL},
	{ "12", "tel_get_call_status", menu_call_get_status, NULL, NULL},
	{ "12a", "tel_get_call_status_all", menu_call_get_status_all, NULL, NULL},
	{ "13g", "tel_get_call_act_line", menu_call_get_act_line, NULL, NULL},
	{ "13s", "tel_set_call_act_line", menu_call_set_act_line, NULL, NULL},
	{ "14g", "tel_get_call_volume_info", menu_call_get_volume_info, NULL, NULL},
	{ "14s", "tel_set_call_volume_info", menu_call_set_volume_info, NULL, NULL},
	{ "15", "tel_set_call_sound_path", menu_call_set_sound_path, NULL, NULL},
	{ "16s", "tel_set_call_mute_status", menu_call_set_mute, NULL, NULL},
	{ "16g", "tel_get_call_mute_status", menu_call_get_mute_status, NULL, NULL},
	{ "17", "tel_set_call_sound_recording", menu_call_set_sound_recording, NULL, NULL},
	{ "18", "tel_set_call_sound_equalization", menu_call_set_sound_equalization, NULL, NULL},
	{ NULL, NULL, },
};

void register_call_event(TapiHandle *handle)
{
	int ret;

	ret = tel_register_noti_event(handle, TAPI_NOTI_VOICE_CALL_STATUS_IDLE, on_noti_call_status_idle, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VOICE_CALL_STATUS_IDLE - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VIDEO_CALL_STATUS_IDLE, on_noti_call_status_idle, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VIDEO_CALL_STATUS_IDLE - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VOICE_CALL_STATUS_ACTIVE, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VOICE_CALL_STATUS_ACTIVE - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VOICE_CALL_STATUS_HELD, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VOICE_CALL_STATUS_HELD - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VOICE_CALL_STATUS_DIALING, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VOICE_CALL_STATUS_DIALING - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VOICE_CALL_STATUS_ALERT, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VOICE_CALL_STATUS_ALERT - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VOICE_CALL_STATUS_INCOMING, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VOICE_CALL_STATUS_INCOMING - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VIDEO_CALL_STATUS_ACTIVE, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VIDEO_CALL_STATUS_ACTIVE - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VIDEO_CALL_STATUS_DIALING, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VIDEO_CALL_STATUS_DIALING - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VIDEO_CALL_STATUS_ALERT, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VIDEO_CALL_STATUS_ALERT - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_VIDEO_CALL_STATUS_INCOMING, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_VIDEO_CALL_STATUS_INCOMING - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_WAITING, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_MO_WAITING - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_HELD, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_HELD - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_ACTIVE, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_ACTIVE - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_JOINED, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_JOINED - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_FORWARDED, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_MO_FORWARDED - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_BARRED_INCOMING, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_MO_BARRED_INCOMING - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_BARRED_OUTGOING, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_MO_BARRED_OUTGOING - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_FORWARD_UNCONDITIONAL, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_MO_FORWARD_UNCONDITIONAL - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_FORWARD_CONDITIONAL, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_MO_FORWARD_CONDITIONAL - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_INFO_TRANSFERED, on_noti_call_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_INFO_TRANSFERED - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_SOUND_RINGBACK_TONE, on_noti_call_sound_ringback_tone, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_SOUND_RINGBACK_TONE - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_SOUND_WBAMR, on_noti_call_sound_wbamr, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_SOUND_WBAMR - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_SOUND_EQUALIZATION, on_noti_call_sound_equalization, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_SOUND_EQUALIZATION - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_CALL_SOUND_CLOCK_STATUS, on_noti_call_sound_clock_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_CALL_SOUND_CLOCK_STATUS - register event failed: [%d]", ret);
}
