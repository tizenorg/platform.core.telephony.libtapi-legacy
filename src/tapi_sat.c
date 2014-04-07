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
#include "TelSat.h"

#include "common.h"
#include "tapi_log.h"
#include "ITapiSat.h"

#include <tel_return.h>
#include <tapi_sat.h>

extern TelSatEventDownloadType_t g_event_list[TAPI_SAT_EVENT_LIST_MAX_COUNT];

static void _tapi_sat_gen_app_exec_result_data(TelSatAppsRetInfo_t *result_info, TelSatAppsRetInfo *app_ret_info_buf)
{
	switch(result_info->commandType)
	{
		case TAPI_SAT_CMD_TYPE_SETUP_MENU:{
			app_ret_info_buf->apps_ret.setup_menu_resp = result_info->appsRet.setupMenu.resp;
		} break;
		case TAPI_SAT_CMD_TYPE_REFRESH:{
			app_ret_info_buf->apps_ret.refresh.app_type = result_info->appsRet.refresh.appType;
			app_ret_info_buf->apps_ret.refresh.resp = result_info->appsRet.refresh.resp;
		} break;
		case TAPI_SAT_CMD_TYPE_SETUP_CALL:{
			app_ret_info_buf->apps_ret.setup_call.resp = result_info->appsRet.setupCall.resp;
			app_ret_info_buf->apps_ret.setup_call.me_problem = result_info->appsRet.setupCall.meProblem;
			app_ret_info_buf->apps_ret.setup_call.permanent_call_ctrl_problem = result_info->appsRet.setupCall.permanentCallCtrlProblem;
			app_ret_info_buf->apps_ret.setup_call.call_cause = result_info->appsRet.setupCall.tapiCause;
		} break;
		case TAPI_SAT_CMD_TYPE_SEND_SS:{
			app_ret_info_buf->apps_ret.send_ss.resp = result_info->appsRet.sendSs.resp;
			app_ret_info_buf->apps_ret.send_ss.me_problem = result_info->appsRet.sendSs.meProblem;
			app_ret_info_buf->apps_ret.send_ss.ss_cause = result_info->appsRet.sendSs.ssCause;
			app_ret_info_buf->apps_ret.send_ss.additional_call_ctrl_problem_info = result_info->appsRet.sendSs.additionalCallCtrlProblemInfo;
			app_ret_info_buf->apps_ret.send_ss.ss_string.string_len = result_info->appsRet.sendSs.ssString.stringLen;
			memcpy(app_ret_info_buf->apps_ret.send_ss.ss_string.string, result_info->appsRet.sendSs.ssString.string, result_info->appsRet.sendSs.ssString.stringLen);
		} break;
		case TAPI_SAT_CMD_TYPE_SEND_USSD:{
			app_ret_info_buf->apps_ret.send_ussd.resp = result_info->appsRet.sendUssd.resp;
			app_ret_info_buf->apps_ret.send_ussd.me_problem = result_info->appsRet.sendUssd.meProblem;
			app_ret_info_buf->apps_ret.send_ussd.ss_cause = result_info->appsRet.sendUssd.ssCause;
			app_ret_info_buf->apps_ret.send_ussd.ussd_string.string_len = result_info->appsRet.sendUssd.ussdString.stringLen;
			memcpy(app_ret_info_buf->apps_ret.send_ussd.ussd_string.string, result_info->appsRet.sendUssd.ussdString.string, result_info->appsRet.sendUssd.ussdString.stringLen);
		} break;
		case TAPI_SAT_CMD_TYPE_SEND_SMS:{
			app_ret_info_buf->apps_ret.send_sms_resp = result_info->appsRet.sendSms.resp;
		} break;
		case TAPI_SAT_CMD_TYPE_SEND_DTMF:{
			app_ret_info_buf->apps_ret.send_dtmf_resp = result_info->appsRet.sendDtmf.resp;
		} break;
		case TAPI_SAT_CMD_TYPE_LAUNCH_BROWSER:{
			app_ret_info_buf->apps_ret.launch_browser.resp = result_info->appsRet.launchBrowser.resp;
			app_ret_info_buf->apps_ret.launch_browser.browser_problem = result_info->appsRet.launchBrowser.browserProblem;
		} break;
		case TAPI_SAT_CMD_TYPE_SETUP_IDLE_MODE_TEXT:{
			app_ret_info_buf->apps_ret.setup_idle_mode_text_resp = result_info->appsRet.setupIdleModeText.resp;
		} break;
		case TAPI_SAT_CMD_TYPE_LANGUAGE_NOTIFICATION:{
			app_ret_info_buf->apps_ret.language_noti_resp = result_info->appsRet.languageNoti.resp;
		} break;
		case TAPI_SAT_CMD_TYPE_PROVIDE_LOCAL_INFO:{
			app_ret_info_buf->apps_ret.provide_local_info.resp = result_info->appsRet.provideLocalInfo.resp;
		} break;
		case TAPI_SAT_CMD_TYPE_PLAY_TONE:{
			app_ret_info_buf->apps_ret.play_tone_resp = result_info->appsRet.playTone.resp;
		} break;
		case TAPI_SAT_CMD_TYPE_DISPLAY_TEXT:{
			app_ret_info_buf->apps_ret.display_text.resp = result_info->appsRet.displayText.resp;
			app_ret_info_buf->apps_ret.display_text.me_problem = result_info->appsRet.displayText.meProblem;
		} break;
		default:
			dbg("unhandled command type(0x%x", result_info->commandType);
			break;
	}
}

static void on_response_menu_selection_envelop(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	int *envelop_rsp;

	envelop_rsp = (int *)data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, result, envelop_rsp, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_download_event_envelop(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	int *envelop_rsp;

	result = -1;
	envelop_rsp = (int *)data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, result, envelop_rsp, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

/**
 *
 * To send envelope command (MENU SELECTION) to USIM.
 *
 * @return		values in enum TapiResult_t.
 * @param[in]		TelSatMenuSelectionReqInfo_t	menu selection evelope data.
 * @Interface		Asynchronous.
 * @remark
 * @Refer		TelSatMenuSelectionReqInfo_t
 */

EXPORT_API int tel_select_sat_menu(TapiHandle *handle, const TelSatMenuSelectionReqInfo_t *pMenuSelect, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSatMenuSelectionReqInfo menu_select;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	menu_select.item_identifier = pMenuSelect->itemIdentifier;
	menu_select.is_help_requested = ( (pMenuSelect->bIsHelpRequested != 0) ? TRUE : FALSE);

	ret = tapi_sat_select_menu((TelHandle *)handle, (TelSatMenuSelectionReqInfo *)&menu_select, on_response_menu_selection_envelop, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

/**
 *
 * API to send event download to SIM.
 *
 * @return		values in enum TapiResult_t.
 * @param[in]		TelSatEventDownloadReqInfo_t	event download data.
 * @Interface		Asynchronous.
 * @remark
 * @Refer		TelSatEventDownloadReqInfo_t
 */
EXPORT_API int tel_download_sat_event(TapiHandle *handle, const TelSatEventDownloadReqInfo_t *pEventData, tapi_response_cb callback, void *user_data)
{
	gboolean evt_check = FALSE;
	int g_index = 0;
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSatEventDownloadReqInfo event_data;

	dbg("Func Entrance ");

	if (pEventData->eventDownloadType != TAPI_EVENT_SAT_DW_TYPE_BROWSER_TERMINATION) {
		for (g_index = 0; g_event_list[g_index] > 0; g_index++) {
			if (g_event_list[g_index] == pEventData->eventDownloadType) {
				dbg("event (%d) shoud be passed to sim", pEventData->eventDownloadType);
				evt_check = TRUE;
				break;
			}
		}
	}
	else
		evt_check = TRUE;

	if (!evt_check) {
		dbg("Request (%d) not initiated from SIM", pEventData->eventDownloadType);
		return TAPI_API_SAT_EVENT_NOT_REQUIRED_BY_USIM;
	}

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	event_data.event_download_type = pEventData->eventDownloadType;
	switch(pEventData->eventDownloadType)
	{
		case TAPI_EVENT_SAT_DW_TYPE_IDLE_SCREEN_AVAILABLE:{
			dbg("idle screen available (%d)", pEventData->u.bIdleScreenAvailable);
			event_data.event_download_data.is_idle_screen_available = ( (pEventData->u.bIdleScreenAvailable != 0) ? TRUE : FALSE);
		} break;
		case TAPI_EVENT_SAT_DW_TYPE_LANGUAGE_SELECTION:{
			dbg("selected language (%d)", pEventData->u.languageSelectionEventReqInfo.language);
			event_data.event_download_data.language_selection_event_req_info = pEventData->u.languageSelectionEventReqInfo.language;
		} break;
		case TAPI_EVENT_SAT_DW_TYPE_BROWSER_TERMINATION:{
			dbg("Cause of browser termination (%d)", pEventData->u.browserTerminationEventReqInfo.browserTerminationCause);
			event_data.event_download_data.browser_termination_event_req_info = pEventData->u.browserTerminationEventReqInfo.browserTerminationCause;
		} break;
		case TAPI_EVENT_SAT_DW_TYPE_DATA_AVAILABLE:
		case TAPI_EVENT_SAT_DW_TYPE_CHANNEL_STATUS:
		default :
			dbg("not support download event (%d)", pEventData->eventDownloadType);
			break;
	}

	ret = tapi_sat_download_event((TelHandle *)handle, (TelSatEventDownloadReqInfo *)&event_data, on_response_download_event_envelop, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);

}

/**
 *
 * API to get main menu information.
 *
 * @return		values in enum TapiResult_t.
 * @param[out]		TelSatSetupMenuInfo_t	main menu data.
 * @Interface		Synchronous.
 * @remark
 * @Refer		TelSatSetupMenuInfo_t
 */
EXPORT_API int tel_get_sat_main_menu_info(TapiHandle *handle, TelSatSetupMenuInfo_t *pMainMenu)
{
	TelReturn ret;
	TelSatSetupMenuInfo main_menu;
	int i;

	dbg("Func Entrance ");

	ret = tapi_sat_get_main_menu_info((TelHandle *)handle, (TelSatSetupMenuInfo *)&main_menu);

	if (ret == TEL_RETURN_SUCCESS) {
		pMainMenu->commandId = main_menu.command_id;
		pMainMenu->bIsMainMenuPresent = (main_menu.is_main_menu_present ? 1 : 0);
		memcpy(pMainMenu->satMainTitle, main_menu.sat_main_title, TAPI_SAT_DEF_TITLE_LEN_MAX+1);
		pMainMenu->satMainMenuNum = main_menu.sat_main_menu_item_count;

		for(i=0;i<main_menu.sat_main_menu_item_count;i++){
			pMainMenu->satMainMenuItem[i].itemId = main_menu.sat_main_menu_item[i].item_id;
			memcpy(pMainMenu->satMainMenuItem[i].itemString, main_menu.sat_main_menu_item[i].item_string, TAPI_SAT_DEF_ITEM_STR_LEN_MAX + 6);
			dbg("item index(%d) id(%d) str(%s)", i+1, pMainMenu->satMainMenuItem[i].itemId, pMainMenu->satMainMenuItem[i].itemString);
		}


		pMainMenu->bIsSatMainMenuHelpInfo = (main_menu.is_sat_main_menu_help_info ? 1 : 0);
		pMainMenu->bIsUpdatedSatMainMenu = (main_menu.is_updated_sat_main_menu ? 1 : 0);
	}

	dbg("result (%d)", ret);
	dbg("command id (%d)", pMainMenu->commandId);
	dbg("menu present (%d)", pMainMenu->bIsMainMenuPresent);
	dbg("menu title (%s)", pMainMenu->satMainTitle);
	dbg("item cnt (%d)", pMainMenu->satMainMenuNum);
	dbg("menu help info (%d)", pMainMenu->bIsSatMainMenuHelpInfo);
	dbg("menu updated (%d)", pMainMenu->bIsUpdatedSatMainMenu);

	CHECK_RETURN(ret, NULL);
}

/**
 *
 * API to send UI display status.
 *
 * @return		values in enum TapiResult_t.
 * @param[in]		TelSatUiDisplayStatusType_t	display status data.
 command_id	command identifier
 * @Interface		Asynchronous.
 * @remark
 * @Refer		TelSatUiDisplayStatusType_t
 */
EXPORT_API int tel_send_sat_ui_display_status(TapiHandle *handle, int commandId, TelSatUiDisplayStatusType_t status)
{
	TelReturn ret;
	TelSatUiDisplayStatusType status_buf;

	dbg("Func Entrance ");

	status_buf = status;
	ret = tapi_sat_send_ui_display_status((TelHandle *)handle, commandId, status_buf);

	dbg("result (%d)", ret);
	CHECK_RETURN(ret, NULL);
}

/**
 *
 * API to send UI user confirmation data.
 *
 * @return		values in enum TapiResult_t.
 * @param[in]		command_id	command identifier
 command_type		command type
 key_type			user confirmation value
 addtional_data		addtional data
 data_len			data length
 * @Interface		Asynchronous.
 * @remark
 * @Refer
 */
EXPORT_API int tel_send_sat_ui_user_confirm(TapiHandle *handle, TelSatUiUserConfirmInfo_t *pUserConfirmData)
{
	TelReturn ret;
	TelSatUiUserConfirmInfo user_confirm_data_buf;

	dbg("Func Entrance ");

	user_confirm_data_buf.command_id = pUserConfirmData->commandId;
	user_confirm_data_buf.command_type = pUserConfirmData->commandType;
	user_confirm_data_buf.key_type = pUserConfirmData->keyType;
	user_confirm_data_buf.additional_data = pUserConfirmData->pAdditionalData;
	user_confirm_data_buf.data_len = pUserConfirmData->dataLen;

	ret = tapi_sat_send_ui_user_confirm((TelHandle *)handle, (TelSatUiUserConfirmInfo *)&user_confirm_data_buf);

	dbg("result (%d)", ret);
	CHECK_RETURN(ret, NULL);
}

/**
 *
 * API to send application excution result.
 *
 * @return		values in enum TapiResult_t.
 * @param[in]		TelSatAppsRetInfo_t	application result information data.
 * @Interface		Asynchronous.
 * @remark
 * @Refer		TelSatAppsRetInfo_t
 */
EXPORT_API int tel_send_sat_app_exec_result(TapiHandle *handle, TelSatAppsRetInfo_t *pAppRetInfo)
{
	TelReturn ret;
	TelSatAppsRetInfo app_ret_info_buf;

	dbg("Func Entrance ");

	app_ret_info_buf.command_id = pAppRetInfo->commandId;
	app_ret_info_buf.command_type = pAppRetInfo->commandType;
	_tapi_sat_gen_app_exec_result_data(pAppRetInfo, &app_ret_info_buf);

	ret = tapi_sat_send_app_exec_result((TelHandle *)handle, (TelSatAppsRetInfo *)&app_ret_info_buf);

	dbg("result (%d)", ret);
	CHECK_RETURN(ret, NULL);
}
