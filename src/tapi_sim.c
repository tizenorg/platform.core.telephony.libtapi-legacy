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
#include "ITapiSim.h"

#include <tel_return.h>
#include <tapi_sim.h>

#define MAP_SIM_CARD_STATUS_RESP(src, dest) \
	do{ \
		switch (src) { \
			case TEL_SIM_STATUS_CARD_ERROR : \
				dest = TAPI_SIM_STATUS_CARD_ERROR; \
				break; \
			case TEL_SIM_STATUS_CARD_NOT_PRESENT : \
				dest = TAPI_SIM_STATUS_CARD_NOT_PRESENT; \
				break; \
			case TEL_SIM_STATUS_SIM_INITIALIZING : \
				dest = TAPI_SIM_STATUS_SIM_INITIALIZING; \
				break; \
			case TEL_SIM_STATUS_SIM_INIT_COMPLETED : \
				dest = TAPI_SIM_STATUS_SIM_INIT_COMPLETED; \
				break; \
			case TEL_SIM_STATUS_SIM_PIN_REQUIRED : \
				dest = TAPI_SIM_STATUS_SIM_PIN_REQUIRED; \
				break; \
			case TEL_SIM_STATUS_SIM_PUK_REQUIRED : \
				dest = TAPI_SIM_STATUS_SIM_PUK_REQUIRED; \
				break; \
			case TEL_SIM_STATUS_CARD_BLOCKED : \
				dest = TAPI_SIM_STATUS_CARD_BLOCKED; \
				break; \
			case TEL_SIM_STATUS_SIM_NCK_REQUIRED : \
				dest = TAPI_SIM_STATUS_SIM_NCK_REQUIRED; \
				break; \
			case TEL_SIM_STATUS_SIM_NSCK_REQUIRED : \
				dest = TAPI_SIM_STATUS_SIM_NSCK_REQUIRED; \
				break; \
			case TEL_SIM_STATUS_SIM_SPCK_REQUIRED : \
				dest = TAPI_SIM_STATUS_SIM_SPCK_REQUIRED; \
				break; \
			case TEL_SIM_STATUS_SIM_CCK_REQUIRED : \
				dest = TAPI_SIM_STATUS_SIM_CCK_REQUIRED; \
				break; \
			case TEL_SIM_STATUS_CARD_REMOVED : \
			case TEL_SIM_STATUS_SIM_CARD_POWEROFF : \
				dest = TAPI_SIM_STATUS_CARD_REMOVED; \
				break; \
			case TEL_SIM_STATUS_SIM_LOCK_REQUIRED: \
				dest = TAPI_SIM_STATUS_SIM_LOCK_REQUIRED; \
				break; \
			case TEL_SIM_STATUS_UNKNOWN: \
				dest = TAPI_SIM_STATUS_UNKNOWN; \
		} \
	}while(0)

#define MAP_SIM_CARD_CHANGE_STATUS_RESP(src, dest) \
do { \
	switch (src) { \
	case TEL_SIM_CHANGE_STATUS_UNKNOWN: \
		dest = FALSE; \
	case TEL_SIM_CHANGE_STATUS_SAME: \
		dest = FALSE; \
	case TEL_SIM_CHANGE_STATUS_CHANGED: \
		dest = TRUE; \
	} \
} while (0)

#define MAP_SIM_MAILBOX_TYPE(src, dest) \
	do{ \
		switch(src) { \
			case TAPI_SIM_MAILBOX_VOICE : \
				dest = TEL_SIM_MAILBOX_VOICE; \
				break; \
			case TAPI_SIM_MAILBOX_FAX : \
				dest = TEL_SIM_MAILBOX_FAX; \
				break; \
			case TAPI_SIM_MAILBOX_DATA : \
				dest = TEL_SIM_MAILBOX_VIDEO; \
				break; \
			case TAPI_SIM_MAILBOX_EMAIL: \
				dest = TEL_SIM_MAILBOX_EMAIL; \
				break; \
			case TAPI_SIM_MAILBOX_OTHER: \
				dest = TEL_SIM_MAILBOX_OTHER; \
		} \
	}while(0)

#define MAP_SIM_MAILBOX_TYPE_RESP(src, dest) \
	do{ \
		switch(src) { \
			case TEL_SIM_MAILBOX_VOICE : \
				dest = TAPI_SIM_MAILBOX_VOICE; \
				break; \
			case TEL_SIM_MAILBOX_FAX : \
				dest = TAPI_SIM_MAILBOX_FAX; \
				break; \
			case TEL_SIM_MAILBOX_VIDEO: \
				dest = TAPI_SIM_MAILBOX_DATA; \
				break; \
			case TEL_SIM_MAILBOX_EMAIL: \
				dest = TAPI_SIM_MAILBOX_EMAIL; \
				break; \
			case TEL_SIM_MAILBOX_OTHER: \
				dest = TAPI_SIM_MAILBOX_OTHER; \
		} \
	}while(0)

#define MAP_SIM_AUTH_TYPE(src, dest) \
	do{ \
		switch(src) { \
			case TAPI_SIM_AUTH_TYPE_IMS : \
				dest = TEL_SIM_AUTH_IMS_AKA; \
				break; \
			case TAPI_SIM_AUTH_TYPE_GSM : \
			case TAPI_SIM_AUTH_TYPE_MAX : \
				dest = TEL_SIM_AUTH_GSM; \
				break; \
			case TAPI_SIM_AUTH_TYPE_3G : \
				dest = TEL_SIM_AUTH_3G_CTX; \
		} \
	}while(0)

#define MAP_SIM_AUTH_TYPE_RESP(src, dest) \
	do{ \
		switch(src) { \
			case TEL_SIM_AUTH_IMS_AKA : \
				dest = TAPI_SIM_AUTH_TYPE_IMS; \
				break; \
			case TEL_SIM_AUTH_GSM : \
				dest = TAPI_SIM_AUTH_TYPE_GSM; \
				break; \
			case TEL_SIM_AUTH_3G_CTX: \
				dest = TAPI_SIM_AUTH_TYPE_3G; \
			default : \
				dest = TAPI_SIM_AUTH_TYPE_MAX; \
		} \
	}while(0)

#define MAP_SIM_LOCK_TYPE(src, dest) \
	do{ \
		switch(src) { \
			case TAPI_SIM_LOCK_PS : \
				dest = TEL_SIM_LOCK_PS; \
				break; \
			case TAPI_SIM_LOCK_PF : \
				dest = TEL_SIM_LOCK_PF; \
				break; \
			case TAPI_SIM_LOCK_SC : \
				dest = TEL_SIM_LOCK_SC; \
				break; \
			case TAPI_SIM_LOCK_FD : \
				dest = TEL_SIM_LOCK_FD; \
				break; \
			case TAPI_SIM_LOCK_PN : \
				dest = TEL_SIM_LOCK_PN; \
				break; \
			case TAPI_SIM_LOCK_PU : \
				dest = TEL_SIM_LOCK_PU; \
				break; \
			case TAPI_SIM_LOCK_PP : \
				dest = TEL_SIM_LOCK_PP; \
				break; \
			case TAPI_SIM_LOCK_PC : \
				dest = TEL_SIM_LOCK_PC; \
				break; \
			default : \
				return TAPI_API_INVALID_INPUT; \
		} \
	}while(0)

#define MAP_SIM_LOCK_TYPE_RESP(src, dest) \
	do{ \
		switch(src) { \
			case TEL_SIM_LOCK_PS : \
				dest = TAPI_SIM_LOCK_PS; \
				break; \
			case TEL_SIM_LOCK_PF : \
				dest = TAPI_SIM_LOCK_PF; \
				break; \
			case TEL_SIM_LOCK_SC : \
				dest = TAPI_SIM_LOCK_SC; \
				break; \
			case TEL_SIM_LOCK_FD : \
				dest = TAPI_SIM_LOCK_FD; \
				break; \
			case TEL_SIM_LOCK_PN : \
				dest = TAPI_SIM_LOCK_PN; \
				break; \
			case TEL_SIM_LOCK_PU : \
				dest = TAPI_SIM_LOCK_PU; \
				break; \
			case TEL_SIM_LOCK_PP : \
				dest = TAPI_SIM_LOCK_PP; \
				break; \
			case TEL_SIM_LOCK_PC : \
				dest = TAPI_SIM_LOCK_PC; \
				break; \
		} \
	}while(0)

static TelSimPinOperationResult_t __map_sim_result(int result)
{
	switch(result) {
		case TEL_SIM_RESULT_SUCCESS :
			return TAPI_SIM_PIN_OPERATION_SUCCESS;
		case TEL_SIM_RESULT_INCOMPATIBLE_PIN_OPERATION :
			return TAPI_SIM_INCOMPATIBLE_PIN_OPERATION;
		case TEL_SIM_RESULT_INCORRECT_PASSWORD :
			return TAPI_SIM_PIN_INCORRECT_PASSWORD;
		case TEL_SIM_RESULT_CARD_ERROR :
			return TAPI_SIM_CARD_ERROR;
		case TEL_SIM_RESULT_PIN_REQUIRED :
			return TAPI_SIM_PIN_REQUIRED;
		case TEL_SIM_RESULT_PUK_REQUIRED :
			return TAPI_SIM_PUK_REQUIRED;
		default :
			return TAPI_SIM_ACCESS_FAILED;
	}
}

static TelSimAccessResult_t __map_sim_access_result (int result)
{
	switch(result) {
		case TEL_SIM_RESULT_SUCCESS :
			return TAPI_SIM_ACCESS_SUCCESS;
		case TEL_SIM_RESULT_CARD_ERROR :
			return TAPI_SIM_ACCESS_CARD_ERROR;
		case TEL_SIM_RESULT_ACCESS_CONDITION_NOT_SATISFIED :
			return TAPI_SIM_ACCESS_ACCESS_CONDITION_NOT_SATISFIED;
		case TEL_SIM_RESULT_INVALID_PARAMETER :
			return TAPI_SIM_ACCESS_FILE_NOT_FOUND;
		default :
			return TAPI_SIM_ACCESS_FAILED;
	}
}

static void on_response_get_sim_iccid(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimIccIdInfo_t resp;
	gchar *resp_buf = NULL;
	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (char *)data;
		if(resp_buf[0]) {
			g_strlcpy(resp.icc_num, resp_buf, TAPI_SIM_ICCID_LEN_MAX + 1);
			resp.icc_length = strlen(resp.icc_num);
			if (evt_cb_data->cb) {
				evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
			}
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_language(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimLanguagePreferenceCode_t resp;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp = *(TelSimLanguagePreferenceCode_t *)data;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_set_sim_language(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_callforwarding_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimCallForwardingResp_t resp;
	TelSimCfisList *resp_buf = NULL;
	unsigned int i;
	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimCfisList *)data;
		resp.b_cphs = 0;
		resp.cphs_cf.b_data = 0;
		resp.cphs_cf.b_fax = 0;
		resp.cphs_cf.b_line1 = 0;
		resp.cphs_cf.b_line2 = 0;

		resp.cf_list.profile_count = resp_buf->profile_count;
		for (i = 0; i < resp_buf->profile_count; i++) {
			resp.cf_list.cf[i].cfu_status = '\0';
			if(resp_buf->cf[i].indication[TEL_SIM_CALL_FORWARDING_VOICE]) {
				resp.cf_list.cf[i].cfu_status |= 0x01 ;
			}
			if(resp_buf->cf[i].indication[TEL_SIM_CALL_FORWARDING_FAX]) {
				resp.cf_list.cf[i].cfu_status |= 0x02 ;
			}
			if(resp_buf->cf[i].indication[TEL_SIM_CALL_FORWARDING_EMAIL]) {
				resp.cf_list.cf[i].cfu_status |= 0x04 ;
			}
			if(resp_buf->cf[i].indication[TEL_SIM_CALL_FORWARDING_OTHER]) {
				resp.cf_list.cf[i].cfu_status |= 0x08 ;
			}
			if(resp_buf->cf[i].indication[TEL_SIM_CALL_FORWARDING_VIDEO]) {
				resp.cf_list.cf[i].cfu_status |= 0x10 ;
			}
			resp.cf_list.cf[i].rec_index = 0;
			resp.cf_list.cf[i].msp_num = '\0';
			resp.cf_list.cf[i].ton = TAPI_SIM_TON_UNKNOWN;
			resp.cf_list.cf[i].npi = TAPI_SIM_NPI_UNKNOWN;
			memset(resp.cf_list.cf[i].cfu_num, '0', TAPI_SIM_XDN_DIALING_NUMBER_LEN + 1);
			resp.cf_list.cf[i].cc2_id = '\0';
			resp.cf_list.cf[i].ext7_id = '\0';
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_messagewaiting_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimMessageWaitingResp_t resp;
	TelSimMwisList *resp_buf = NULL;
	unsigned int i;
	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimMwisList *)data;
		resp.mw_list.profile_count = resp_buf->profile_count;
		for (i = 0; i < resp_buf->profile_count; i++) {
			resp.b_cphs = ((resp_buf->mw[i].count_valid) ? 0 : 1);
			if (resp.b_cphs) {				/*CPHS Case*/
				if (resp_buf->mw[i].profile_id == 1){
					resp.cphs_mw.b_voice1 = resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_VOICE].indication;
					resp.cphs_mw.b_fax = resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_FAX].indication;
					resp.cphs_mw.b_data= resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_EMAIL].indication;
				} else {
					resp.cphs_mw.b_voice2 = resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_VOICE].indication;
				}
			} else {						/*MW Case*/
				resp.mw_list.mw[i].indicator_status = 0;
				if(resp_buf->mw[i].msg_waiting[0].indication) {
					resp.mw_list.mw[i].voice_count = resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_VOICE].count;
					resp.mw_list.mw[i].indicator_status |= 0x01 ;
				}
				if(resp_buf->mw[i].msg_waiting[1].indication) {
					resp.mw_list.mw[i].fax_count = resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_FAX].count;
					resp.mw_list.mw[i].indicator_status |= 0x02 ;
				}
				if(resp_buf->mw[i].msg_waiting[2].indication) {
					resp.mw_list.mw[i].email_count = resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_EMAIL].count;
					resp.mw_list.mw[i].indicator_status |= 0x04 ;
				}
				if(resp_buf->mw[i].msg_waiting[3].indication) {
					resp.mw_list.mw[i].other_count = resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_OTHER].count;
					resp.mw_list.mw[i].indicator_status |= 0x08 ;
				}
				if(resp_buf->mw[i].msg_waiting[4].indication) {
					resp.mw_list.mw[i].video_count = resp_buf->mw[i].msg_waiting[TEL_SIM_MAILBOX_VIDEO].count;
					resp.mw_list.mw[i].indicator_status |= 0x10 ;
				}
				resp.mw_list.mw[i].rec_index = 0;
			}
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_set_sim_messagewaiting_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_mailbox_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimMailboxList_t resp;
	TelSimMailboxList *resp_buf = NULL;
	unsigned int i;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimMailboxList *)data;
		resp.count = resp_buf->count;
		for (i = 0; i < resp_buf->count; i++) {
			resp.list[i].profile_num = resp_buf->list[i].profile_id;
			if (resp_buf->list[i].alpha_id)
				memcpy(resp.list[i].alpha_id, resp_buf->list[i].alpha_id, resp_buf->list[i].alpha_id_len);
			resp.list[i].alpha_id_max_len = resp_buf->list[i].alpha_id_len;
			if (resp_buf->list[i].number)
				g_strlcpy (resp.list[i].num, resp_buf->list[i].number, TAPI_SIM_XDN_DIALING_NUMBER_LEN + 1);
			MAP_SIM_MAILBOX_TYPE_RESP(resp_buf->list[i].mb_type, resp.list[i].mb_type);
			resp.list[i].b_cphs = resp.list[i].rec_index = resp.list[i].npi = resp.list[i].ton = 0;
			resp.list[i].cc_id = resp.list[i].ext1_id = '\0';
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}


static void on_response_set_sim_mailbox_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (evt_cb_data->cb) {
		evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_msisdn(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimMsisdnList_t resp;
	TelSimMsisdnList *resp_buf = NULL;
	unsigned int i;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimMsisdnList *)data;
		resp.count = resp_buf->count;
		for (i = 0; i < resp_buf->count; i++) {
			if (resp_buf->list[i].alpha_id)
				g_strlcpy (resp.list[i].name, resp_buf->list[i].alpha_id, TAPI_SIM_XDN_ALPHA_ID_MAX_LEN + 1);
			if (resp_buf->list[i].num)
				g_strlcpy (resp.list[i].num, resp_buf->list[i].num, TAPI_SIM_XDN_DIALING_NUMBER_LEN + 1);
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_spn(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSpn_t resp;
	TelSimSpn *resp_buf = NULL;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimSpn *)data;
		resp.display_condition = (unsigned char) resp_buf->display_condition;
		if (resp_buf->spn)
			g_strlcpy((gchar *)resp.spn, resp_buf->spn, TAPI_SIM_NET_FULL_NAME_MAX_LEN + 1);
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_cphs_netname(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimCphsNetName_t resp;
	TelSimCphsNetName *resp_buf = NULL;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimCphsNetName *)data;
		if (resp_buf->full_name)
			g_strlcpy((gchar *)resp.full_name, resp_buf->full_name, TAPI_SIM_NET_FULL_NAME_MAX_LEN + 1);
		if (resp_buf->short_name)
			g_strlcpy((gchar *)resp.short_name, resp_buf->short_name, TAPI_SIM_NET_SHORT_NAME_MAX_LEN + 1);
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_req_sim_authentication(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimAuthenticationResponse_t resp;
	TelSimAuthenticationResponse *resp_buf = NULL;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimAuthenticationResponse *)data;
		MAP_SIM_AUTH_TYPE_RESP(resp_buf->auth_type, resp.auth_type);

		resp_buf->detailed_result = resp.auth_result;
		resp_buf->resp_length = resp.resp_length;
		if (resp_buf->resp_data)
			g_strlcpy(resp.resp_data, resp_buf->resp_data, TAPI_SIM_AUTH_MAX_RESP_DATA_LEN + 1);
		resp_buf->authentication_key_length = resp.authentication_key_length;
		if (resp_buf->authentication_key)
			g_strlcpy(resp.authentication_key, resp_buf->authentication_key, TAPI_SIM_AUTH_MAX_RESP_DATA_LEN + 1);
		resp_buf->cipher_length = resp.cipher_length;
		if (resp_buf->cipher_data)
			g_strlcpy(resp.cipher_data, resp_buf->cipher_data, TAPI_SIM_AUTH_MAX_RESP_DATA_LEN + 1);
		resp_buf->integrity_length = resp.integrity_length;
		if (resp_buf->integrity_data)
			g_strlcpy(resp.integrity_data, resp_buf->integrity_data, TAPI_SIM_AUTH_MAX_RESP_DATA_LEN + 1);
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_verify_sim_pins(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSecResult_t resp;
	TelSimSecPinResult *resp_buf = NULL;

	TelSimPinOperationResult_t res =__map_sim_result(result);

	if (res == TAPI_SIM_PIN_OPERATION_SUCCESS) {
		resp_buf = (TelSimSecPinResult *)data;
		resp.type = resp_buf->pin_type;
		resp.retry_count = resp_buf->retry_count;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_verify_sim_puks(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSecResult_t resp;
	TelSimSecPukResult *resp_buf = NULL;

	TelSimPinOperationResult_t res =__map_sim_result(result);

	if (res == TAPI_SIM_PIN_OPERATION_SUCCESS) {
		resp_buf = (TelSimSecPukResult *)data;
		if (resp_buf->puk_type == TEL_SIM_PUK_TYPE_PUK1)
			resp.type = TAPI_SIM_PTYPE_PUK1;
		else
			resp.type = TAPI_SIM_PTYPE_PUK2;
		resp.retry_count = resp_buf->retry_count;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_change_sim_pins(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimSecResult_t resp;
	TelSimSecPinResult *resp_buf = NULL;

	TelSimPinOperationResult_t res =__map_sim_result(result);

	if (res == TAPI_SIM_PIN_OPERATION_SUCCESS) {
		resp_buf = (TelSimSecPinResult *)data;
		resp.type = resp_buf->pin_type;
		resp.retry_count = resp_buf->retry_count;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_disable_sim_facility(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimFacilityResult_t resp;
	TelSimFacilityResult *resp_buf = NULL;

	TelSimPinOperationResult_t res =__map_sim_result(result);

	if (res == TAPI_SIM_PIN_OPERATION_SUCCESS) {
		resp_buf = (TelSimFacilityResult *)data;
		MAP_SIM_LOCK_TYPE_RESP(resp_buf->type, resp.type);
		resp.retry_count = resp_buf->retry_count;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_enable_sim_facility(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimFacilityResult_t resp;
	TelSimFacilityResult *resp_buf = NULL;

	TelSimPinOperationResult_t res =__map_sim_result(result);

	if (res == TAPI_SIM_PIN_OPERATION_SUCCESS) {
		resp_buf = (TelSimFacilityResult *)data;
		MAP_SIM_LOCK_TYPE_RESP(resp_buf->type, resp.type);
		resp.retry_count = resp_buf->retry_count;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_facility(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimFacilityInfo_t resp;
	TelSimFacilityInfo *resp_buf = NULL;

	TelSimPinOperationResult_t res =__map_sim_result(result);

	if (res == TAPI_SIM_PIN_OPERATION_SUCCESS) {
		resp_buf = (TelSimFacilityInfo *)data;
		MAP_SIM_LOCK_TYPE_RESP(resp_buf->type, resp.type);
		resp.f_status = resp_buf->f_status;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_get_sim_lock_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimLockInfo_t resp;
	TelSimLockInfo *resp_buf = NULL;

	TelSimPinOperationResult_t res =__map_sim_result(result);

	if (res == TAPI_SIM_PIN_OPERATION_SUCCESS) {
		resp_buf = (TelSimLockInfo *)data;
		MAP_SIM_LOCK_TYPE_RESP(resp_buf->lock_type, resp.lock_type);
		resp.lock_status = resp_buf->lock_status;
		resp.retry_count= resp_buf->retry_count;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_req_sim_apdu(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimApduResp_t resp;
	TelSimApduResp *resp_buf = NULL;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimApduResp *)data;
		if (resp_buf->apdu_resp_len) {
			resp.apdu_resp_len= resp_buf->apdu_resp_len;
			memcpy((gchar *)resp.apdu_resp, (gchar *)resp_buf->apdu_resp, resp_buf->apdu_resp_len);
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

static void on_response_req_sim_atr(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimAtrResp_t resp;
	TelSimAtr *resp_buf = NULL;

	TelSimAccessResult_t res =__map_sim_access_result(result);

	if (res == TAPI_SIM_ACCESS_SUCCESS) {
		resp_buf = (TelSimAtr *)data;
		if (resp_buf->atr_len) {
			resp.atr_resp_len = resp_buf->atr_len;
			memcpy((gchar *)resp.atr_resp, (gchar *)resp_buf->atr, resp_buf->atr_len);
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, &resp, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, res, NULL, evt_cb_data->user_data);
		}
	}

	g_free(evt_cb_data);
}

EXPORT_API int tel_get_sim_init_info(TapiHandle *handle, TelSimCardStatus_t *sim_status,
		int *card_changed)
{
	TelReturn ret;
	TelSimCardStatusInfo info_buf = {0,};

	dbg("Func Entrance");

	ret = tapi_sim_get_init_info((TelHandle *)handle, &info_buf);

	MAP_SIM_CARD_STATUS_RESP(info_buf.status, *sim_status);
	MAP_SIM_CARD_CHANGE_STATUS_RESP(info_buf.change_status, *card_changed);

	CHECK_RETURN(ret, NULL);
}

EXPORT_API int tel_get_sim_type(TapiHandle *handle, TelSimCardType_t *card_type)
{
	TelReturn ret;
	dbg("Func Entrance");
	TelSimCardType card_buf = 0;

	ret = tapi_sim_get_type((TelHandle *)handle, &card_buf);

	*card_type = card_buf;

	CHECK_RETURN(ret, NULL);
}

EXPORT_API int tel_get_sim_imsi(TapiHandle *handle, TelSimImsiInfo_t *imsi)
{
	TelReturn ret;
	TelSimImsiInfo info_buf ={{0},};

	dbg("Func Entrance");

	ret = tapi_sim_get_imsi((TelHandle *)handle, &info_buf);

	if(info_buf.mcc[0])
		g_strlcpy(imsi->szMcc, info_buf.mcc, TEL_SIM_MCC_MNC_LEN_MAX + 1);
	if(info_buf.mnc[0])
		g_strlcpy(imsi->szMnc, info_buf.mnc, TEL_SIM_MCC_MNC_LEN_MAX + 1);
	if(info_buf.msin[0])
		g_strlcpy(imsi->szMsin, info_buf.msin, TEL_SIM_MSIN_LEN_MAX + 1);

	CHECK_RETURN(ret, NULL);
}

EXPORT_API int tel_get_sim_ecc(TapiHandle *handle, TelSimEccList_t *ecc)
{
	TelReturn ret;
	TelSimEccList info_buf = {0,};
	int i;

	dbg("Func Entrance");

	ret = tapi_sim_get_ecc((TelHandle *)handle, &info_buf);

	ecc->ecc_count = info_buf.count;
	for (i = 0; i< ecc->ecc_count; i++) {
		if (info_buf.list[i].name[0])
			g_strlcpy(ecc->list[i].name, info_buf.list[i].name, 30 + 1);
		if (info_buf.list[i].number[0])
			g_strlcpy(ecc->list[i].number, info_buf.list[i].number, TEL_SIM_ECC_NUMBER_LEN_MAX + 1);
		ecc->list[i].category = info_buf.list[i].category;
	}

	CHECK_RETURN(ret, NULL);
}

EXPORT_API int tel_get_sim_iccid(TapiHandle *handle, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_get_iccid((TelHandle *)handle, on_response_get_sim_iccid, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sim_language(TapiHandle *handle, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_get_language((TelHandle *)handle, on_response_get_sim_language, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sim_language(TapiHandle *handle, TelSimLanguagePreferenceCode_t language,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_set_language((TelHandle *)handle, (TelSimLanguagePreferenceCode)language,
		on_response_set_sim_language, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sim_callforwarding_info(TapiHandle *handle, tapi_response_cb cb,
		void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_get_callforwarding_info((TelHandle *)handle, on_response_get_sim_callforwarding_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sim_callforwarding_info(TapiHandle *handle, TelSimCallForwardingReq_t *req_cf,
		tapi_response_cb cb, void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}

EXPORT_API int tel_get_sim_messagewaiting_info(TapiHandle *handle, tapi_response_cb cb,
		void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_get_messagewaiting_info((TelHandle *)handle, on_response_get_sim_messagewaiting_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sim_messagewaiting_info(TapiHandle *handle, TelSimMessageWaitingReq_t *req_mw,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret = TEL_RETURN_FAILURE;
	struct user_data_buf *cb_data = NULL;
	TelSimMwis info_buf = {0,};
	unsigned char type = 0;
	unsigned char mask = 0x01;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	if (req_mw->b_cphs) {
		info_buf.count_valid = FALSE;
		if(req_mw->mw_data_u.cphs_mw.b_voice2) {
			info_buf.profile_id = 2;
			info_buf.msg_waiting[TEL_SIM_MAILBOX_VOICE].indication = TRUE;
			info_buf.msg_waiting[TEL_SIM_MAILBOX_VOICE].count = 1;

			ret = tapi_sim_set_messagewaiting_info((TelHandle *)handle, &info_buf, on_response_set_sim_messagewaiting_info,
				(void *)cb_data);

			CHECK_RETURN(ret, cb_data);
		}

		if(req_mw->mw_data_u.cphs_mw.b_voice1) {
			info_buf.profile_id = 1;
			info_buf.msg_waiting[TEL_SIM_MAILBOX_VOICE].indication = TRUE;
			info_buf.msg_waiting[TEL_SIM_MAILBOX_VOICE].count = 1;
			if (req_mw->mw_data_u.cphs_mw.b_fax) {
				info_buf.msg_waiting[TEL_SIM_MAILBOX_FAX].indication = TRUE;
				info_buf.msg_waiting[TEL_SIM_MAILBOX_FAX].count = 1;
			}
			if (req_mw->mw_data_u.cphs_mw.b_data) {
				info_buf.msg_waiting[TEL_SIM_MAILBOX_EMAIL].indication = TRUE;
				info_buf.msg_waiting[TEL_SIM_MAILBOX_EMAIL].count = 1;
			}
			ret = tapi_sim_set_messagewaiting_info((TelHandle *)handle, &info_buf, on_response_set_sim_messagewaiting_info,
							(void *)cb_data);
		}

	} else {
		unsigned int i = 0;
		info_buf.count_valid = TRUE;
		info_buf.profile_id = 0; /*Assigning default.*/
		type = req_mw->mw_data_u.mw.indicator_status;
		if (type) {
			for (i = 0; i < 5; i++) {
				switch (type & mask) {
					case 0x01:
						info_buf.msg_waiting[TEL_SIM_MAILBOX_VOICE].indication = TRUE;
						info_buf.msg_waiting[TEL_SIM_MAILBOX_VOICE].count = req_mw->mw_data_u.mw.voice_count;
						break;
					case 0x02:
						info_buf.msg_waiting[TEL_SIM_MAILBOX_FAX].indication = TRUE;
						info_buf.msg_waiting[TEL_SIM_MAILBOX_FAX].count = req_mw->mw_data_u.mw.fax_count;
						break;
					case 0x04:
						info_buf.msg_waiting[TEL_SIM_MAILBOX_EMAIL].indication = TRUE;
						info_buf.msg_waiting[TEL_SIM_MAILBOX_EMAIL].count = req_mw->mw_data_u.mw.email_count;
					case 0x08:
						info_buf.msg_waiting[TEL_SIM_MAILBOX_OTHER].indication = TRUE;
						info_buf.msg_waiting[TEL_SIM_MAILBOX_OTHER].count = req_mw->mw_data_u.mw.other_count;
						break;
					case 0x10:
						info_buf.msg_waiting[TEL_SIM_MAILBOX_VIDEO].indication = TRUE;
						info_buf.msg_waiting[TEL_SIM_MAILBOX_VIDEO].count = req_mw->mw_data_u.mw.video_count;
						break;
				}
				mask = mask << 1;
			}
		}
		ret = tapi_sim_set_messagewaiting_info((TelHandle *)handle, &info_buf, on_response_set_sim_messagewaiting_info,
				(void *)cb_data);
	}

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sim_mailbox_info(TapiHandle *handle, tapi_response_cb cb,
		void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_get_mailbox_info((TelHandle *)handle, on_response_get_sim_mailbox_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_set_sim_mailbox_info(TapiHandle *handle, TelSimMailBoxNumber_t *req_mb,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimMailBoxNumber info_buf = {0,};

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	info_buf.profile_id = req_mb->profile_num;
	MAP_SIM_MAILBOX_TYPE(req_mb->mb_type, info_buf.mb_type);
	if (req_mb->alpha_id[0])
		g_strlcpy(info_buf.alpha_id, req_mb->alpha_id, TAPI_SIM_XDN_ALPHA_ID_MAX_LEN + 1);
	if (req_mb->num[0])
		g_strlcpy(info_buf.number, req_mb->num, TEL_SIM_MBDN_NUM_LEN_MAX + 1);

	ret = tapi_sim_set_mailbox_info((TelHandle *)handle, &info_buf, on_response_set_sim_mailbox_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_get_sim_cphs_info(TapiHandle *handle, tapi_response_cb cb,
		void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_get_sim_msisdn(TapiHandle *handle, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_get_msisdn((TelHandle *)handle, on_response_get_sim_msisdn, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sim_oplmnwact(TapiHandle *handle, tapi_response_cb cb,
		void *user_data)
{
	err("Unsupported Operation");
	return TAPI_API_NOT_SUPPORTED;
}


EXPORT_API int tel_get_sim_spn(TapiHandle *handle, tapi_response_cb cb,
		void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_get_spn((TelHandle *)handle, on_response_get_sim_spn, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_get_sim_cphs_netname(TapiHandle *handle, tapi_response_cb cb,
		void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_get_cphs_netname((TelHandle *)handle, on_response_get_sim_cphs_netname, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_req_sim_authentication(TapiHandle *handle,
		TelSimAuthenticationData_t *authentication_data, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimAuthenticationData info_buf = {0,};

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	MAP_SIM_AUTH_TYPE(authentication_data->auth_type, info_buf.auth_type);

	info_buf.autn_length = (unsigned int)authentication_data->autn_length;
	if(authentication_data->autn_data[0])
		memcpy(info_buf.autn_data, authentication_data->autn_data, authentication_data->autn_length);
	info_buf.rand_length = (unsigned int)authentication_data->rand_length;
	if(authentication_data->rand_data[0])
		memcpy(info_buf.rand_data, authentication_data->rand_data, authentication_data->rand_length);

	ret = tapi_sim_req_authentication((TelHandle *)handle, &info_buf, on_response_req_sim_authentication, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_verifiy_sim_pins(TapiHandle *handle, const TelSimSecPw_t *pin_data,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimSecPinPw info_buf = {0,};

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	info_buf.pin_type = pin_data->type;
	if (pin_data->pw_len)
		info_buf.pw = (char *)pin_data->pw;

	ret = tapi_sim_verify_pins((TelHandle *)handle, &info_buf, on_response_verify_sim_pins, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_verify_sim_puks(TapiHandle *handle, const TelSimSecPw_t *puk_data,
		const TelSimSecPw_t *new_pin_data, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimSecPukPw info_buf = {0,};

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	info_buf.puk_type = puk_data->type - 2;
	if (puk_data->pw_len)
		info_buf.puk_pw = (char *)puk_data->pw;
	if (new_pin_data->pw_len)
		info_buf.new_pin_pw = (char *)new_pin_data->pw;

	ret = tapi_sim_verify_puks((TelHandle *)handle, &info_buf, on_response_verify_sim_puks, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_change_sim_pins(TapiHandle *handle, const TelSimSecPw_t *old_pin,
		const TelSimSecPw_t *new_pin, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimSecChangePinPw info_buf = {0,};

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	info_buf.pin_type = old_pin->type;
	if (old_pin->pw_len)
		info_buf.old_pw = (char *)old_pin->pw;
	if (new_pin->pw_len)
		info_buf.new_pw = (char *)new_pin->pw;

	ret = tapi_sim_change_pins((TelHandle *)handle, &info_buf, on_response_change_sim_pins, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_disable_sim_facility(TapiHandle *handle, TelSimFacilityPw_t *pw,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimFacilityPw info_buf = {0,};

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	MAP_SIM_LOCK_TYPE(pw->lock_type, info_buf.lock_type);

	if (pw->pw_len)
		info_buf.pw = (char *)pw->pw;

	ret = tapi_sim_disable_facility((TelHandle *)handle, &info_buf, on_response_disable_sim_facility, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_enable_sim_facility(TapiHandle *handle, TelSimFacilityPw_t *pw,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimFacilityPw info_buf = {0,};

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	MAP_SIM_LOCK_TYPE(pw->lock_type, info_buf.lock_type);

	if (pw->pw_len)
		info_buf.pw = (char *)pw->pw;

	ret = tapi_sim_enable_facility((TelHandle *)handle, &info_buf, on_response_enable_sim_facility, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_get_sim_facility(TapiHandle *handle, TelSimLockType_t type,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimLockType req_type;

	dbg("Func Entrance");

	MAP_SIM_LOCK_TYPE(type, req_type);

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);
	ret = tapi_sim_get_facility((TelHandle *)handle, req_type, on_response_get_sim_facility, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_get_sim_lock_info(TapiHandle *handle, TelSimLockType_t type,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimLockType req_type;

	dbg("Func Entrance");

	MAP_SIM_LOCK_TYPE(type, req_type);

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);
	ret = tapi_sim_get_lock_info((TelHandle *)handle, req_type, on_response_get_sim_lock_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_req_sim_apdu(TapiHandle *handle, TelSimApdu_t* apdu_data,
		tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelSimApdu info_buf = {0,};

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	if (apdu_data->apdu_len) {
		memcpy((gchar *)info_buf.apdu, (gchar *)apdu_data->apdu, apdu_data->apdu_len);
		info_buf.apdu_len = apdu_data->apdu_len;
	}

	ret = tapi_sim_req_apdu((TelHandle *)handle, &info_buf, on_response_req_sim_apdu, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}


EXPORT_API int tel_req_sim_atr(TapiHandle *handle, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_sim_req_atr((TelHandle *)handle, on_response_req_sim_atr, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}
