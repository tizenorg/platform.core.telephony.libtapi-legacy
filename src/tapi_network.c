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
#include "TelNetwork.h"

#include "common.h"
#include "tapi_log.h"
#include "ITapiNetwork.h"

#include <tapi_network.h>
#include <tel_return.h>

static gboolean __map_network_act(TelNetworkSystemType_t *act, TelNetworkAct act_buf)
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
			/*HSUPA and HSPA are not present in the Legacy code. So mapped to UMTS as it is in close proximity*/ \
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

static gboolean __map_network_act_buf(TelNetworkAct *act_buf, TelNetworkSystemType_t act)
{
	gboolean ret = TRUE;
	switch(act) {
		case TAPI_NETWORK_SYSTEM_NO_SRV :
			*act_buf = TEL_NETWORK_ACT_UNKNOWN;
			break;
		case TAPI_NETWORK_SYSTEM_GSM :
			*act_buf = TEL_NETWORK_ACT_GSM;
			break;
		case TAPI_NETWORK_SYSTEM_GPRS :
			*act_buf = TEL_NETWORK_ACT_GPRS;
			break;
		case TAPI_NETWORK_SYSTEM_EGPRS :
			*act_buf = TEL_NETWORK_ACT_EGPRS;
			break;
		case TAPI_NETWORK_SYSTEM_UMTS :
			*act_buf = TEL_NETWORK_ACT_UMTS;
			break;
		case TAPI_NETWORK_SYSTEM_GSM_AND_UMTS :
			*act_buf = TEL_NETWORK_ACT_GSM_AND_UMTS;
			break;
		case TAPI_NETWORK_SYSTEM_HSDPA :
			*act_buf = TEL_NETWORK_ACT_HSDPA;
			break;
		case TAPI_NETWORK_SYSTEM_LTE :
			*act_buf = TEL_NETWORK_ACT_LTE;
			break;
		default :
			err("Unhandled ACT");
			ret = FALSE;
	}
	return ret;
}

static TelNetworkOperationCause_t __map_network_result(int result)
{
	switch(result) {
		case TEL_NETWORK_RESULT_SUCCESS :
			return TAPI_NETWORK_NO_ERR;
		case TEL_NETWORK_RESULT_MEMORY_FAILURE :
			return TAPI_NETWORK_MEMORY_FULL_ERR;
		default :
			return TAPI_NETWORK_OPERATION_FAILED;
	}
}

static void on_response_search_network(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelNetworkOperationCause_t network_result;

	network_result = __map_network_result(result);

	if (network_result == TAPI_NETWORK_NO_ERR) {
		TelNetworkPlmnList_t list = {0, };
		TelNetworkPlmnList *list_buf = data;
		unsigned int i;

		list.networks_count = (char)list_buf->count;
		for(i = 0; i < list_buf->count; i++) {
			if (__map_network_act(&list.network_list[i].access_technology, list_buf->network_list[i].act) == FALSE) {
				network_result = TAPI_NETWORK_OPERATION_FAILED;
				goto failure;
			}
			switch(list_buf->network_list[i].plmn_status) {
				case TEL_NETWORK_STATUS_UNKNOWN :
					list.network_list[i].type_of_plmn = TAPI_UNKNOWN_PLMN;
					break;
				case TEL_NETWORK_STATUS_CURRENT :
					list.network_list[i].type_of_plmn = TAPI_HOME_PLMN;
					break;
				case TEL_NETWORK_STATUS_AVAILABLE :
					list.network_list[i].type_of_plmn = TAPI_AVAILABLE_PLMN;
					break;
				case TEL_NETWORK_STATUS_FORBIDDEN :
					list.network_list[i].type_of_plmn = TAPI_FORBIDDEN_PLMN;
					break;
				default :
					err("Unidentified plmn status. Unknown error");
					network_result = TAPI_NETWORK_OPERATION_FAILED;
					goto failure;
			}
			g_strlcpy(list.network_list[i].plmn, list_buf->network_list[i].network_identity.plmn, TAPI_NETWORK_PLMN_LEN_MAX+1);
			list.network_list[i].plmn_id = atoi(list.network_list[i].plmn);
			if (list_buf->network_list[i].network_identity.long_name)
				g_strlcpy(list.network_list[i].network_name, list_buf->network_list[i].network_identity.long_name,
					TAPI_NETWORK_NAME_LEN_MAX+1);
			else if (list_buf->network_list[i].network_identity.short_name)
				g_strlcpy(list.network_list[i].network_name, list_buf->network_list[i].network_identity.short_name,
					TAPI_NETWORK_NAME_LEN_MAX+1);
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, network_result, &list, evt_cb_data->user_data);
		}
		g_free(evt_cb_data);
		return;
	}
failure :
	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, network_result, NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_get_network_selection_mode(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelNetworkOperationCause_t network_result;

	network_result = __map_network_result(result);

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, network_result, (TelNetworkSelectionMode_t *)data, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_default_set(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, __map_network_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_get_network_preferred_plmn(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelNetworkOperationCause_t network_result;

	network_result = __map_network_result(result);

	if (network_result == TAPI_NETWORK_NO_ERR) {
		TelNetworkPreferredPlmnList_t plmn_list = {0, };
		TelNetworkPreferredPlmnList *plmn_list_buf = data;
		unsigned int i;

		plmn_list.NumOfPrefPlmns = plmn_list_buf->count;
		for(i = 0; i < plmn_list.NumOfPrefPlmns; i++) {
			plmn_list.PrefPlmnRecord[i].Index = (unsigned char)plmn_list_buf->list[i].index;
			g_strlcpy(plmn_list.PrefPlmnRecord[i].Plmn, plmn_list_buf->list[i].plmn, TAPI_NETWORK_PLMN_LEN_MAX+1);
			if (__map_network_act(&plmn_list.PrefPlmnRecord[i].SystemType, plmn_list_buf->list[i].act) == FALSE) {
				network_result = TAPI_NETWORK_OPERATION_FAILED;
				goto failure;
			}
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, network_result, &plmn_list, evt_cb_data->user_data);
		}
		g_free(evt_cb_data);
		return;
	}
failure :
	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, network_result, NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_get_network_mode(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelNetworkOperationCause_t network_result;

	network_result = __map_network_result(result);

	if (network_result == TAPI_NETWORK_NO_ERR) {
		TelNetworkMode *mode_buf = data;
		int mode;

		switch(*mode_buf) {
			case TEL_NETWORK_MODE_AUTO :
				mode = TAPI_NETWORK_MODE_AUTO;
				break;
			case TEL_NETWORK_MODE_2G :
				mode = TAPI_NETWORK_MODE_GSM;
				break;
			case TEL_NETWORK_MODE_3G :
				mode = TAPI_NETWORK_MODE_WCDMA;
				break;
			case TEL_NETWORK_MODE_LTE :
				mode = TAPI_NETWORK_MODE_LTE;
				break;
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, network_result, &mode, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, network_result, NULL, evt_cb_data->user_data);
		}
	}
	g_free(evt_cb_data);
}

static void on_response_get_network_serving(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelNetworkOperationCause_t network_result;

	network_result = __map_network_result(result);

	if (network_result == TAPI_NETWORK_NO_ERR) {
		TelNetworkIdentityInfo *id_info_resp_buf = data;
		TelNetworkServing_t net_serv_resp = {0, };
		TelNetworkRegistrationInfo reg_info_buf = {{0, }, };

		g_strlcpy(net_serv_resp.plmn, id_info_resp_buf->plmn, TAPI_NETWORK_PLMN_LEN_MAX+1);

		if (tapi_network_get_registration_info(handle, &reg_info_buf) != TEL_RETURN_SUCCESS
			|| __map_network_act(&net_serv_resp.act, reg_info_buf.reg_status_info.act) == FALSE) {
			network_result = TAPI_NETWORK_OPERATION_FAILED;
			goto failure;
		}
		net_serv_resp.lac = reg_info_buf.cell_info.lac;

		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, network_result, &net_serv_resp, evt_cb_data->user_data);
		}
		g_free(evt_cb_data);
		return;
	}
failure :
	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, network_result, NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_get_neighboring_cell_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelNetworkOperationCause_t network_result;

	network_result = __map_network_result(result);

	if (network_result == TAPI_NETWORK_NO_ERR) {
		TelNetworkNeighbourCellInfo *cell_info_list_buf = data;
		TelNetworkNeighboringCellInfo_t cell_info_list = {0, };
		int i;

		cell_info_list.geran_list_count = (int)cell_info_list_buf->gsm_list_count;
		for(i = 0; i < cell_info_list.geran_list_count; i++) {
			cell_info_list.geran_list[i].bcch = cell_info_list_buf->gsm_list[i].bcch;
			cell_info_list.geran_list[i].bsic = cell_info_list_buf->gsm_list[i].bsic;
			cell_info_list.geran_list[i].cell_id = cell_info_list_buf->gsm_list[i].cell_id;
			cell_info_list.geran_list[i].lac = cell_info_list_buf->gsm_list[i].lac;
			cell_info_list.geran_list[i].rxlev = cell_info_list_buf->gsm_list[i].rxlev;
		}
		cell_info_list.umts_list_count = (int)cell_info_list_buf->umts_list_count;
		for(i = 0; i < cell_info_list.umts_list_count; i++) {
			cell_info_list.umts_list[i].cell_id = cell_info_list_buf->umts_list[i].cell_id;
			cell_info_list.umts_list[i].lac = cell_info_list_buf->umts_list[i].lac;
			cell_info_list.umts_list[i].arfcn = cell_info_list_buf->umts_list[i].arfcn;
			cell_info_list.umts_list[i].psc = cell_info_list_buf->umts_list[i].psc;
			cell_info_list.umts_list[i].rscp = cell_info_list_buf->umts_list[i].rscp;
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, network_result, &cell_info_list, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, network_result, NULL, evt_cb_data->user_data);
		}
	}
	g_free(evt_cb_data);
}

EXPORT_API int tel_search_network(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_search((TelHandle *)handle, on_response_search_network, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_network_selection_mode(struct tapi_handle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_get_selection_mode((TelHandle *)handle, on_response_get_network_selection_mode, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_select_network_automatic(struct tapi_handle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_select_automatic((TelHandle *)handle, on_response_default_set, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_select_network_manual(struct tapi_handle *handle, const char *plmn, int type, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelNetworkSelectManualInfo select_info_buf = {0, };

	dbg("Func Entrance");

	select_info_buf.plmn = (char *)plmn;
	if (__map_network_act_buf(&select_info_buf.act, type) == FALSE)
		return TAPI_API_INVALID_INPUT;

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_select_manual((TelHandle *)handle, &select_info_buf, on_response_default_set, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_network_preferred_plmn( TapiHandle *handle, TelNetworkPreferredPlmnOp_t operation,
		TelNetworkPreferredPlmnInfo_t *info, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelNetworkPreferredPlmnInfo plmn_info_buf = {0, };

	dbg("Func Entrance");

	if (operation == TAPI_NETWORK_PREF_PLMN_ADD || operation == TAPI_NETWORK_PREF_PLMN_EDIT) {
		if (info->Index > TAPI_PREFERRED_PLMN_RECORDS_MAX) {
			err("Invalid index");
			return TAPI_API_INVALID_INPUT;
		}
		plmn_info_buf.index = info->Index;
		plmn_info_buf.plmn = info->Plmn;
		if (__map_network_act_buf(&plmn_info_buf.act, info->SystemType) == FALSE)
			return TAPI_API_INVALID_INPUT;
	} else if (operation == TAPI_NETWORK_PREF_PLMN_DELETE) {
		plmn_info_buf.index = info->Index;
		plmn_info_buf.plmn = NULL;
		plmn_info_buf.act = TEL_NETWORK_ACT_UNKNOWN;
	} else {
		err("Invalid operation");
		return TAPI_API_INVALID_INPUT;
	}

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_set_preferred_plmn((TelHandle *)handle, &plmn_info_buf, on_response_default_set, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_network_preferred_plmn(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_get_preferred_plmn((TelHandle *)handle, on_response_get_network_preferred_plmn, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_network_band(TapiHandle *handle, TelNetworkBandPreferred_t mode, TelNetworkBand_t band, tapi_response_cb callback, void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_get_network_band(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}

EXPORT_API int tel_set_network_mode(TapiHandle *handle, int mode, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelNetworkMode mode_buf = -1;

	dbg("Func Entrance");

	switch(mode) {
		case TAPI_NETWORK_MODE_AUTO :
			mode_buf = TEL_NETWORK_MODE_AUTO;
			break;
		case TAPI_NETWORK_MODE_GSM :
			mode_buf = TEL_NETWORK_MODE_2G;
			break;
		case TAPI_NETWORK_MODE_WCDMA :
			mode_buf = TEL_NETWORK_MODE_3G;
			break;
		case TAPI_NETWORK_MODE_LTE :
			mode_buf = TEL_NETWORK_MODE_LTE;
			break;
		default :
			/*Combined modes not supported in Tizen 3.0*/
			/*TAPI_NETWORK_MODE_CDMA - Not supported*/
			err("unhandled network mode");
			return TAPI_API_INVALID_INPUT;
	}

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_set_mode((TelHandle *)handle, mode_buf, on_response_default_set, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_network_mode(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_get_mode((TelHandle *)handle, on_response_get_network_mode, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_network_service_domain(TapiHandle *handle, TelNetworkServiceDomain_t domain, tapi_response_cb callback, void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}

EXPORT_API int tel_get_network_service_domain(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_cancel_network_manual_search(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_cancel_search((TelHandle *)handle, on_response_default_set, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_network_serving(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_get_identity_info((TelHandle *)handle, on_response_get_network_serving, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_network_neighboring_cell_info(TapiHandle *handle, tapi_response_cb callback, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, callback, user_data, NULL, 0);

	ret = tapi_network_get_neighboring_cell_info((TelHandle *)handle, on_response_get_neighboring_cell_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

