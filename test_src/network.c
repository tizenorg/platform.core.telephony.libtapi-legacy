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
#include <ITapiNetwork.h>
#include <TapiUtility.h>

#include "menu.h"
#include "network.h"

static char data_net_set_plmn_mode_manual_act[MENU_DATA_SIZE + 1] = "5";
static char data_net_set_plmn_mode_manual_plmn[MENU_DATA_SIZE + 1] = "45001";
static char data_net_set_preferred_plmn_mode[MENU_DATA_SIZE + 1] = "1";
static char data_net_set_preferred_plmn_plmn[MENU_DATA_SIZE + 1] = "45001";
static char data_net_set_preferred_plmn_type[MENU_DATA_SIZE + 1] = "5";
static char data_net_set_preferred_plmn_ef_index[MENU_DATA_SIZE + 1] = "0";

static char data_net_set_mode_mode[MENU_DATA_SIZE + 1] = "3";

static void on_noti_registration_status(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	struct tel_noti_network_registration_status *noti = data;

	msgb("event(%s) receive !!", TAPI_NOTI_NETWORK_REGISTRATION_STATUS);

	if (!noti)
		return;

	msg("circuit status [%d]", noti->cs);
	msg("packet status [%d]", noti->ps);
	msg("service type [%d]", noti->type);
	msg("is roaming [%d]", noti->is_roaming);
}

static void on_noti_cell_info(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	struct tel_noti_network_cell_info *noti = data;

	msgb("event(%s) receive !!", TAPI_NOTI_NETWORK_CELLINFO);

	if (!noti)
		return;

	msg("lac: [%d]", noti->lac);
	msg("cell_id: [%d]", noti->cell_id);
}

static void on_noti_info(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	struct tel_noti_network_info *noti = data;

	msgb("event(%s) receive !!", TAPI_NOTI_NETWORK_INFO);

	if (!noti)
		return;

	msg("rssi: [%d]", noti->rssi);
	msg("battery: [%d]", noti->battery);
}

static void on_noti_time_info(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	struct tel_noti_network_time_info *noti = data;

	msgb("event(%s) receive !!", TAPI_NOTI_NETWORK_TIMEINFO);

	if (!noti)
		return;

	msg("plmn = %s", noti->plmn);
	msg("year = %d", noti->year);
	msg("month = %d", noti->month);
	msg("day = %d", noti->day);
	msg("hour = %d", noti->hour);
	msg("minute = %d", noti->minute);
	msg("second = %d", noti->second);
	msg("wday = %d", noti->wday);
	msg("gmtoff = %d", noti->gmtoff);
	msg("dstoff = %d", noti->dstoff);
	msg("isdst = %d", noti->isdst);
}

static void on_noti_identity(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	struct tel_noti_network_identity *noti = data;

	msgb("event(%s) receive !!", TAPI_NOTI_NETWORK_IDENTITY);

	if (!noti)
		return;

	msg("plmn = %s", noti->plmn);
	msg("short_name = %s", noti->short_name);
	msg("full_name = %s", noti->full_name);
}

static void on_net_search(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelNetworkPlmnList_t *list = data;
	int i =0;

	msgb("tel_search_network() response receive: [%d]", result);

	if (!list) {
		msg("failed");
		return;
	}

	msg(" - count = %d", list->networks_count);

	for (i = 0; i < list->networks_count; i++) {
		msg(" - [%d] plmn=%s, act=%d, type=%d, name=%s",
				i,
				list->network_list[i].plmn,
				list->network_list[i].access_technology,
				list->network_list[i].type_of_plmn,
				list->network_list[i].network_name);
	}
}

static void on_net_set_plmn_mode_automatic(TapiHandle *handle, int result, void *data, void *user_data)
{
	msgb("tel_select_network_automatic() response receive: [%d]", result);
}

static void on_net_set_plmn_mode_manual(TapiHandle *handle, int result, void *data, void *user_data)
{
	msgb("tel_select_network_manual() response receive: [%d]", result);
}

static void on_net_get_plmn_mode(TapiHandle *handle, int result, void *data, void *user_data)
{
	int *mode = data;

	msgb("tel_get_network_selection_mode() response receive: [%d]", result);

	if (!mode )
		return;

	msg("mode: [%d]", *mode);
}

static void on_net_set_preferred_plmn(TapiHandle *handle, int result, void *data, void *user_data)
{
	msgb("tel_set_preferred_plmn() response receive: [%d]", result);
}

static void on_net_get_preferred_plmn(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelNetworkPreferredPlmnList_t *info = data;
	unsigned int i = 0;

	msgb("tel_get_preferred_plmn() response receive: [%d]", result);

	if (!info) {
		msg("failed");
		return;
	}

	msg("count: [%d]", info->NumOfPrefPlmns);

	for (i = 0; i < info->NumOfPrefPlmns; i++) {
		msg("PLMN%d ---> plmn: [%s], type: [%d], ef_index: [%d], name: [%s]",
				i+1,
				info->PrefPlmnRecord[i].Plmn,
				info->PrefPlmnRecord[i].SystemType,
				info->PrefPlmnRecord[i].Index,
				info->PrefPlmnRecord[i].network_name);
	}
}

static void on_net_set_cancel_manual_search(TapiHandle *handle, int result, void *data, void *user_data)
{
	msgb("tel_cancel_network_manual_search() response receive: [%d]", result);
}

static void on_net_get_serving_network(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelNetworkServing_t *info = data;

	msgb("tel_get_network_serving() response receive: [%d]", result);

	if (!info) {
		msg("failed");
		return;
	}

	msg("plmn: [%s]", info->plmn);
	msg("access technology: [%d]", info->act);
	msg("lac: [%d]", info->lac);
}

static void on_net_set_mode(TapiHandle *handle, int result, void *data, void *user_data)
{
	msgb("tel_set_network_mode() response receive: [%d]", result);
}

static void on_net_get_mode(TapiHandle *handle, int result, void *data, void *user_data)
{
	int *mode = data;

	msgb("tel_get_network_mode() response receive: [%d]", result);

	if (!mode)
		return;

	msg("mode: [%d]", *mode);
}

static void on_net_get_neighboring_cell_info(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelNetworkNeighboringCellInfo_t *info = data;
	int i,j;
	msgb("tel_get_neighboring_cell_info() response receive: [%d]", result);

	if (!info)
		return;

	msg("geran_list_count=%d",info->geran_list_count);
	for(i=0; i<info->geran_list_count; i++) {
		msg("[%d] cell_id=%d, lac=%d, bcch=%d, bsic=%d, rxlev=%d,",i,
			info->geran_list[i].cell_id, info->geran_list[i].lac, info->geran_list[i].bcch,
			info->geran_list[i].bsic, info->geran_list[i].rxlev);
	}

	msg("umts_list_count=%d",info->umts_list_count);
	for(j=0; j<info->umts_list_count; j++) {
		msg("[%d] cell_id=%d, lac=%d, arfcn=%d, psc=%d, rscp=%d,",j,
			info->umts_list[j].cell_id, info->umts_list[j].lac, info->umts_list[j].arfcn,
			info->umts_list[j].psc, info->umts_list[j].rscp);
	}
}

static int run_net_search(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_search_network()");

	result = tel_search_network(handle, on_net_search, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_set_plmn_mode_automatic(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_select_network_automatic()");

	result = tel_select_network_automatic(handle, on_net_set_plmn_mode_automatic, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_set_plmn_mode_manual(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	const char *plmn;
	int act;

	msg("call tel_select_network_manual()");

	plmn = data_net_set_plmn_mode_manual_plmn;
	act = atoi(data_net_set_plmn_mode_manual_act);

	result = tel_select_network_manual(handle, plmn, act, on_net_set_plmn_mode_manual, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_get_plmn_mode(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_get_network_selection_mode()");

	result = tel_get_network_selection_mode(handle, on_net_get_plmn_mode, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_set_preferred_plmn(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	int operation;
	TelNetworkPreferredPlmnInfo_t info;

	msg("call tel_set_network_preferred_plmn()");

	memset(&info, 0, sizeof(TelNetworkPreferredPlmnInfo_t));

	operation = atoi(data_net_set_preferred_plmn_mode);
	info.Index = atoi(data_net_set_preferred_plmn_ef_index);
	info.SystemType = atoi(data_net_set_preferred_plmn_type);
	strncpy(info.Plmn, data_net_set_preferred_plmn_plmn, 6);

	result = tel_set_network_preferred_plmn(handle, operation, &info, on_net_set_preferred_plmn, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_get_preferred_plmn(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_get_network_preferred_plmn()");

	result = tel_get_network_preferred_plmn(handle, on_net_get_preferred_plmn, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_set_cancel_manual_search(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_cancel_network_manual_search()");

	result = tel_cancel_network_manual_search(handle, on_net_set_cancel_manual_search, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_get_serving_network(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_get_network_serving()");

	result = tel_get_network_serving(handle, on_net_get_serving_network, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_set_mode(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	int mode;

	msg("call tel_set_network_mode()");

	mode = atoi(data_net_set_mode_mode);

	result = tel_set_network_mode(handle, mode, on_net_set_mode, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_get_mode(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_get_network_mode()");

	result = tel_get_network_mode(handle, on_net_get_mode, NULL);

	CHECK_RESULT(result);

	return 0;
}

static int run_net_get_neighboring_cell_info(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;

	msg("call tel_get_network_neighboring_cell_info()");

	result = tel_get_network_neighboring_cell_info(handle, on_net_get_neighboring_cell_info, NULL);

	CHECK_RESULT(result);

	return 0;
}

static struct menu_data menu_net_search[] = {
	{ "1", "run", NULL, run_net_search, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_net_set_plmn_mode_automatic[] = {
	{ "1", "run", NULL, run_net_set_plmn_mode_automatic, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_net_set_plmn_mode_manual[] = {
	{ "1", "act", NULL, NULL, data_net_set_plmn_mode_manual_act},
	{ "2", "plmn (45001)", NULL, NULL, data_net_set_plmn_mode_manual_plmn},
	{ "3", "run", NULL, run_net_set_plmn_mode_manual, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_net_get_plmn_mode[] = {
	{ "1", "run", NULL, run_net_get_plmn_mode, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_net_set_preferred_plmn[] = {
	{ "1", "mode (1=add, 2=edit, 3=delete)", NULL, NULL, data_net_set_preferred_plmn_mode},
	{ "2", "plmn (45001)", NULL, NULL, data_net_set_preferred_plmn_plmn},
	{ "3", "type (1=GSM, 2=GPRS, 3=EGPRS, 5=UMTS, ...)", NULL, NULL, data_net_set_preferred_plmn_type},
	{ "4", "ef_index (0,...)", NULL, NULL, data_net_set_preferred_plmn_ef_index},
	{ "5", "run", NULL, run_net_set_preferred_plmn, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_net_get_preferred_plmn[] = {
	{ "1", "run", NULL, run_net_get_preferred_plmn, NULL},
	{ NULL, NULL, },
};


static struct menu_data menu_net_set_cancel_manual_search[] = {
	{ "1", "run", NULL, run_net_set_cancel_manual_search, NULL},
	{ NULL, NULL, },
};


static struct menu_data menu_net_get_serving_network[] = {
	{ "1", "run", NULL, run_net_get_serving_network, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_net_set_mode[] = {
	{ "1", "mode (1=GSM, 2=WCDMA, 4=CDMA, 8=LTE)", NULL, NULL, data_net_set_mode_mode},
	{ "3", "run", NULL, run_net_set_mode, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_net_get_mode[] = {
	{ "1", "run", NULL, run_net_get_mode, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_net_get_neighboring_cell_info[] = {
	{ "1", "run", NULL, run_net_get_neighboring_cell_info, NULL},
	{ NULL, NULL, },
};

struct menu_data menu_net[] = {
	{ "1", "Search", menu_net_search, NULL, NULL},
	{ "2a", "SET PLMN Selection mode (Automatic)", menu_net_set_plmn_mode_automatic, NULL, NULL},
	{ "2s", "SET PLMN Selection mode (Manual)", menu_net_set_plmn_mode_manual, NULL, NULL},
	{ "2g", "GET PLMN Selection mode", menu_net_get_plmn_mode, NULL, NULL},
	{ "3s", "SET Preferred PLMN", menu_net_set_preferred_plmn, NULL, NULL},
	{ "3g", "GET Preferred PLMN", menu_net_get_preferred_plmn, NULL, NULL},
	{ "4", "SET Cancel manual search", menu_net_set_cancel_manual_search, NULL, NULL},
	{ "5", "GET Serving network", menu_net_get_serving_network, NULL, NULL},
	{ "6s", "SET Mode", menu_net_set_mode, NULL, NULL},
	{ "6g", "GET Mode", menu_net_get_mode, NULL, NULL},
	{ "7g", "GET Neighboring Cell Info", menu_net_get_neighboring_cell_info, NULL, NULL},
	{ NULL, NULL, },
};

void register_network_event(TapiHandle *handle)
{
	int ret;

	ret = tel_register_noti_event(handle, TAPI_NOTI_NETWORK_REGISTRATION_STATUS, on_noti_registration_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_NETWORK_REGISTRATION_STATUS - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_NETWORK_CELLINFO, on_noti_cell_info, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_NETWORK_CELLINFO - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_NETWORK_INFO, on_noti_info, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_NETWORK_INFO - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_NETWORK_TIMEINFO, on_noti_time_info, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_NETWORK_TIMEINFO - register event failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_NETWORK_IDENTITY, on_noti_identity, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_NETWORK_IDENTITY - register event failed: [%d]", ret);
}
