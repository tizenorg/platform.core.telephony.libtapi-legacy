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
#include <ITapiSs.h>
#include <TapiUtility.h>

#include "menu.h"
#include "ss.h"

static char data_ss_set_barring_class[MENU_DATA_SIZE + 1] = "16";
static char data_ss_set_barring_mode[MENU_DATA_SIZE + 1] = "0";
static char data_ss_set_barring_type[MENU_DATA_SIZE + 1] = "1";
static char data_ss_set_barring_password[MENU_DATA_SIZE + 1] = "1111";

static char data_ss_get_barring_status_class[MENU_DATA_SIZE + 1] = "16";
static char data_ss_get_barring_status_type[MENU_DATA_SIZE + 1] = "1";

static char data_ss_change_barring_password_old[MENU_DATA_SIZE + 1] = "1111";
static char data_ss_change_barring_password_new[MENU_DATA_SIZE + 1] = "1234";

static char data_ss_set_forward_class[MENU_DATA_SIZE + 1] = "16";
static char data_ss_set_forward_mode[MENU_DATA_SIZE + 1] = "1";
static char data_ss_set_forward_condition[MENU_DATA_SIZE + 1] = "5";
static char data_ss_set_forward_timer[MENU_DATA_SIZE + 1] = "5";
static char data_ss_set_forward_number[MENU_DATA_SIZE + 1] = "01030018655";

static char data_ss_get_forward_status_class[MENU_DATA_SIZE + 1] = "16";
static char data_ss_get_forward_status_condition[MENU_DATA_SIZE + 1] = "1";

static char data_ss_set_waiting_class[MENU_DATA_SIZE + 1] = "16";
static char data_ss_set_waiting_mode[MENU_DATA_SIZE + 1] = "0";

static char data_ss_get_waiting_status_class[MENU_DATA_SIZE + 1] = "16";

static char data_ss_get_cli_status_type[MENU_DATA_SIZE + 1] = "1";

static char data_ss_ussd_request_type[MENU_DATA_SIZE + 1] = "1";
static char data_ss_ussd_request_string[MENU_DATA_SIZE + 1] = "";

static void on_noti_ss_ussd(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelSsUssdMsgInfo_t *noti = data;

	msgb("Event(%s) received !!", TAPI_NOTI_SS_USSD);

	if (!noti) {
		msg("Notification info is missing");
		return;
	}

	msg("Type: [0x%x]", noti->Type);
	msg("Length: [%d]", noti->Length);
	msg("szString: [%s]", noti->szString);
}

static void on_noti_ss_release_complete(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelSsRelCompMsgInfo_t *info = data;

	msgb("Event(%s) received !!", TAPI_NOTI_SS_RELEASE_COMPLETE);

	if (!info) {
		msg("Notification info is missing");
		return;
	}

	msg("Release Message Length: [%d]", info->RelCompMsgLen);

	info->szRelCompMsg[info->RelCompMsgLen + 1] = '\0';

	msg("Release complete message: [%s]", info->szRelCompMsg);
}

static void on_ss_barring(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSsBarringResp_t *resp = data;
	int i = 0;

	msgb("tel_set_ss_barring() or tel_get_ss_barring_status() response receive");
	msg("Result: [%d]", result);

	if (!resp) {
		msg("Response is NULL");
		return;
	}

	msg("Total Records: [%d]", resp->record_num);

	for (i = 0; i < resp->record_num; i++) {
		msg("Record:%d ----> Class: [%d], Status: [%d], Flavour: [%d]",
				i+1,
				resp->record[i].Class,
				resp->record[i].Status,
				resp->record[i].Flavour);
	}
}

static void on_ss_change_barring_password(TapiHandle *handle, int result, void *data, void *user_data)
{
	msgb("tel_change_ss_barring_password() response receive");
	msg("Result: [0x%x]", result);
}

static void on_ss_forward(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSsForwardResp_t *resp = data;
	int i;

	msgb("tel_set_ss_forward() or tel_get_ss_forward_status() response receive");
	msg("Result: [%d]", result);

	if (!resp) {
		msg("Response is NULL");
		return;
	}

	msg("Total Records: [%d]", resp->record_num);

	for (i = 0; i < resp->record_num; i++) {
		msg("Record:%d ----> Class: [%d], Status: %d], ForwardCondition: [%d], NoReplyWaitTime: [%d],\
							bCallForwardingNumberPresend: [%d], szCallForwardingNumber:[%s]",
				i,
				resp->record[i].Class,
				resp->record[i].Status,
				resp->record[i].ForwardCondition,
				resp->record[i].NoReplyWaitTime,
				resp->record[i].bCallForwardingNumberPresent,
				resp->record[i].szCallForwardingNumber);
	}
}

static void on_ss_waiting(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSsWaitingResp_t *resp = data;
	int i;

	msgb("tel_set_ss_waiting() or tel_get_ss_waiting_status() response receive");
	msg("Result: [%d]", result);

	if (!resp) {
		msg("Response is NULL");
		return;
	}

	msg("Total Records: [%d]", resp->record_num);

	for (i = 0; i < resp->record_num; i++) {
		msg("Record:%d ----> Class: [%d], Status: [%d]",
				i,
				resp->record[i].Class,
				resp->record[i].Status);
	}
}

static void on_ss_get_cli_status(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSsCliResp_t *resp = data;

	msgb("tel_get_ss_cli_status() response receive");
	msg("Result: [%d]", result);

	if (!resp) {
		msg("Response is NULL");
		return;
	}

	msg("Type: [%d]", resp->Type);
	msg("Status: [%d]", resp->Status);
}

static void on_ss_send_ussd_request(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSsUssdResp_t *resp = data;

	msgb("tel_send_ss_ussd_request() response receive");
	msg("Result: [%d]", result);

	if (!resp) {
		msg("Response is NULL");
		return;
	}

	msg("Type: [%d]", resp->Type);
	msg("Status: [%d]", resp->Status);
	msg("Length: [%d]", resp->Length);
	msg("String: [%s]", resp->szString);
}

static int run_ss_set_barring(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelSsBarringInfo_t info;

	msg("call tel_set_ss_barring()");

	memset(&info, 0, sizeof(TelSsBarringInfo_t));
	info.Class = atoi(data_ss_set_barring_class);
	info.Type = atoi(data_ss_set_barring_type);
	g_strlcpy(info.szPassword, data_ss_set_barring_password, TAPI_SS_GSM_BARR_PW_LEN_MAX);

	result = tel_set_ss_barring(handle, &info, on_ss_barring, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_ss_get_barring_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelSsClass_t class;
	TelSsBarringType_t type;

	msg("call tel_get_ss_barring_status()");

	class = atoi(data_ss_get_barring_status_class);
	type = atoi(data_ss_get_barring_status_type);

	result = tel_get_ss_barring_status(handle, class, type, on_ss_barring, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_ss_change_barring_password(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_change_ss_barring_password()");

	result = tel_change_ss_barring_password(handle,
			data_ss_change_barring_password_old,
			data_ss_change_barring_password_new,
			data_ss_change_barring_password_new,
			on_ss_change_barring_password, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_ss_set_forward(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelSsForwardInfo_t info;

	msg("call tel_set_ss_forward()");

	memset(&info, 0, sizeof(TelSsForwardInfo_t));
	info.Class = atoi(data_ss_set_forward_class);
	info.Condition = atoi(data_ss_set_forward_condition);
	info.NoReplyConditionTimer = atoi(data_ss_set_forward_timer);
	info.Mode = atoi(data_ss_set_forward_mode);
	g_strlcpy(info.szPhoneNumber, data_ss_set_forward_number, TAPI_CALL_DIALDIGIT_LEN_MAX + 1);

	result = tel_set_ss_forward(handle, &info, on_ss_forward, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_ss_get_forward_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelSsClass_t class;
	TelSsForwardWhen_t condition;

	msg("call tel_get_ss_forward_status()");

	class = atoi(data_ss_get_forward_status_class);
	condition = atoi(data_ss_get_forward_status_condition);

	result = tel_get_ss_forward_status(handle, class, condition, on_ss_forward, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_ss_set_waiting(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelSsWaitingInfo_t info;

	msg("call tel_set_ss_waiting()");

	memset(&info, 0, sizeof(TelSsWaitingInfo_t));
	info.Class = atoi(data_ss_set_waiting_class);
	info.Mode = atoi(data_ss_set_waiting_mode);

	result = tel_set_ss_waiting(handle, &info, on_ss_waiting, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_ss_get_waiting_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelSsClass_t class;

	msg("call tel_get_ss_waiting_status()");

	class = atoi(data_ss_get_waiting_status_class);

	result = tel_get_ss_waiting_status(handle, class, on_ss_waiting, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_ss_get_cli_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelSsCliType_t type;

	msg("call tel_get_ss_cli_status()");

	type = atoi(data_ss_get_cli_status_type);

	result = tel_get_ss_cli_status(handle, type, on_ss_get_cli_status, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_ss_send_ussd_request(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	TelSsUssdMsgInfo_t info;

	msg("call tel_send_ss_ussd_request()");

	memset(&info, 0, sizeof(TelSsUssdMsgInfo_t));
	info.Type = atoi(data_ss_ussd_request_type);
	info.Length = strlen(data_ss_ussd_request_string);
	g_strlcpy(info.szString, data_ss_ussd_request_string, TAPI_SS_USSD_DATA_SIZE_MAX + 1);

	result = tel_send_ss_ussd_request(handle, &info, on_ss_send_ussd_request, NULL);

	CHECK_RESULT(result);

	return 0;
}

static struct menu_data menu_ss_set_barring[] = {
	{ "1", "class", NULL, NULL, data_ss_set_barring_class},
	{ "-", "(16=All teleservices, 17=All voice, 18=All data teleservices", NULL, NULL, NULL},
	{ "-", " 19=FAX, 22=SMS, 23=VGCS, 24=VBS, 25=ALL_TELE_EXPT_SMS, ...)", NULL, NULL, NULL},
	{ "2", "mode", NULL, NULL, data_ss_set_barring_mode},
	{ "-", "(0=Activate, 1=Deactivate)", NULL, NULL, NULL},
	{ "3", "type", NULL, NULL, data_ss_set_barring_type},
	{ "-", "(1=All outgoing calls, 2=Outgoing international calls, 3=BOIC_NOT_HC", NULL, NULL, NULL},
	{ "-", " 4=All incoming calls, 5=BIC_ROAM, 6=AB, 7=AOB, 8=AIB, 9=BIC_NOT_SIM)", NULL, NULL, NULL},
	{ "4", "password", NULL, NULL, data_ss_set_barring_password},
	{ "5", "run", NULL, run_ss_set_barring, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_ss_get_barring_status[] = {
	{ "1", "class", NULL, NULL, data_ss_get_barring_status_class},
	{ "-", "(16=All teleservices, 17=All voice, 18=All data teleservices", NULL, NULL, NULL},
	{ "-", " 19=FAX, 22=SMS, 23=VGCS, 24=VBS, 25=ALL_TELE_EXPT_SMS, ...)", NULL, NULL, NULL},
	{ "2", "type", NULL, NULL, data_ss_get_barring_status_type},
	{ "-", "(1=All outgoing calls, 2=Outgoing international calls, 3=BOIC_NOT_HC", NULL, NULL, NULL},
	{ "-", " 4=All incoming calls, 5=BIC_ROAM, 6=AB, 7=AOB, 8=AIB, 9=BIC_NOT_SIM)", NULL, NULL, NULL},
	{ "3", "run", NULL, run_ss_get_barring_status, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_ss_change_barring_password[] = {
	{ "1", "old password", NULL, NULL, data_ss_change_barring_password_old},
	{ "2", "new password", NULL, NULL, data_ss_change_barring_password_new},
	{ "3", "run", NULL, run_ss_change_barring_password, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_ss_set_forward[] = {
	{ "1", "class", NULL, NULL, data_ss_set_forward_class},
	{ "-", "(16=All teleservices, 17=All voice, 18=All data teleservices", NULL, NULL, NULL},
	{ "-", " 19=FAX, 22=SMS, 23=VGCS, 24=VBS, 25=ALL_TELE_EXPT_SMS, ...)", NULL, NULL, NULL},
	{ "2", "mode", NULL, NULL, data_ss_set_forward_mode},
	{ "-", "(0=Deactivate, 1=Activate, 2=Register, 3=Deregister)", NULL, NULL, NULL},
	{ "3", "condition", NULL, NULL, data_ss_set_forward_condition},
	{ "-", "(1=CFU, 2=CFB, 3=CFNRy, 4=CFNRc, 5=CF_ALL, 6=CFC)", NULL, NULL, NULL},
	{ "4", "timer", NULL, NULL, data_ss_set_forward_timer},
	{ "-", "(5/10/15/20/25/30 secs)", NULL, NULL, NULL},
	{ "5", "number", NULL, NULL, data_ss_set_forward_number},
	{ "6", "run", NULL, run_ss_set_forward, NULL},

	{ NULL, NULL, },
};

static struct menu_data menu_ss_get_forward_status[] = {
	{ "1", "class", NULL, NULL, data_ss_get_forward_status_class},
	{ "-", "(16=All teleservices, 17=All voice, 18=All data teleservices", NULL, NULL, NULL},
	{ "-", " 19=FAX, 22=SMS, 23=VGCS, 24=VBS, 25=ALL_TELE_EXPT_SMS, ...)", NULL, NULL, NULL},
	{ "2", "condition", NULL, NULL, data_ss_get_forward_status_condition},
	{ "-", "(1=CFU, 2=CFB, 3=CFNRy, 4=CFNRc, 5=CF_ALL, 6=CFC)", NULL, NULL, NULL},
	{ "3", "run", NULL, run_ss_get_forward_status, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_ss_set_waiting[] = {
	{ "1", "class", NULL, NULL, data_ss_set_waiting_class},
	{ "-", "(16=All teleservices, 17=All voice, 18=All data teleservices", NULL, NULL, NULL},
	{ "-", " 19=FAX, 22=SMS, 23=VGCS, 24=VBS, 25=ALL_TELE_EXPT_SMS, ...)", NULL, NULL, NULL},
	{ "2", "mode", NULL, NULL, data_ss_set_waiting_mode},
	{ "-", "(0=Activate, 1=Deactivate)", NULL, NULL, NULL},
	{ "3", "run", NULL, run_ss_set_waiting, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_ss_get_waiting_status[] = {
	{ "1", "class", NULL, NULL, data_ss_get_waiting_status_class},
	{ "-", "(16=All teleservices, 17=All voice, 18=All data teleservices", NULL, NULL, NULL},
	{ "-", " 19=FAX, 22=SMS, 23=VGCS, 24=VBS, 25=ALL_TELE_EXPT_SMS, ...)", NULL, NULL, NULL},
	{ "2", "run", NULL, run_ss_get_waiting_status, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_ss_get_cli_status[] = {
	{ "1", "type", NULL, NULL, data_ss_get_cli_status_type},
	{ "-", "(1=CLIP, 2=CLIR, 3=COLP, 4=COLR, 5=CDIP, 6=CNAP)", NULL, NULL, NULL},
	{ "2", "run", NULL, run_ss_get_cli_status, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_ss_send_ussd_request[] = {
	{ "1", "type", NULL, NULL, data_ss_ussd_request_type},
	{ "-", "(1=INIT, 2=RSP, 3=REL)", NULL, NULL, NULL},
	{ "2", "string", NULL, NULL, data_ss_ussd_request_string},
	{ "3", "run", NULL, run_ss_send_ussd_request, NULL},
	{ NULL, NULL, },
};

struct menu_data menu_ss[] = {
	{ "1", "tel_set_ss_barring", menu_ss_set_barring, NULL, NULL},
	{ "2", "tel_get_ss_barring_status", menu_ss_get_barring_status, NULL, NULL},
	{ "3", "tel_change_ss_barring_password", menu_ss_change_barring_password, NULL, NULL},
	{ "4", "tel_set_ss_forward", menu_ss_set_forward, NULL, NULL},
	{ "5", "tel_get_ss_forward_status", menu_ss_get_forward_status, NULL, NULL},
	{ "6", "tel_set_ss_waiting", menu_ss_set_waiting, NULL, NULL},
	{ "7", "tel_get_ss_waiting_status", menu_ss_get_waiting_status, NULL, NULL},
	{ "8", "tel_get_ss_cli_status", menu_ss_get_cli_status, NULL, NULL},
	{ "9", "tel_send_ss_ussd_request", menu_ss_send_ussd_request, NULL, NULL},
	{ NULL, NULL, },
};

void register_ss_event(TapiHandle *handle)
{
	int ret;

	ret = tel_register_noti_event(handle, TAPI_NOTI_SS_USSD, on_noti_ss_ussd, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_SS_USSD - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_SS_RELEASE_COMPLETE, on_noti_ss_release_complete, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_SS_RELEASE_COMPLETE - register event failed: [%d]", ret);
}
