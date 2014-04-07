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

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "tapi_common.h"
#include "TapiUtility.h"

#include "ITapiPS.h"
#include "TelNetwork.h"
#include "TelSms.h"
#include "TelSat.h"
#include "TelSs.h"
#include "TelCall.h"
#include "TelPower.h"

#include "common.h"
#include "tapi_log.h"

#include <tel_return.h>
#include <tel_network.h>
#include <tel_sim.h>
#include <tel_ss.h>
#include <tel_sms.h>
#include <tel_call.h>
#include <tel_gps.h>
#include <tel_phonebook.h>
#include <tel_modem.h>
#include <tel_sat.h>
#include <tel_sap.h>

#include <tapi_events.h>
#include <tapi.h>

TelSatEventDownloadType_t g_event_list[TAPI_SAT_EVENT_LIST_MAX_COUNT] = {-1};

#define CALLBACK_CALL(data) \
	if (evt_cb_data->cb_fn) { \
		evt_cb_data->cb_fn((TapiHandle *)handle, \
			evt_cb_data->evt_id, data, evt_cb_data->user_data); \
	}

static GHashTable *evt_table = NULL;

static int __map_result(TelReturn ret)
{
	switch(ret) {
		case TEL_RETURN_SUCCESS :
			return TAPI_API_SUCCESS;
		case TEL_RETURN_INVALID_PARAMETER :
			return TAPI_API_INVALID_INPUT;
		case TEL_RETURN_MEMORY_FAILURE :
			return TAPI_API_SYSTEM_OUT_OF_MEM;
		case TEL_RETURN_OPERATION_NOT_SUPPORTED :
			return TAPI_API_NOT_SUPPORTED;
		default :
			return TAPI_API_OPERATION_FAILED;
	}
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

static void __sms_asciistring_to_hex(const char *ascii_string,
			unsigned char *hex_string, unsigned int ascii_len)
{
	unsigned int i;

	if (ascii_len % 2 == 0) { /* Make sure ASCII len is even */
		for(i = 0; i < ascii_len; i += 2) {
			hex_string[i/2] = (char)(((
				__sms_hexchar_to_int(ascii_string[i + 1]) << 4) |
				__sms_hexchar_to_int(ascii_string[i])));
		}
	} else { /* Odd ASCII len */
		for (i = 0; i < ascii_len; i += 2) {
			if (i == ascii_len - 1) {
				hex_string[i / 2] = (char)(0xF0 |
					__sms_hexchar_to_int(ascii_string[i]));
			} else {
				hex_string[i / 2] = (char)(((
					__sms_hexchar_to_int(ascii_string[i + 1]) << 4) |
					__sms_hexchar_to_int(ascii_string[i])));
			}
		}
	}
}

static gboolean __map_network_act(TelNetworkAct act_buf, TelNetworkSystemType_t *act)
{
	gboolean ret = TRUE;
	switch(act_buf) {
		case TEL_NETWORK_ACT_UNKNOWN :
			*act = TAPI_NETWORK_SYSTEM_NO_SRV;
			break;
		case TEL_NETWORK_ACT_GSM :
			*act = TAPI_NETWORK_SYSTEM_GSM;
			break;
		case TEL_NETWORK_ACT_GPRS :
			*act = TAPI_NETWORK_SYSTEM_GPRS;
			break;
		case TEL_NETWORK_ACT_EGPRS :
			*act = TAPI_NETWORK_SYSTEM_EGPRS;
			break;
		case TEL_NETWORK_ACT_UMTS :
		case TEL_NETWORK_ACT_HSUPA :
		case TEL_NETWORK_ACT_HSPA :
			/*HSUPA and HSPA are not present in the Legacy code.
			* So mapped to UMTS as it is in close proximity*/
			*act = TAPI_NETWORK_SYSTEM_UMTS;
			break;
		case TEL_NETWORK_ACT_GSM_AND_UMTS :
			*act = TAPI_NETWORK_SYSTEM_GSM_AND_UMTS;
			break;
		case TEL_NETWORK_ACT_HSDPA :
			*act = TAPI_NETWORK_SYSTEM_HSDPA;
			break;
		case TEL_NETWORK_ACT_LTE :
			*act = TAPI_NETWORK_SYSTEM_LTE;
			break;
		default :
			err("Unidentified ACT. Unknown error");
			ret = FALSE;
	}
	return ret;
}

static gboolean __map_modem_power_status(TelModemPowerStatus status_in,
		tapi_power_phone_power_status_t *status_out)
{
	switch (status_in) {
	case TEL_MODEM_POWER_OFF:
		*status_out = TAPI_PHONE_POWER_STATUS_OFF;
	case TEL_MODEM_POWER_ON:
		*status_out = TAPI_PHONE_POWER_STATUS_ON;
	case TEL_MODEM_POWER_ERROR:
		*status_out = TAPI_PHONE_POWER_STATUS_ERROR;
	}

	return TRUE;
}

static int __map_sim_card_status(TelSimCardStatus status)
{
	switch(status) {
		case TEL_SIM_STATUS_CARD_ERROR :
			return TAPI_SIM_STATUS_CARD_ERROR;
		case TEL_SIM_STATUS_CARD_NOT_PRESENT :
			return TAPI_SIM_STATUS_CARD_NOT_PRESENT;
		case TEL_SIM_STATUS_SIM_INITIALIZING :
			return TAPI_SIM_STATUS_SIM_INITIALIZING;
		case TEL_SIM_STATUS_SIM_INIT_COMPLETED :
			return TAPI_SIM_STATUS_SIM_INIT_COMPLETED;
		case TEL_SIM_STATUS_SIM_PIN_REQUIRED :
			return TAPI_SIM_STATUS_SIM_PIN_REQUIRED;
		case TEL_SIM_STATUS_SIM_PUK_REQUIRED :
			return TAPI_SIM_STATUS_SIM_PUK_REQUIRED;
		case TEL_SIM_STATUS_CARD_BLOCKED :
			return TAPI_SIM_STATUS_CARD_BLOCKED;
		case TEL_SIM_STATUS_SIM_NCK_REQUIRED :
			return TAPI_SIM_STATUS_SIM_NCK_REQUIRED;
		case TEL_SIM_STATUS_SIM_NSCK_REQUIRED :
			return TAPI_SIM_STATUS_SIM_NSCK_REQUIRED;
		case TEL_SIM_STATUS_SIM_SPCK_REQUIRED :
			return TAPI_SIM_STATUS_SIM_SPCK_REQUIRED;
		case TEL_SIM_STATUS_SIM_CCK_REQUIRED :
			return TAPI_SIM_STATUS_SIM_CCK_REQUIRED;
		case TEL_SIM_STATUS_CARD_REMOVED :
			return TAPI_SIM_STATUS_CARD_REMOVED;
		case TEL_SIM_STATUS_SIM_LOCK_REQUIRED :
			return TAPI_SIM_STATUS_SIM_LOCK_REQUIRED;
		case TEL_SIM_STATUS_UNKNOWN :
		case TEL_SIM_STATUS_SIM_CARD_POWEROFF :
			return TAPI_SIM_STATUS_UNKNOWN;
		default :
			err("Invalid card status [%d]", status);
			return -1;
	}
}

static TelTapiEndCause_t __map_call_end_cause(TelCallEndCause cause)
{
	switch(cause) {
		case TEL_CALL_END_CAUSE_FAILED :
			return TAPI_CALL_END_NO_CAUSE;
		case TEL_CALL_END_CAUSE_UNASSIGNED_NUMBER :
			return TAPI_CC_CAUSE_UNASSIGNED_NUMBER;
		case TEL_CALL_END_CAUSE_NO_ROUTE_TO_DEST :
			return TAPI_CC_CAUSE_NO_ROUTE_TO_DEST;
		case TEL_CALL_END_CAUSE_OPERATOR_DETERMINED_BARRING :
			return TAPI_CC_CAUSE_OPERATOR_DETERMINED_BARRING;
		case TEL_CALL_END_CAUSE_NORMAL_CALL_CLEARING :
			return TAPI_CC_CAUSE_NORMAL_CALL_CLEARING;
		case TEL_CALL_END_CAUSE_USER_BUSY :
			return TAPI_CC_CAUSE_USER_BUSY;
		case TEL_CALL_END_CAUSE_NO_USER_RESPONDING :
			return TAPI_CC_CAUSE_NO_USER_RESPONDING;
		case TEL_CALL_END_CAUSE_USER_ALERTING_NO_ANSWER :
			return TAPI_CC_CAUSE_USER_ALERTING_NO_ANSWER;
		case TEL_CALL_END_CAUSE_CALL_REJECTED :
			return TAPI_CC_CAUSE_CALL_REJECTED;
		case TEL_CALL_END_CAUSE_NUMBER_CHANGED :
			return TAPI_CC_CAUSE_NUMBER_CHANGED;
		case TEL_CALL_END_CAUSE_DESTINATION_OUT_OF_ORDER :
			return TAPI_CC_CAUSE_DESTINATION_OUT_OF_ORDER;
		case TEL_CALL_END_CAUSE_INVALID_NUMBER_FORMAT :
			return TAPI_CC_CAUSE_INVALID_NUMBER_FORMAT;
		case TEL_CALL_END_CAUSE_FACILITY_REJECTED :
			return TAPI_CC_CAUSE_FACILITY_REJECTED;
		case TEL_CALL_END_CAUSE_NO_CIRCUIT_CHANNEL_AVAILABLE :
			return TAPI_CC_CAUSE_NO_CIRCUIT_CHANNEL_AVAILABLE;
		case TEL_CALL_END_CAUSE_NETWORK_OUT_OF_ORDER :
			return TAPI_CC_CAUSE_NETWORK_OUT_OF_ORDER;
		case TEL_CALL_END_CAUSE_TEMPORARY_FAILURE :
			return TAPI_CC_CAUSE_TEMPORARY_FAILURE;
		case TEL_CALL_END_CAUSE_SWITCHING_EQUIPMENT_CONGESTION :
			return TAPI_CC_CAUSE_SWITCHING_EQUIPMENT_CONGESTION;
		case TEL_CALL_END_CAUSE_REQUESTED_CIRCUIT_CHANNEL_NOT_AVAILABLE :
			return TAPI_CC_CAUSE_REQUESTED_CIRCUIT_CHANNEL_NOT_AVAILABLE;
		case TEL_CALL_END_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED :
			return TAPI_CC_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED;
		case TEL_CALL_END_CAUSE_ACM_GEQ_ACMMAX :
			return TAPI_CC_CAUSE_ACM_GEQ_ACMMAX;
		case TEL_CALL_END_CAUSE_IMEI_NOT_ACCEPTED :
			return TAPI_REJECT_CAUSE_IMEI_NOT_ACCEPTED;
		case TEL_CALL_END_CAUSE_NETWORK_FAILURE :
			return TAPI_REJECT_CAUSE_NETWORK_FAILURE;
		case TEL_CALL_END_CAUSE_CONGESTION :
			return TAPI_REJECT_CAUSE_CONGESTTION;
		case TEL_CALL_END_CAUSE_SERVICE_OPTION_OUT_OF_ORDER :
			return TAPI_REJECT_CAUSE_SERVICE_OPT__OUT_OF_ORDER;
		case TEL_CALL_END_CAUSE_ACCESS_CLASS_BLOCKED :
			return TAPI_REJECT_CAUSE_MM_REJ_ACCESS_CLASS_BLOCKED;
		default :
			return TAPI_CALL_END_NO_CAUSE;
	}
}

static void __process_network_event(TelHandle *handle, const char *evt_id,
				void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}
	dbg("evt_id[%s]", evt_id);

	if (!g_strcmp0(evt_id, TEL_NOTI_NETWORK_REGISTRATION_STATUS)) {
		TelNetworkRegStatusInfo *reg_status = data;
		struct tel_noti_network_registration_status noti = {0, };

		noti.cs = reg_status->cs_status;
		noti.ps = reg_status->ps_status;
		if (__map_network_act(reg_status->act,
			(TelNetworkSystemType_t *)&noti.type) == FALSE) {
			err("map network act failed");
			return;
		}

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_NETWORK_CELL_INFO)) {
		TelNetworkCellInfo *cell_info = data;
		struct tel_noti_network_cell_info noti = {0, };

		noti.cell_id = cell_info->cell_id;
		noti.lac = cell_info->lac;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_NETWORK_TIME_INFO)) {
		TelNetworkNitzInfoNoti *time_info = data;
		struct tel_noti_network_time_info noti = {0, };

		noti.day = (int)time_info->day;
		noti.dstoff = time_info->dstoff;
		noti.gmtoff = time_info->gmtoff;
		noti.hour = (int)time_info->hour;
		noti.isdst = time_info->isdst;
		noti.minute = (int)time_info->minute;
		noti.month = (int)time_info->month;
		g_strlcpy(noti.plmn, time_info->plmn, TAPI_NETWORK_PLMN_LEN_MAX + 1);
		noti.second = (int)time_info->second;
		noti.year = (int)time_info->year;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_NETWORK_IDENTITY)) {
		TelNetworkIdentityInfo *identity = data;
		struct tel_noti_network_identity noti = {{0, }, };

		g_strlcpy(noti.plmn, identity->plmn, TAPI_NETWORK_PLMN_LEN_MAX + 1);
		g_strlcpy(noti.short_name, identity->short_name, 17);
		g_strlcpy(noti.full_name, identity->long_name, 33);

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_NETWORK_RSSI)) {
		struct tel_noti_network_info noti = {0, };

		noti.rssi = *(int *)data;

		CALLBACK_CALL(&noti);
	} else {
		err("Unhandled Network noti[%s]", evt_id);
	}
}

static void __process_sms_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}
	dbg("evt_id[%s]", evt_id);

	if (!g_strcmp0(evt_id, TEL_NOTI_SMS_INCOM_MSG)) {
		TelSmsDatapackageInfo *incoming_msg = data;
		struct tel_noti_sms_incomming_msg noti = {{0, }, };

		noti.MsgLength = incoming_msg->tpdu_length;
		memcpy(noti.szData, incoming_msg->tpdu, noti.MsgLength);

		if (incoming_msg->sca.number[0] != '\0') {
			noti.Sca[0] = strlen(incoming_msg->sca.number);
			noti.Sca[1] = ((incoming_msg->sca.ton << 4) & 0xF0) |
					(incoming_msg->sca.npi & 0x0F);
			__sms_asciistring_to_hex(
					(const char *)incoming_msg->sca.number,
					(unsigned char *)&noti.Sca[2],
					(unsigned int)noti.Sca[0]);
		} else {
			err("sca number is null");
			return;
		}

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_SMS_CB_INCOM_MSG)) {
		TelSmsCbMsgInfo *cb_msg = data;
		struct tel_noti_sms_incomming_cb_msg noti = {0, };

		noti.CbMsgType = cb_msg->cb_type + 1;
		noti.Length = (short)cb_msg->length;
		memcpy(noti.szMsgData, cb_msg->cb_data, noti.Length);

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_SMS_ETWS_INCOM_MSG)) {
		TelSmsEtwsMsgInfo *etws_msg = data;
		struct tel_noti_sms_incomming_etws_msg noti = {0, };

		noti.EtwsMsgType = etws_msg->etws_type;
		noti.Length = (short)etws_msg->length;
		memcpy(noti.szMsgData, etws_msg->etws_data, noti.Length);

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_SMS_SIM_MEMORY_STATUS)) {
		int noti = *(int *)data;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_SMS_INIT_STATUS)) {
		gboolean noti = *(gboolean *)data;

		CALLBACK_CALL(&noti);
	} else {
		err("Unhandled Sms noti[%s]", evt_id);
	}
}

static void __process_call_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}
	dbg("evt_id[%s]", evt_id);

	if (!g_strcmp0(evt_id, TEL_NOTI_VOICE_CALL_STATUS_IDLE) ||
			!g_strcmp0(evt_id, TEL_NOTI_VIDEO_CALL_STATUS_IDLE)) {
		TelCallStatusIdleNoti *idle_noti = data;
		TelCallStatusIdleNoti_t noti = {0, };

		noti.id = idle_noti->call_id;
		noti.cause = __map_call_end_cause(idle_noti->cause);

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_VOICE_CALL_STATUS_DIALING) ||
			!g_strcmp0(evt_id, TEL_NOTI_VIDEO_CALL_STATUS_DIALING)) {
		TelCallStatusDialingNoti_t noti = {0, };
		noti.id = *(unsigned int *)data;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_VOICE_CALL_STATUS_ALERT) ||
			!g_strcmp0(evt_id, TEL_NOTI_VIDEO_CALL_STATUS_ALERT)) {
		TelCallStatusAlertNoti_t noti = {0, };
		noti.id = *(unsigned int *)data;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_VOICE_CALL_STATUS_ACTIVE) ||
			!g_strcmp0(evt_id, TEL_NOTI_VIDEO_CALL_STATUS_ACTIVE)) {
		TelCallStatusActiveNoti_t noti = {0, };
		noti.id = *(unsigned int *)data;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_VOICE_CALL_STATUS_HELD) ) {
		TelCallStatusHeldNoti_t noti = {0, };
		noti.id = *(unsigned int *)data;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_VOICE_CALL_STATUS_INCOMING) ||
			!g_strcmp0(evt_id, TEL_NOTI_VIDEO_CALL_STATUS_INCOMING)) {
		TelCallStatusIncomingNoti_t noti = {0, };
		noti.id = *(unsigned int *)data;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_MO_WAITING)) {
		TelCallInfoWaitingNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_MO_FORWARDED)) {
		TelCallInfoForwardedNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_MO_BARRED_INCOMING)) {
		TelCallInfoBarredIncomingNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_MO_BARRED_OUTGOING)) {
		TelCallInfoBarredOutgoingNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_MO_FORWARD_CONDITIONAL)) {
		TelCallInfoForwardConditionalNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_MO_FORWARD_UNCONDITIONAL)) {
		TelCallInfoForwardUnconditionalNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_ACTIVE)) {
		TelCallInfoActiveNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_HELD)) {
		TelCallInfoHeldNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_INFO_JOINED)) {
		TelCallInfoJoinedNoti_t noti = {0, };
		noti.id = 1; /* Sending default call_id, as we are not sending any call_id in tizen 3.0 for this notification */

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_SOUND_RINGBACK_TONE)) {
		TelCallSoundRingbackToneNoti_t noti = *(TelCallSoundRingbackToneNoti_t *)data;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_SOUND_WBAMR)) {
		TelCallSoundWbamrNoti_t noti = *(TelCallSoundWbamrNoti_t *)data;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_SOUND_EQUALIZATION)) {
		TelCallSoundEqualizationNoti *equ_noti = data;
		TelCallSoundEqualizationNoti_t noti = {0 , };

		noti.mode = equ_noti->mode;
		noti.direction = equ_noti->direction;

		CALLBACK_CALL(&noti);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_CALL_SOUND_CLOCK_STATUS)) {
		gboolean noti = *(gboolean *)data;

		CALLBACK_CALL(&noti);
	} else {
		dbg("Unhandled Call noti[%s]", evt_id);
	}
}

static void _process_sat_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}
	dbg("evt_id[%s]", evt_id);

	if (!g_strcmp0(evt_id, TEL_NOTI_SAT_SETUP_MENU)) {
		TelSatSetupMenuInfo_t noti;
		TelSatSetupMenuInfo *main_menu = (TelSatSetupMenuInfo *)data;
		int i;

		noti.commandId = main_menu->command_id;
		noti.bIsMainMenuPresent = (main_menu->is_main_menu_present ? 1 : 0);
		memcpy(noti.satMainTitle, main_menu->sat_main_title, TAPI_SAT_DEF_TITLE_LEN_MAX+1);
		noti.satMainMenuNum = main_menu->sat_main_menu_item_count;
		for(i=0;i<main_menu->sat_main_menu_item_count;i++){
			noti.satMainMenuItem[i].itemId = main_menu->sat_main_menu_item[i].item_id;
			memcpy(noti.satMainMenuItem[i].itemString, main_menu->sat_main_menu_item[i].item_string, TAPI_SAT_DEF_ITEM_STR_LEN_MAX + 6);
			dbg("item index(%d) id(%d) str(%s)", i+1, noti.satMainMenuItem[i].itemId, noti.satMainMenuItem[i].itemString);
		}
		noti.bIsSatMainMenuHelpInfo = (main_menu->is_sat_main_menu_help_info ? 1 : 0);
		noti.bIsUpdatedSatMainMenu = (main_menu->is_updated_sat_main_menu ? 1 : 0);

		dbg("command id (%d)", noti.commandId);
		dbg("menu present (%d)", noti.bIsMainMenuPresent);
		dbg("menu title (%s)", noti.satMainTitle);
		dbg("item cnt (%d)", noti.satMainMenuNum);
		dbg("menu help info (%d)", noti.bIsSatMainMenuHelpInfo);
		dbg("menu updated (%d)", noti.bIsUpdatedSatMainMenu);

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_DISPLAY_TEXT)) {
		TelSatDisplayTextInd_t noti;
		TelSatDisplayTextInd *display_text = (TelSatDisplayTextInd *)data;

		noti.commandId = display_text->command_id;
		memcpy(noti.text.string, display_text->text.string, TAPI_SAT_DEF_TEXT_STRING_LEN_MAX+1);
		noti.text.stringLen = display_text->text.string_len;
		noti.duration = display_text->duration;
		noti.bIsPriorityHigh = (display_text->is_priority_high ? 1 : 0);
		noti.bIsUserRespRequired = (display_text->is_user_resp_required ? 1 : 0);
		noti.b_immediately_resp = (display_text->immediately_resp ? 1 : 0);

		dbg("command id (%d)", noti.commandId);
		dbg("display text (%s)", noti.text.string);
		dbg("string len(%d)", noti.text.stringLen);
		dbg("duration (%d)", noti.duration);
		dbg("high priority (%d)", noti.bIsPriorityHigh);
		dbg("user response required(%d)", noti.bIsUserRespRequired);
		dbg("immediately response (%d)", noti.b_immediately_resp);

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_SELECT_ITEM))  {
		TelSatSelectItemInd_t noti;
		TelSatSelectItemInd *select_item = (TelSatSelectItemInd *) data;
		int i;

		noti.commandId = select_item->command_id;
		noti.bIsHelpInfoAvailable = (select_item->is_help_info_available ? 1 : 0);
		memcpy(noti.text.string, select_item->text.string, TAPI_SAT_DEF_TITLE_LEN_MAX+1);
		noti.text.stringLen = select_item->text.string_len;
		noti.defaultItemIndex = select_item->default_item_index;
		noti.menuItemCount = select_item->menu_item_count;

		for(i=0;i<select_item->menu_item_count;i++){
				noti.menuItem[i].itemId = select_item->menu_item[i].item_id;
				noti.menuItem[i].textLen = select_item->menu_item[i].text_len;
				memcpy(noti.menuItem[i].text, select_item->menu_item[i].text, TAPI_SAT_ITEM_TEXT_LEN_MAX + 1);
				dbg("item index(%d) id(%d) len(%d) str(%s)", i+1,
						noti.menuItem[i].itemId, noti.menuItem[i].textLen, noti.menuItem[i].text);
		}

		dbg("command id (%d)", noti.commandId);
		dbg("help info(%d)", noti.bIsHelpInfoAvailable);
		dbg("selected item string(%s)", noti.text.string);
		dbg("string len(%d)", noti.text.stringLen);
		dbg("default item index(%d)", noti.defaultItemIndex);
		dbg("item count(%d)", noti.menuItemCount);

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_GET_INKEY))  {
		TelSatGetInkeyInd_t noti;
		TelSatGetInkeyInd *get_inkey = (TelSatGetInkeyInd *) data;

		noti.commandId = get_inkey->command_id;
		noti.keyType = get_inkey->key_type;
		noti.inputCharMode = get_inkey->input_char_mode;
		noti.bIsNumeric = (get_inkey->is_numeric ? 1 : 0);
		noti.bIsHelpInfoAvailable = (get_inkey->is_help_info_available ? 1 : 0);
		memcpy(noti.text.string, get_inkey->text.string, TAPI_SAT_DEF_TEXT_STRING_LEN_MAX+1);
		noti.text.stringLen = get_inkey->text.string_len;
		noti.duration = get_inkey->duration;

		dbg("command id(%d)", noti.commandId);
		dbg("key type(%d)", noti.keyType);
		dbg("input character mode(%d)", noti.inputCharMode);
		dbg("numeric(%d)", noti.bIsNumeric);
		dbg("help info available(%d)", noti.bIsHelpInfoAvailable);
		dbg("text (%s)", noti.text.string);
		dbg("text length", noti.text.stringLen);
		dbg("duration", noti.duration);

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_GET_INPUT))  {
		TelSatGetInputInd_t noti;
		TelSatGetInputInd *get_input = (TelSatGetInputInd *) data;

		noti.commandId = get_input->command_id;
		noti.inputCharMode = get_input->input_char_mode;
		noti.bIsNumeric = (get_input->is_numeric ? 1 : 0);
		noti.bIsHelpInfoAvailable = (get_input->is_help_info_available ? 1 : 0);
		noti.bIsEchoInput = (get_input->is_echo_input ? 1 : 0);
		memcpy(noti.text.string, get_input->text.string, TAPI_SAT_DEF_TEXT_STRING_LEN_MAX+1);
		noti.text.stringLen = get_input->text.string_len;
		noti.respLen.max = get_input->resp_len.max;
		noti.respLen.min = get_input->resp_len.min;
		memcpy(noti.defaultText.string, get_input->default_text.string, TAPI_SAT_DEF_TEXT_STRING_LEN_MAX+1);
		noti.defaultText.stringLen = get_input->default_text.string_len;

		dbg("command id(%d)", noti.commandId);
		dbg("input character mode(%d)", noti.inputCharMode);
		dbg("numeric(%d)", noti.bIsNumeric);
		dbg("help info avaiable(%d)", noti.bIsHelpInfoAvailable);
		dbg("echo input(%d)", noti.bIsEchoInput);
		dbg("text(%s)", noti.text.string);
		dbg("text length(%d)", noti.text.stringLen);
		dbg("response length max(%d)", noti.respLen.max);
		dbg("response length min(%d)", noti.respLen.min);
		dbg("default text(%s)", noti.defaultText.string);
		dbg("default text length(%d)", noti.defaultText.stringLen);

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_SEND_SMS))  {
		TelSatSendSmsIndSmsData_t noti;
		TelSatSendSmsIndSmsData *send_sms = (TelSatSendSmsIndSmsData *) data;

		noti.commandId = send_sms->command_id;
		noti.bIsPackingRequired = (send_sms->is_packing_required ? 1 : 0);

		noti.address.ton = send_sms->address.ton;
		noti.address.npi = send_sms->address.npi;
		noti.address.diallingNumberLen = send_sms->address.dialing_number_len;
		memcpy(noti.address.diallingNumber, send_sms->address.dialing_number, TAPI_SAT_DIALLING_NUMBER_LEN_MAX);

		noti.smsTpdu.tpduType = send_sms->sms_tpdu.tpdu_type;
		noti.smsTpdu.dataLen = send_sms->sms_tpdu.data_len;
		memcpy(noti.smsTpdu.data, send_sms->sms_tpdu.data, noti.smsTpdu.dataLen);

		dbg("command id(%d)", noti.commandId);
		dbg("packing required(%d)", noti.bIsPackingRequired);
		dbg("address ton(%d)", noti.address.ton);
		dbg("address npi(%d)", noti.address.npi);
		dbg("address dialing number (%s)", noti.address.diallingNumber);
		dbg("address number length (%d)", noti.address.diallingNumberLen);
		dbg("tpdu type (%d)", noti.smsTpdu.tpduType);
		dbg("tpdu length (%d)", noti.smsTpdu.dataLen);

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_SETUP_EVENT_LIST))  {
		int g_index = 0;
		TelSatEventListData_t noti;
		TelSatEventListData *event_list = (TelSatEventListData *) data;

		if(event_list->is_idle_screen_available) {
			noti.bIsIdleScreenAvailable = 1;
			dbg("g_index(%d) event(%d)", g_index, data);
			g_event_list[g_index] = TAPI_EVENT_SAT_DW_TYPE_IDLE_SCREEN_AVAILABLE;
			g_index++;
		}
		else if(event_list->is_language_selection) {
			noti.bIsLanguageSelection = 1;
			dbg("g_index(%d) event(%d)", g_index, data);
			g_event_list[g_index] = TAPI_EVENT_SAT_DW_TYPE_LANGUAGE_SELECTION;
			g_index++;
		}
		else if(event_list->is_browser_termination) {
			noti.bIsBrowserTermination = 1;
			dbg("g_index(%d) event(%d)", g_index, data);
			g_event_list[g_index] = TAPI_EVENT_SAT_DW_TYPE_BROWSER_TERMINATION;
			g_index++;
		}
		else if(event_list->is_data_available) {
			noti.bIsDataAvailable = 1;
			dbg("g_index(%d) event(%d)", g_index, data);
			g_event_list[g_index] = TAPI_EVENT_SAT_DW_TYPE_DATA_AVAILABLE;
			g_index++;
		}
		else if(event_list->is_channel_status) {
			noti.bIsChannelStatus = 1;
			dbg("g_index(%d) event(%d)", g_index, data);
			g_event_list[g_index] = TAPI_EVENT_SAT_DW_TYPE_CHANNEL_STATUS;
			g_index++;
		}

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_REFRESH)) {
		TelSatRefreshInd_t noti;
		TelSatRefreshInd *refresh_info = (TelSatRefreshInd *) data;

		noti.commandId = refresh_info->command_id;
		noti.refreshMode = refresh_info->refresh_mode;
		//noti.fileId = refresh_info->file_id;
		noti.fileCount = refresh_info->file_count;

		dbg("refresh event/file cnt(%d)", noti.fileCount);

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_SEND_DTMF)) {
		TelSatSendDtmfIndDtmfData_t noti;
		TelSatSendDtmfIndDtmfData *send_dtmf = (TelSatSendDtmfIndDtmfData *) data;

		noti.commandId = send_dtmf->command_id;
		noti.bIsHiddenMode = (send_dtmf->is_hidden_mode? 1 : 0);

		noti.dtmfString.stringLen = send_dtmf->dtmf_string.string_len;
		memcpy(noti.dtmfString.string, send_dtmf->dtmf_string.string, TAPI_SAT_DEF_TEXT_STRING_LEN_MAX+1);

		dbg("dtmf event command id(%d)", noti.commandId);
		dbg("dtmf event dtmf(%s)", noti.dtmfString.string);

		CALLBACK_CALL(&noti);
	}
	else if (!g_strcmp0(evt_id, TEL_NOTI_SAT_SESSION_END_EVENT))  {
		dbg("end session evt ");
	}
	else {
		dbg("not handled Sat noti[%s]", evt_id);
	}
}

static void __process_sim_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}
	dbg("evt_id[%s]", evt_id);

	if (!g_strcmp0(evt_id, TEL_NOTI_SIM_STATUS)) {
		TelSimCardStatusInfo *status_info = data;
		int noti = __map_sim_card_status(status_info->status);
		if (noti == -1)
			return;

		CALLBACK_CALL(&noti);
	} else {
		err("Unhandled SIM noti[%s]",evt_id );
	}
}

static void __process_pb_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}

	if (!g_strcmp0(evt_id, TEL_NOTI_PB_STATUS)) {
		TelPbInitInfo *noti = data;

		CALLBACK_CALL(noti);
	} else {
		err("Unhandled Phonebook noti[%s]",evt_id );
	}
}

static void __process_sap_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}

	if (!g_strcmp0(evt_id, TEL_NOTI_SAP_STATUS)) {
		int noti = *(int *)data;

		CALLBACK_CALL(&noti);
	} else {
		err("Unhandled SAP noti[%s]",evt_id );
	}
}

static void __process_modem_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}
	dbg("evt_id[%s]", evt_id);

	if (!g_strcmp0(evt_id, TEL_NOTI_MODEM_POWER_STATUS)) {
		TelModemPowerStatus *status = data;
		tapi_power_phone_power_status_t noti;

		__map_modem_power_status(*status, &noti);

		CALLBACK_CALL(&noti);
	} else {
		err("Unhandled Modem noti[%s]", evt_id);
	}
}

static void __process_ss_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}
	dbg("evt_id[%s]", evt_id);

	if (!g_strcmp0(evt_id, TEL_NOTI_SS_USSD)) {
		TelSsUssdNoti *ussd_noti = data;
		TelSsUssdMsgInfo_t noti = {0, };

		if (ussd_noti->str) {
			/*
			 * noti.Type not present in Tizen 3.0 structure.
			 * Taking user_init as default
			 */
			noti.Type = TAPI_SS_USSD_TYPE_USER_INIT;
			g_strlcpy(noti.szString,
				(const gchar *)ussd_noti->str,
				TAPI_SS_USSD_DATA_SIZE_MAX);
			noti.Length = strlen(noti.szString);
		} else {
			err("ussd string is null");
			return;
		}
		CALLBACK_CALL(&noti);
	} else {
		err("Unhandled SS noti[%s]", evt_id);
	}
}

static void __process_gps_event(TelHandle *handle, const char *evt_id,
			void *data, struct tapi_evt_cb *evt_cb_data)
{
	if (!evt_cb_data) {
		err("event callback data is null");
		return;
	}
	dbg("evt_id[%s]", evt_id);

	if (!g_strcmp0(evt_id, TEL_NOTI_GPS_ASSIST_DATA)
			|| !g_strcmp0(evt_id, TEL_NOTI_GPS_MEASURE_POSITION)) {
		TelGpsDataInfo *gps_data = data;
		unsigned char *decoded_data = gps_data->data;

		CALLBACK_CALL(decoded_data);
	} else if (!g_strcmp0(evt_id, TEL_NOTI_GPS_RESET_ASSIST_DATA)) {
		/* noting to decode */
	} else {
		dbg("Unhandled Gps noti[%s]", evt_id);
	}
}

static void on_signal_callback(TelHandle *handle, const char *evt_id,
			void *data, void *user_data)
{
	struct tapi_evt_cb *evt_cb_data = user_data;
	char **interface_name = NULL;

	interface_name = g_strsplit(evt_id, ":", 2);

	if (!interface_name) {
		err("invalid evt_id");
		return;
	}

	if (!interface_name[0] || !interface_name[1]) {
		g_strfreev(interface_name);
		err("invalid evt_id");
		return;
	}

	if (!g_strcmp0(interface_name[0], TELEPHONY_NETWORK_INTERFACE)) {
		__process_network_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_SMS_INTERFACE)) {
		__process_sms_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_CALL_INTERFACE)) {
		__process_call_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_SAT_INTERFACE)) {
		_process_sat_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_SIM_INTERFACE)) {
		__process_sim_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_PB_INTERFACE)) {
		__process_pb_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_SAP_INTERFACE)) {
		__process_sap_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_MODEM_INTERFACE)) {
		__process_modem_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_SS_INTERFACE)) {
		__process_ss_event(handle, evt_id, data, evt_cb_data);
	}
	else if (!g_strcmp0(interface_name[0], TELEPHONY_GPS_INTERFACE)) {
		__process_gps_event(handle, evt_id, data, evt_cb_data);
	}
	else {
		err("can't find interface(%s)", interface_name[0]);
	}
	g_strfreev(interface_name);
}

EXPORT_API int tel_get_property_int(TapiHandle *handle, const char *property, int *result)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}

static char *__map_noti_id_to_buf(const char *noti_id)
{
	/*Network*/
	if (!g_strcmp0(noti_id, TAPI_NOTI_NETWORK_REGISTRATION_STATUS))
		return TEL_NOTI_NETWORK_REGISTRATION_STATUS;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_NETWORK_CELLINFO))
		return TEL_NOTI_NETWORK_CELL_INFO;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_NETWORK_TIMEINFO))
		return TEL_NOTI_NETWORK_TIME_INFO;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_NETWORK_IDENTITY))
		return TEL_NOTI_NETWORK_IDENTITY;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_NETWORK_INFO))
		return TEL_NOTI_NETWORK_RSSI;
	/*SMS*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SMS_INCOM_MSG))
		return TEL_NOTI_SMS_INCOM_MSG;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SMS_CB_INCOM_MSG))
		return TEL_NOTI_SMS_CB_INCOM_MSG;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SMS_ETWS_INCOM_MSG))
		return TEL_NOTI_SMS_ETWS_INCOM_MSG;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SMS_MEMORY_STATUS))
		return TEL_NOTI_SMS_SIM_MEMORY_STATUS;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SMS_DEVICE_READY))
		return TEL_NOTI_SMS_INIT_STATUS;
	/*CALL*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_IDLE))
		return TEL_NOTI_VOICE_CALL_STATUS_IDLE;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_ACTIVE))
		return TEL_NOTI_VOICE_CALL_STATUS_ACTIVE;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_HELD))
		return TEL_NOTI_VOICE_CALL_STATUS_HELD;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_DIALING))
		return TEL_NOTI_VOICE_CALL_STATUS_DIALING;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_ALERT))
		return TEL_NOTI_VOICE_CALL_STATUS_ALERT;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_INCOMING))
		return TEL_NOTI_VOICE_CALL_STATUS_INCOMING;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VIDEO_CALL_STATUS_IDLE))
		return TEL_NOTI_VIDEO_CALL_STATUS_IDLE;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VIDEO_CALL_STATUS_ACTIVE))
		return TEL_NOTI_VIDEO_CALL_STATUS_ACTIVE;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VIDEO_CALL_STATUS_DIALING))
		return TEL_NOTI_VIDEO_CALL_STATUS_DIALING;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VIDEO_CALL_STATUS_ALERT))
		return TEL_NOTI_VIDEO_CALL_STATUS_ALERT;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VIDEO_CALL_STATUS_INCOMING))
		return TEL_NOTI_VIDEO_CALL_STATUS_INCOMING;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_WAITING))
		return TEL_NOTI_CALL_INFO_MO_WAITING;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_FORWARDED))
		return TEL_NOTI_CALL_INFO_MO_FORWARDED;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_BARRED_INCOMING))
		return TEL_NOTI_CALL_INFO_MO_BARRED_INCOMING;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_BARRED_OUTGOING))
		return TEL_NOTI_CALL_INFO_MO_BARRED_OUTGOING;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_FORWARD_UNCONDITIONAL))
		return TEL_NOTI_CALL_INFO_MO_FORWARD_UNCONDITIONAL;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_FORWARD_CONDITIONAL))
		return TEL_NOTI_CALL_INFO_MO_FORWARD_CONDITIONAL;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_DEFLECTED))
		return TEL_NOTI_CALL_INFO_MO_DEFLECTED;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_FORWARDED_CALL))
		return TEL_NOTI_CALL_INFO_MT_FORWARDED;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_DEFLECTED_CALL))
		return TEL_NOTI_CALL_INFO_MT_DEFLECTED;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_TRANSFERED))
		return TEL_NOTI_CALL_INFO_TRANSFERED;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_JOINED))
		return TEL_NOTI_CALL_INFO_JOINED;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_ACTIVE))
		return TEL_NOTI_CALL_INFO_ACTIVE;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_HELD))
		return TEL_NOTI_CALL_INFO_HELD;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_CLOCK_STATUS))
		return TEL_NOTI_CALL_SOUND_CLOCK_STATUS;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_EQUALIZATION))
		return TEL_NOTI_CALL_SOUND_EQUALIZATION;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_WBAMR))
		return TEL_NOTI_CALL_SOUND_WBAMR;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_RINGBACK_TONE))
		return TEL_NOTI_CALL_SOUND_RINGBACK_TONE;
	/* Register notifications which Call app currently register
	    TODO: Need to be check removing these notification on application side*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_WAITING))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_CALL_CONNECTED_LINE))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_CUG))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_CLIR_SUPPRESSION_REJECT))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_CALL_LINE_IDENTITY))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_CALL_NAME_INFORMATION))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_CUG_CALL))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_RELEASED_ON_HOLD))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_TRANSFER_ALERT))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_CF_CHECK_MESSAGE))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_NOISE_REDUCTION))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_SOUND_CLOCK_STATUS))
		return "Not support";
	else if (!g_strcmp0(noti_id, TAPI_NOTI_CALL_INFO_TRANSFERED_CALL))
		return "Not support";
	/*SAT*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_SETUP_MENU))
			return TEL_NOTI_SAT_SETUP_MENU;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_DISPLAY_TEXT))
			return TEL_NOTI_SAT_DISPLAY_TEXT;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_SELECT_ITEM))
			return TEL_NOTI_SAT_SELECT_ITEM;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_GET_INKEY))
			return TEL_NOTI_SAT_GET_INKEY;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_GET_INPUT))
			return TEL_NOTI_SAT_GET_INPUT;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_REFRESH))
			return TEL_NOTI_SAT_REFRESH;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_SEND_SMS))
			return TEL_NOTI_SAT_SEND_SMS;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_SETUP_EVENT_LIST))
			return TEL_NOTI_SAT_SETUP_EVENT_LIST;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_SEND_DTMF))
			return TEL_NOTI_SAT_SEND_DTMF;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAT_SESSION_END_EVENT))
			return TEL_NOTI_SAT_SESSION_END_EVENT;

	/*SIM*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SIM_STATUS))
		return TEL_NOTI_SIM_STATUS;
	/*Phonebook*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_PB_STATUS))
		return TEL_NOTI_PB_STATUS;
	/*SAP*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SAP_STATUS))
		return TEL_NOTI_SAP_STATUS;
	/*Modem*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_MODEM_POWER))
		return TEL_NOTI_MODEM_POWER_STATUS;
	/*SS*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_SS_USSD))
		return TEL_NOTI_SS_USSD;
	/*GPS*/
	else if (!g_strcmp0(noti_id, TAPI_NOTI_GPS_ASSIST_DATA))
		return TEL_NOTI_GPS_ASSIST_DATA;
	else if (!g_strcmp0(noti_id, TAPI_IND_GPS_MEASURE_POSITION))
		return TEL_NOTI_GPS_MEASURE_POSITION;
	else if (!g_strcmp0(noti_id, TAPI_NOTI_GPS_RESET_ASSIST_DATA))
		return TEL_NOTI_GPS_RESET_ASSIST_DATA;

	return NULL;
}

EXPORT_API char **tel_get_cp_name_list(void)
{
	TelReturn ret;
	char **cp_list = NULL;

	dbg("Func Entrance");

	if ((ret = tapi_get_cp_name_list(&cp_list)) != TEL_RETURN_SUCCESS)
		err("result [%d]", ret);

	return cp_list;
}

EXPORT_API TapiHandle *tel_init(const char *cp_name)
{
	TelHandle *tapi_handle;

	dbg("Func Entrance");

	tapi_handle = tapi_init(cp_name);
	if (tapi_handle != NULL) {
		/* Create Hash Table */
		evt_table = g_hash_table_new_full(g_str_hash, g_str_equal,
				g_free, g_free);
	}

	return (TapiHandle *)tapi_handle;
}

EXPORT_API int tel_deinit(TapiHandle *handle)
{
	TelReturn ret;

	dbg("Func Entrance");

	ret = tapi_deinit((TelHandle *)handle);

	/* Destroy Hash Table */
	g_hash_table_destroy(evt_table);
	evt_table = NULL;

	return (__map_result(ret));
}

EXPORT_API int tel_register_noti_event(TapiHandle *handle, const char *noti_id,
	tapi_notification_cb callback, void *user_data)
{
	TelReturn ret;
	struct tapi_evt_cb *evt_cb_data= NULL;
	char *noti_id_buf = NULL;

	dbg("Func Entrance");

	noti_id_buf = __map_noti_id_to_buf(noti_id);
	if (noti_id_buf == NULL) {
		err("Unsupported event [%s]", noti_id);
		return TAPI_API_NOT_SUPPORTED;
	} else if (g_strcmp0(noti_id_buf, "Not support") == 0){
		err("Unsupported event [%s]", noti_id);
		return TAPI_API_SUCCESS;
	}
	evt_cb_data = g_new0(struct tapi_evt_cb, 1);
	evt_cb_data->cb_fn = callback;
	evt_cb_data->user_data = user_data;
	evt_cb_data->evt_id = g_strdup(noti_id);

	ret = tapi_register_event_id((TelHandle *)handle, noti_id_buf,
				on_signal_callback, (void *)evt_cb_data);

	if (ret != TEL_RETURN_SUCCESS) {
		g_free(evt_cb_data->evt_id);
		g_free(evt_cb_data);
	} else {
		g_hash_table_insert(evt_table,
			evt_cb_data->evt_id, evt_cb_data);
	}

	return (__map_result(ret));
}

EXPORT_API int tel_deregister_noti_event(TapiHandle *handle, const char *noti_id)
{
	TelReturn ret;
	char *noti_id_buf = NULL;

	dbg("Func Entrance");

	if (g_hash_table_remove(evt_table, noti_id) == FALSE) {
		err("noti event (%s) not registered", noti_id);
		return TAPI_API_OPERATION_FAILED;
	}

	noti_id_buf = __map_noti_id_to_buf(noti_id);
	ret = tapi_deregister_event_id((TelHandle *)handle, noti_id_buf);

	return (__map_result(ret));
}
/*	EOF	*/
