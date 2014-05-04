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
#include <ITapiSim.h>
#include <TapiUtility.h>

#include "menu.h"
#include "sim.h"

static char data_sim_set_language[MENU_DATA_SIZE + 1] = { 0x01,};

static char data_sim_set_mw_b_cphs[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_rec_index[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_indicator_status[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_voice_cnt[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_fax_cnt[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_email_cnt[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_other_cnt[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_video_cnt[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_cphs_voice1[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_cphs_voice2[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_cphs_fax[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mw_cphs_data[MENU_DATA_SIZE +1] = {};

static char data_sim_set_mb_b_cphs[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_type[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_rec_index[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_profile_num[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_alpha_id_max_len[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_alpha_id[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_ton[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_npi[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_number[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_cc_id[MENU_DATA_SIZE +1] = {};
static char data_sim_set_mb_ext1_id[MENU_DATA_SIZE +1] = {};

static char data_sim_req_authentication_type[MENU_DATA_SIZE + 1] = { 0x01,}; //SIM_AUTH_TYPE_GSM
static char data_sim_req_authentication_autn_data[MENU_DATA_SIZE + 1] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, };
static char data_sim_req_authentication_rand_data[MENU_DATA_SIZE + 1] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, };

static char data_sim_verify_pins_type[MENU_DATA_SIZE +1] = {};
static char data_sim_verify_pins_pw[MENU_DATA_SIZE +1] = {};

static char data_sim_verify_puks_type[MENU_DATA_SIZE +1] = {};
static char data_sim_verify_puks_puk[MENU_DATA_SIZE +1] = {};
static char data_sim_verify_puks_pin[MENU_DATA_SIZE +1] = {};

static char data_sim_change_pins_type[MENU_DATA_SIZE +1] = {};
static char data_sim_change_pins_old_pw[MENU_DATA_SIZE +1] = {};
static char data_sim_change_pins_new_pw[MENU_DATA_SIZE +1] = {};

static char data_sim_disable_facility_type[MENU_DATA_SIZE +1] = {};
static char data_sim_disable_facility_pw[MENU_DATA_SIZE +1] = {};

static char data_sim_enable_facility_type[MENU_DATA_SIZE +1] = {};
static char data_sim_enable_facility_pw[MENU_DATA_SIZE +1] = {};

static char data_sim_get_facility_type[MENU_DATA_SIZE +1] = {};

static char data_sim_get_lock_type[MENU_DATA_SIZE +1] = {};

static char data_sim_transmit_apdu_apdu[MENU_DATA_SIZE +1] = {};


static int _sim_str_to_hex(char a)
{
	int ret = -1;

	if (a >= '0' && a <= '9')
		ret = a - '0';
	else if (a >= 'a' && a <= 'f')
		ret = a - 'a' + 10;

	return ret;
}


static void on_noti_sim_status(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelSimCardStatus_t *status = data;

	msgb("event(%s) receive !!", TAPI_NOTI_SIM_STATUS);

	if (!status)
		return;

	msg("status: [%d]", *status);
}

static int run_sim_get_init_info(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimCardStatus_t o_status = 0;
	gboolean o_new = FALSE;

	msg("call [%s] tapi service !!!", menu->title);

	rt = tel_get_sim_init_info(handle, &o_status, &o_new);

	CHECK_RESULT(rt);

	msg("status[%d]", o_status);
	msg("new_sim[%d]", o_new);
	return 0;
}

static int run_sim_get_card_type(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimCardType_t o_type = 0;

	msg("call [%s] tapi service !!!", menu->title);

	rt = tel_get_sim_type(handle, &o_type);

	CHECK_RESULT(rt);

	msg("card type[%d]", o_type);
	return 0;
}

static int run_sim_get_imsi(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimImsiInfo_t imsi;
	memset(&imsi, 0, sizeof(TelSimImsiInfo_t));

	msg("call [%s] tapi service !!!", menu->title);

	rt = tel_get_sim_imsi(handle, &imsi);

	CHECK_RESULT(rt);

	msg("mcc[%s]", imsi.szMcc);
	msg("mnc[%s]", imsi.szMnc);
	msg("msin[%s]", imsi.szMsin);

	return 0;
}

static int run_sim_get_ecc(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimEccList_t ecc_list;
	int i =0;
	memset(&ecc_list, 0, sizeof(TelSimEccList_t));

	msg("call [%s] tapi service !!!", menu->title);

	rt = tel_get_sim_ecc(handle, &ecc_list);

	CHECK_RESULT(rt);

	msg("ecc_list.ecc_count[%d]",ecc_list.ecc_count);

	for(i =0; i < ecc_list.ecc_count; i++) {
		msg("ecc_list.list[%d].name[%s]",i, ecc_list.list[i].name);
		msg("ecc_list.list[%d].number[%s]",i, ecc_list.list[i].number);
		msg("ecc_list.list[%d].category[%d]",i, ecc_list.list[i].category);
	}

	return 0;
}

static void on_sim_get_iccid(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimIccIdInfo_t *iccid = data;

	msg("TAPI_SERVICE_SIM_GET_ICCID response received: [%d]", result);

	if (!iccid)
		return;

	msg("iccid length[%d]", iccid->icc_length);
	msg("iccid[%s]", iccid->icc_num);
}

static int run_sim_get_iccid(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);

	rt = tel_get_sim_iccid(handle, on_sim_get_iccid, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_language(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimLanguagePreferenceCode_t *language = data;

	msg("TAPI_SERVICE_SIM_GET_LANGUAGE response received: [%d]", result);

	if (!language)
		return;

	msg("language[%d]", *language);
}

static int run_sim_get_language(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);

	rt = tel_get_sim_language(handle, on_sim_get_language, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_set_language(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("TAPI_SERVICE_SIM_SET_LANGUAGE response received: [%d]", result);
}

static int run_sim_set_language(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	TelSimLanguagePreferenceCode_t language;
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);
	language = atoi(data_sim_set_language);
	rt = tel_set_sim_language(handle, language, on_sim_set_language, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_cf(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimCallForwardingResp_t *cf = data;
	int i = 0;
	msg("TAPI_SERVICE_SIM_GET_CF response received");

	msg("access_rt[%d]", access_rt);

	if (!cf)
		return;

	msg("cf->b_cphs[%d]", cf->b_cphs);

	if(cf->b_cphs){
		msg("cf->cphs_cf.b_line1[%d]", cf->cphs_cf.b_line1);
		msg("cf->cphs_cf.b_line2[%d]", cf->cphs_cf.b_line2);
		msg("cf->cphs_cf.b_fax[%d]", cf->cphs_cf.b_fax);
		msg("cf->cphs_cf.b_data[%d]", cf->cphs_cf.b_data);
	} else {
		for(i=0; i< cf->cf_list.profile_count; i++) {
			msg("cf->cf_list.cf[%d].rec_index[%d]", i, cf->cf_list.cf[i].rec_index);
			msg("cf->cf_list.cf[%d].msp_num[%d]", i, cf->cf_list.cf[i].msp_num);
			msg("cf->cf_list.cf[%d].cfu_status[0x%x]", i, cf->cf_list.cf[i].cfu_status);
			msg("cf->cf_list.cf[%d].cfu_num[%s]", i, cf->cf_list.cf[i].cfu_num);
			msg("cf->cf_list.cf[%d].ton[%d]", i, cf->cf_list.cf[i].ton);
			msg("cf->cf_list.cf[%d].npi[%d]", i, cf->cf_list.cf[i].npi);
			msg("cf->cf_list.cf[%d].cc2_id[0x%x]", i, cf->cf_list.cf[i].cc2_id);
			msg("cf->cf_list.cf[%d].ext7_id[0x%x]", i, cf->cf_list.cf[i].ext7_id);
		}
	}
}

static int run_sim_get_cf(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);
	rt = tel_get_sim_callforwarding_info(handle, on_sim_get_cf, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_mw(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimMessageWaitingResp_t *mw = data;
	int i = 0;
	msg("TAPI_SERVICE_SIM_GET_MW response received");

	msg("access_rt[%d]", access_rt);

	if (!mw)
		return;

	msg("mw->b_cphs[%d]",mw->b_cphs);

	if(mw->b_cphs) {
		msg("mw->cphs_mw.b_voice1[%d]",mw->cphs_mw.b_voice1);
		msg("mw->cphs_mw.b_voice2[%d]",mw->cphs_mw.b_voice2);
		msg("mw->cphs_mw.b_fax[%d]",mw->cphs_mw.b_fax);
		msg("mw->cphs_mw.b_data[%d]",mw->cphs_mw.b_data);
	} else {
		for(i=0;i<mw->mw_list.profile_count; i++){
			msg("mw->mw_list.mw[%d].rec_index[%d]",i, mw->mw_list.mw[i].rec_index);
			msg("mw->mw_list.mw[%d].indicator_status[0x%x]",i, mw->mw_list.mw[i].indicator_status);
			msg("mw->mw_list.mw[%d].voice_count[%d]",i, mw->mw_list.mw[i].voice_count);
			msg("mw->mw_list.mw[%d].fax_count[%d]",i, mw->mw_list.mw[i].fax_count);
			msg("mw->mw_list.mw[%d].email_count[%d]",i, mw->mw_list.mw[i].email_count);
			msg("mw->mw_list.mw[%d].other_count[%d]",i, mw->mw_list.mw[i].other_count);
			msg("mw->mw_list.mw[%d].video_count[%d]",i, mw->mw_list.mw[i].video_count);
		}
	}
}

static int run_sim_get_mw(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);
	rt = tel_get_sim_messagewaiting_info(handle, on_sim_get_mw, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_set_mw(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("Message waiting info response received: [%d]", result);
}

static int run_sim_set_mw(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimMessageWaitingReq_t mw;

	memset(&mw, 0, sizeof(TelSimMessageWaitingReq_t));

	msg("call [%s] tapi service !!!", menu->title);

	mw.b_cphs = atoi(data_sim_set_mw_b_cphs);

	if(mw.b_cphs) {
		mw.mw_data_u.cphs_mw.b_voice1 = atoi(data_sim_set_mw_cphs_voice1);
		mw.mw_data_u.cphs_mw.b_voice2 = atoi(data_sim_set_mw_cphs_voice2);
		mw.mw_data_u.cphs_mw.b_fax = atoi(data_sim_set_mw_cphs_fax);
		mw.mw_data_u.cphs_mw.b_data = atoi(data_sim_set_mw_cphs_data);
	} else {
		mw.mw_data_u.mw.rec_index = atoi(data_sim_set_mw_rec_index);
		mw.mw_data_u.mw.indicator_status = atoi(data_sim_set_mw_indicator_status);
		mw.mw_data_u.mw.voice_count = atoi(data_sim_set_mw_voice_cnt);
		mw.mw_data_u.mw.fax_count = atoi(data_sim_set_mw_fax_cnt);
		mw.mw_data_u.mw.email_count = atoi(data_sim_set_mw_email_cnt);
		mw.mw_data_u.mw.other_count = atoi(data_sim_set_mw_other_cnt);
		mw.mw_data_u.mw.video_count = atoi(data_sim_set_mw_video_cnt);
	}

	rt = tel_set_sim_messagewaiting_info(handle, &mw, on_sim_set_mw, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_mb(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimMailboxList_t *list = data;
	int i = 0;

	msg("TAPI_SERVICE_SIM_GET_MB response received");

	msg("access_rt[%d]", access_rt);

	if (!list)
		return;

	msg("count[%d]", list->count);

	for (i = 0; i < list->count; i++) {
		msg("list->list[%d].b_cphs[%d]",i, list->list[i].b_cphs);
		msg("list->list[%d].rec_index[%d]",i, list->list[i].rec_index);
		msg("list->list[%d].profile_num[%d]",i, list->list[i].profile_num);
		msg("list->list[%d].mb_type[%d]",i, list->list[i].mb_type);
		msg("list->list[%d].alpha_id_max_len[%d]",i, list->list[i].alpha_id_max_len);
		msg("list->list[%d].alpha_id[%s]",i, list->list[i].alpha_id);
		msg("list->list[%d].ton[0x%x]",i, list->list[i].ton);
		msg("list->list[%d].npi[0x%x]",i, list->list[i].npi);
		msg("list->list[%d].num[%s]",i, list->list[i].num);
		msg("list->list[%d].cc_id[0x%x]",i, list->list[i].cc_id);
		msg("list->list[%d].ext1_id[0x%x]",i, list->list[i].ext1_id);
	}
}

static int run_sim_get_mb(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);
	rt = tel_get_sim_mailbox_info(handle, on_sim_get_mb, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_set_mb(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("Mailbox info response received: [%d]", result);
}

static int run_sim_set_mb(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimMailBoxNumber_t mb;

	msg("call [%s] tapi service !!!", menu->title);

	memset(&mb, 0, sizeof(TelSimMailBoxNumber_t));

	mb.b_cphs = atoi(data_sim_set_mb_b_cphs);
	mb.mb_type = atoi(data_sim_set_mb_type);
	mb.rec_index = atoi(data_sim_set_mb_rec_index);
	mb.profile_num = atoi(data_sim_set_mb_profile_num);
	mb.alpha_id_max_len = atoi(data_sim_set_mb_alpha_id_max_len);
	memcpy(&mb.alpha_id, data_sim_set_mb_alpha_id, strlen(data_sim_set_mb_alpha_id));
	mb.ton = atoi(data_sim_set_mb_ton);
	mb.npi = atoi(data_sim_set_mb_npi);
	memcpy(&mb.num, data_sim_set_mb_number, strlen(data_sim_set_mb_number));
	mb.cc_id = atoi(data_sim_set_mb_cc_id);
	mb.ext1_id = atoi(data_sim_set_mb_ext1_id);

	rt = tel_set_sim_mailbox_info(handle, &mb, on_sim_set_mb, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_msisdn(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimMsisdnList_t *list = data;
	int i =0;
	msg("TAPI_SERVICE_SIM_GET_MSISDN response received");

	msg("access_rt[%d]", access_rt);

	if (!list)
		return;

	msg("list->count[%d]", list->count);

	for (i = 0; i < list->count; i++)
		msg("index[%d] - name[%s], num[%s]", i, list->list[i].name, list->list[i].num);
}

static int run_sim_get_msisdn(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);
	rt = tel_get_sim_msisdn(handle, on_sim_get_msisdn, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_spn(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimSpn_t *spn_info = data;

	msg("TAPI_SERVICE_SIM_GET_SPN response received");
	msg("access_rt[%d]", access_rt);

	if (!spn_info)
		return;

	msg("spn name[%s]", spn_info->spn);
	msg("spn condition[0x%x]", spn_info->display_condition);
}

static int run_sim_get_spn(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);
	rt = tel_get_sim_spn(handle, on_sim_get_spn, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_cphs_netname(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimCphsNetName_t *cphs_net = data;

	msg("TAPI_SERVICE_SIM_GET_CPHS_NETNAME response received");
	msg("access_rt[%d]", access_rt);

	if (!cphs_net)
		return;

	msg("cphs full name[%s]", cphs_net->full_name);
	msg("cphs short name[%s]", cphs_net->short_name);
}

static int run_sim_get_cphs_netname(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);
	rt = tel_get_sim_cphs_netname(handle, on_sim_get_cphs_netname, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_req_authentication(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimAuthenticationResponse_t *auth_resp = data;
	int i = 0;
	msg("TAPI_SERVICE_SIM_AUTHENTICATION response received");

	msg("access_rt[%d]", access_rt);

	if (!auth_resp)
		return;

	msg("auth_resp->auth_type[%d]", auth_resp->auth_type);
	msg("auth_resp->auth_result[%d]", auth_resp->auth_result);

	msg("auth_resp->authentication_key_length[%d]", auth_resp->authentication_key_length);
	if (auth_resp->authentication_key_length){
		for( i = 0; i < auth_resp->authentication_key_length; i++)
			msg("auth_resp->authentication_key[%d][0x%2x]", i, auth_resp->authentication_key[i]);
	}

	msg("auth_resp->cipher_length[%d]", auth_resp->cipher_length);
	if (auth_resp->cipher_length){
		for( i = 0; i < auth_resp->cipher_length; i++)
			msg("auth_resp->cipher_data[%d][0x%2x]", i, auth_resp->cipher_data[i]);
	}

	msg("auth_resp->integrity_length[%d]", auth_resp->integrity_length);
	if (auth_resp->integrity_length){
		for( i = 0; i < auth_resp->integrity_length; i++)
			msg("auth_resp->integrity_data[%d][0x%2x]", i, auth_resp->integrity_data[i]);
	}

	msg("auth_resp->resp_length[%d]", auth_resp->resp_length);
	if (auth_resp->resp_length){
		for( i = 0; i < auth_resp->resp_length; i++)
			msg("auth_resp->resp_data[%d][0x%2x]", i, auth_resp->resp_data[i]);
	}

}

static int run_sim_req_authentication(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimAuthenticationData_t auth;
	memset(&auth, 0, sizeof(TelSimAuthenticationData_t));

	msg("call [%s] tapi service !!!", menu->title);

	auth.auth_type = atoi(data_sim_req_authentication_type);
	auth.autn_length = strlen(data_sim_req_authentication_autn_data);
	memcpy(&auth.autn_data, data_sim_req_authentication_autn_data, auth.autn_length);
	auth.rand_length = strlen(data_sim_req_authentication_rand_data);
	memcpy(&auth.rand_data, data_sim_req_authentication_rand_data, auth.rand_length);

	rt = tel_req_sim_authentication(handle, &auth, on_sim_req_authentication, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_security_op_resp(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimPinOperationResult_t sec_rt = result;
	TelSimSecResult_t *rt = data;

	msg("TAPI_SERVICE_SIM security response received");

	msg("sec_rt[%d]", sec_rt);

	if (!rt)
		return;

	msg("rt->type[%d]", rt->type);
	msg("rt->retry_count[%d]", rt->retry_count);
}

static int run_sim_verify_pins(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimSecPw_t pwt;
	memset(&pwt, 0, sizeof(TelSimSecPw_t));

	msg("call [%s] tapi service !!!", menu->title);

	pwt.type = atoi(data_sim_verify_pins_type);
	pwt.pw_len = strlen(data_sim_verify_pins_pw);
	pwt.pw = (unsigned char*) malloc(pwt.pw_len + 1);
	g_strlcpy((gchar *)pwt.pw, data_sim_verify_pins_pw, pwt.pw_len + 1);

	rt = tel_verifiy_sim_pins(handle, &pwt, on_sim_security_op_resp, NULL);

	CHECK_RESULT(rt);
	free(pwt.pw);

	return 0;
}

static int run_sim_verify_puks(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimSecPw_t puk_t;
	TelSimSecPw_t new_pin_t;
	memset(&puk_t, 0, sizeof(TelSimSecPw_t));
	memset(&new_pin_t, 0, sizeof(TelSimSecPw_t));
	msg("call [%s] tapi service !!!", menu->title);

	puk_t.type = atoi(data_sim_verify_puks_type);
	puk_t.pw_len = strlen(data_sim_verify_puks_puk);
	puk_t.pw = (unsigned char*) malloc(puk_t.pw_len + 1);
	g_strlcpy((gchar *)puk_t.pw, data_sim_verify_puks_puk, puk_t.pw_len + 1);

	if(puk_t.type == TAPI_SIM_PTYPE_PUK1)
		new_pin_t.type = TAPI_SIM_PTYPE_PIN1;
	if(puk_t.type == TAPI_SIM_PTYPE_PUK2)
		new_pin_t.type = TAPI_SIM_PTYPE_PIN2;

	new_pin_t.pw_len = strlen(data_sim_verify_puks_pin);
	new_pin_t.pw = (unsigned char*) malloc(new_pin_t.pw_len + 1);
	g_strlcpy((gchar *)new_pin_t.pw, data_sim_verify_puks_pin, new_pin_t.pw_len + 1);

	rt = tel_verify_sim_puks(handle, &puk_t, &new_pin_t, on_sim_security_op_resp, NULL);

	CHECK_RESULT(rt);
	free(puk_t.pw);
	free(new_pin_t.pw);

	return 0;
}

static int run_sim_change_pins(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimSecPw_t old;
	TelSimSecPw_t new;
	memset(&old, 0, sizeof(TelSimSecPw_t));
	memset(&new, 0, sizeof(TelSimSecPw_t));
	msg("call [%s] tapi service !!!", menu->title);

	old.type = atoi(data_sim_change_pins_type);
	old.pw_len = strlen(data_sim_change_pins_old_pw);
	old.pw = (unsigned char*) malloc(old.pw_len + 1);
	g_strlcpy((gchar *)old.pw, data_sim_change_pins_old_pw, old.pw_len + 1);

	new.type = atoi(data_sim_change_pins_type);
	new.pw_len = strlen(data_sim_change_pins_new_pw);
	new.pw = (unsigned char*) malloc(new.pw_len + 1);
	g_strlcpy((gchar *)new.pw, data_sim_change_pins_new_pw, new.pw_len + 1);

	rt = tel_change_sim_pins(handle, &old, &new, on_sim_security_op_resp, NULL);

	CHECK_RESULT(rt);
	free(old.pw);
	free(new.pw);

	return 0;
}

static void on_sim_facility_op_resp(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimPinOperationResult_t sec_rt = result;
	TelSimFacilityResult_t *f_rt = data;

	msg("TAPI_SERVICE_SIM facility response received");

	msg("sec_rt[%d]", sec_rt);

	if (!f_rt)
		return;

	msg("f_rt->type[%d]", f_rt->type);
	msg("f_rt->retry_count[%d]", f_rt->retry_count);
}

static int run_sim_disable_facility(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimFacilityPw_t fpw;
	msg("call [%s] tapi service !!!", menu->title);

	memset(&fpw, 0, sizeof(fpw));

	fpw.lock_type = atoi(data_sim_disable_facility_type);
	fpw.pw_len = strlen(data_sim_disable_facility_pw);
	fpw.pw = (unsigned char*) malloc(fpw.pw_len + 1);
	g_strlcpy((gchar *)fpw.pw, data_sim_disable_facility_pw, fpw.pw_len + 1);
	msg("fpw.lock_type[%d], fpw.pw_len[%d], fpw.pw[%s]", fpw.lock_type, fpw.pw_len, fpw.pw);
	rt = tel_disable_sim_facility(handle, &fpw, on_sim_facility_op_resp, NULL);

	CHECK_RESULT(rt);
	free(fpw.pw);

	return 0;
}

static int run_sim_enable_facility(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimFacilityPw_t fpw;
	msg("call [%s] tapi service !!!", menu->title);

	memset(&fpw, 0, sizeof(fpw));

	fpw.lock_type = atoi(data_sim_enable_facility_type);
	fpw.pw_len = strlen(data_sim_enable_facility_pw);
	fpw.pw = (unsigned char*) malloc(fpw.pw_len + 1);
	g_strlcpy((gchar *)fpw.pw, data_sim_enable_facility_pw, fpw.pw_len + 1);
	msg("fpw.lock_type[%d], fpw.pw_len[%d], fpw.pw[%s]", fpw.lock_type, fpw.pw_len, fpw.pw);
	rt = tel_enable_sim_facility(handle, &fpw, on_sim_facility_op_resp, NULL);

	CHECK_RESULT(rt);
	free(fpw.pw);

	return 0;
}

static void on_sim_get_facility(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimPinOperationResult_t sec_rt = result;
	TelSimFacilityInfo_t *fi = data;

	msg("TAPI_SERVICE_SIM facility response received");

	msg("sec_rt[%d]", sec_rt);

	if (!fi)
		return;

	msg("fi->type[%d]", fi->type);
	msg("fi->f_status[%d]", fi->f_status);
}

static int run_sim_get_facility(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimLockType_t t =0;
	msg("call [%s] tapi service !!!", menu->title);

	switch (atoi(data_sim_get_facility_type)){
		case 1:
			t = TAPI_SIM_LOCK_PS;
			break;
		case 2:
			t = TAPI_SIM_LOCK_PF;
			break;
		case 3:
			t = TAPI_SIM_LOCK_SC;
			break;
		case 4:
			t = TAPI_SIM_LOCK_FD;
			break;
		case 5:
			t = TAPI_SIM_LOCK_PN;
			break;
		case 6:
			t = TAPI_SIM_LOCK_PU;
			break;
		case 7:
			t = TAPI_SIM_LOCK_PP;
			break;
		case 8:
			t = TAPI_SIM_LOCK_PC;
			break;
		default:
			msg("not handled type[%d]",atoi(data_sim_get_facility_type));
			break;
	}
	rt = tel_get_sim_facility(handle, t, on_sim_get_facility, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_lock_info(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimPinOperationResult_t sec_rt = result;
	TelSimLockInfo_t *lock = data;

	msg("TAPI_SERVICE_SIM lock info response received");

	msg("sec_rt[%d]", sec_rt);

	if (!lock)
		return;

	msg("lock->lock_type[%d]", lock->lock_type);
	msg("lock->lock_status[%d]-0:not need, 1:pin, 2:puk, 3:pin2, 4:puk2, 5:blocked", lock->lock_status);
	msg("lock->retry_count[%d]", lock->retry_count);
}

static int run_sim_get_lock_info(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimLockType_t t =0;
	msg("call [%s] tapi service !!!", menu->title);

	switch (atoi(data_sim_get_lock_type)){
		case 1:
			t = TAPI_SIM_LOCK_PS;
			break;
		case 2:
			t = TAPI_SIM_LOCK_PF;
			break;
		case 3:
			t = TAPI_SIM_LOCK_SC;
			break;
		case 4:
			t = TAPI_SIM_LOCK_FD;
			break;
		case 5:
			t = TAPI_SIM_LOCK_PN;
			break;
		case 6:
			t = TAPI_SIM_LOCK_PU;
			break;
		case 7:
			t = TAPI_SIM_LOCK_PP;
			break;
		case 8:
			t = TAPI_SIM_LOCK_PC;
			break;
		default:
			msg("not handled type[%d]", atoi(data_sim_get_lock_type));
			break;
	}
	rt = tel_get_sim_lock_info(handle, t, on_sim_get_lock_info, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_transmit_apdu(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimApduResp_t *r_apdu = data;
	int i =0;

	msg("TAPI_SERVICE_SIM_APDU response received");

	msg("access_rt[%d]", access_rt);

	if (!r_apdu)
		return;

	msg("r_apdu->apdu_resp_len[%d]", r_apdu->apdu_resp_len);
	if (r_apdu->apdu_resp_len) {
		for(i=0; i < r_apdu->apdu_resp_len; i++)
			msg("[%d]`s byte is [0x%x]",i, r_apdu->apdu_resp[i]);
	}
}

static int run_sim_transmit_apdu(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	TelSimApdu_t t_apdu;
	unsigned char result_apdu[MENU_DATA_SIZE +1] = {0,};
	int h = 0;
	int i, len;
	int t_i;

	memset(&t_apdu, 0, sizeof(TelSimApdu_t));

	msg("call [%s] tapi service !!!", menu->title);

	len = strlen(data_sim_transmit_apdu_apdu);

	for (i=0,t_i=0; i<len; i+=2,t_i++) {
		h = (_sim_str_to_hex(data_sim_transmit_apdu_apdu[i]) << 4) | (_sim_str_to_hex(data_sim_transmit_apdu_apdu[i+1]));
		printf("0x%x\n", h);

		result_apdu[t_i] = h;
	}

	for (i=0; i< len/2; i++) {
		printf("[%02d] = 0x%02x\n", i, result_apdu[i]);
	}

	t_apdu.apdu_len = len/2;
	t_apdu.apdu = (unsigned char*) malloc(t_apdu.apdu_len);
	memcpy(t_apdu.apdu, result_apdu, t_apdu.apdu_len);

	rt = tel_req_sim_apdu(handle, &t_apdu, on_sim_transmit_apdu, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_sim_get_atr(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimAtrResp_t *r_atr  = data;
	int i =0;
	msg("TAPI_SERVICE_SIM_APDU response received");

	msg("access_rt[%d]", access_rt);

	if (!r_atr)
		return;

	msg("r_atr->atr_resp_len[%d]", r_atr->atr_resp_len);
	if (r_atr->atr_resp_len) {
		for(i=0; i < r_atr->atr_resp_len; i++)
			msg("[%d]`s byte is [0x%x]",i, r_atr->atr_resp[i]);
	}
}

static int run_sim_get_atr(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call [%s] tapi service !!!", menu->title);

	rt = tel_req_sim_atr(handle, on_sim_get_atr, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static struct menu_data menu_sim_get_init_info[] = {
		{ "1", "run", NULL, run_sim_get_init_info, NULL},
		{ NULL, NULL, },
};

static struct menu_data menu_sim_get_card_type[] = {
		{ "1", "run", NULL, run_sim_get_card_type, NULL},
		{ NULL, NULL, },
};

static struct menu_data menu_sim_get_imsi[] = {
		{ "1", "run", NULL, run_sim_get_imsi, NULL},
		{ NULL, NULL, },
};

static struct menu_data menu_sim_get_ecc[] = {
		{ "1", "run", NULL, run_sim_get_ecc, NULL},
		{ NULL, NULL, },
};

static struct menu_data menu_sim_get_iccid[] = {
		{ "1", "run", NULL, run_sim_get_iccid, NULL},
		{ NULL, NULL, },
};

static struct menu_data menu_sim_get_language[] = {
		{"1", "run", NULL, run_sim_get_language, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_set_language[] = {
		{"1", "select language", NULL, NULL, (char*)data_sim_set_language},
		{"2", "run", NULL, run_sim_set_language, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_cf[] = {
		{"1", "run", NULL, run_sim_get_cf, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_mw[] = {
		{"1", "run", NULL, run_sim_get_mw, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_set_mw[] = {
		{"1", "b_cphs no/yes => 0/1", NULL, NULL, data_sim_set_mw_b_cphs},
		{"2", "rec_index int", NULL, NULL, data_sim_set_mw_rec_index},
		{"3", "indicator_status int", NULL, NULL, data_sim_set_mw_indicator_status},
		{"4", "voice_cnt int", NULL, NULL, data_sim_set_mw_voice_cnt},
		{"5", "fax_cnt int", NULL, NULL, data_sim_set_mw_fax_cnt},
		{"6", "email_cnt int", NULL, NULL, data_sim_set_mw_email_cnt},
		{"7", "other_cnt int", NULL, NULL, data_sim_set_mw_other_cnt},
		{"8", "video_cnt int", NULL, NULL, data_sim_set_mw_video_cnt},
		{"9", "cphs_voice1 no/yes => 0/1", NULL, NULL, data_sim_set_mw_cphs_voice1},
		{"10", "cphs_voice2 no/yes => 0/1", NULL, NULL, data_sim_set_mw_cphs_voice2},
		{"11", "cphs_fax no/yes => 0/1", NULL, NULL, data_sim_set_mw_cphs_fax},
		{"12", "cphs_data no/yes => 0/1", NULL, NULL, data_sim_set_mw_cphs_data},
		{"13", "run", NULL, run_sim_set_mw, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_mb[] = {
		{"1", "run", NULL, run_sim_get_mb, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_set_mb[] = {
		{"1", "b_cphs no/yes => 0/1", NULL, NULL, data_sim_set_mb_b_cphs},
		{"2", "type 1voice 2voice2 3fax 4data 5email 6other  int", NULL, NULL, data_sim_set_mb_type},
		{"3", "rec_index int", NULL, NULL, data_sim_set_mb_rec_index},
		{"4", "profile_num int", NULL, NULL, data_sim_set_mb_profile_num},
		{"5", "alpha_id_max_len int", NULL, NULL, data_sim_set_mb_alpha_id_max_len},
		{"6", "alpha_id string", NULL, NULL, data_sim_set_mb_alpha_id},
		{"7", "ton int", NULL, NULL, data_sim_set_mb_ton},
		{"8", "npi int", NULL, NULL, data_sim_set_mb_npi},
		{"9", "number string", NULL, NULL, data_sim_set_mb_number},
		{"10", "cc_id int", NULL, NULL, data_sim_set_mb_cc_id},
		{"11", "ext1_id int", NULL, NULL, data_sim_set_mb_ext1_id},
		{"12", "run", NULL, run_sim_set_mb, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_msisdn[] = {
		{"1", "run", NULL, run_sim_get_msisdn, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_spn[] = {
		{"1", "run", NULL, run_sim_get_spn, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_cphs_netname[] = {
		{"1", "run", NULL, run_sim_get_cphs_netname, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_req_authentication[] = {
		{"1", "type", NULL, NULL, (char*)data_sim_req_authentication_type},
		{"2", "autn_data", NULL, NULL, data_sim_req_authentication_autn_data},
		{"3", "rand_data", NULL, NULL, data_sim_req_authentication_rand_data},
		{"4", "run", NULL, run_sim_req_authentication, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_verify_pins[] = {
		{"1", "type(0:pin1, 1:pin2, 6:sim)", NULL, NULL, (char*)data_sim_verify_pins_type},
		{"2", "pw", NULL, NULL, data_sim_verify_pins_pw},
		{"3", "run", NULL, run_sim_verify_pins, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_verify_puks[] = {
		{"1", "type(2:puk1, 3:puk2)", NULL, NULL, (char*)data_sim_verify_puks_type},
		{"2", "puk", NULL, NULL, data_sim_verify_puks_puk},
		{"3", "pin", NULL, NULL, data_sim_verify_puks_pin},
		{"4", "run", NULL, run_sim_verify_puks, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_change_pins[] = {
		{"1", "type(0:pin1, 1:pin2)", NULL, NULL, (char*)data_sim_change_pins_type},
		{"2", "old_pw", NULL, NULL, data_sim_change_pins_old_pw},
		{"3", "new_pw", NULL, NULL, data_sim_change_pins_new_pw},
		{"4", "run", NULL, run_sim_change_pins, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_disable_facility[] = {
		{"1", "type(1:simlock, 3:pinlock., 4:fdnlock, 5:net, 6:netsub, 7:sp, 8:cp)", NULL, NULL, (char*)data_sim_disable_facility_type},
		{"2", "pw", NULL, NULL, data_sim_disable_facility_pw},
		{"3", "run", NULL, run_sim_disable_facility, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_enable_facility[] = {
		{"1", "type(1:simlock, 3:pinlock., 4:fdnlock, 5:net, 6:netsub, 7:sp, 8:cp)", NULL, NULL, (char*)data_sim_enable_facility_type},
		{"2", "pw", NULL, NULL, data_sim_enable_facility_pw},
		{"3", "run", NULL, run_sim_enable_facility, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_facility[] = {
		{"1", "type(1:simlock, 3:pinlock., 4:fdnlock, 5:net, 6:netsub, 7:sp, 8:cp)", NULL, NULL, (char*)data_sim_get_facility_type},
		{"2", "run", NULL, run_sim_get_facility, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_lock_info[] = {
		{"1", "type(1:simlock, 3:pinlock., 4:fdnlock, 5:net, 6:netsub, 7:sp, 8:cp)", NULL, NULL, (char*)data_sim_get_lock_type},
		{"2", "run", NULL, run_sim_get_lock_info, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_transmit_apdu[] = {
		{"1", "apdu", NULL, NULL, data_sim_transmit_apdu_apdu},
		{"2", "run", NULL, run_sim_transmit_apdu, NULL},
		{NULL, NULL},
};

static struct menu_data menu_sim_get_atr[] = {
		{"1", "run", NULL, run_sim_get_atr, NULL},
		{NULL, NULL},
};


struct menu_data menu_sim[] = {
	{"1", "sim init info", menu_sim_get_init_info, NULL, NULL},
	{"2", "card type", menu_sim_get_card_type, NULL, NULL},
	{"3", "imsi", menu_sim_get_imsi, NULL, NULL},
	{"4", "ecc", menu_sim_get_ecc, NULL, NULL},
	{"5", "iccid", menu_sim_get_iccid, NULL, NULL},
	{"6", "language", menu_sim_get_language, NULL, NULL},
	{"7", "update language", menu_sim_set_language, NULL, NULL},
	{"8", "cf info", menu_sim_get_cf, NULL, NULL},
	{"9", "mw info", menu_sim_get_mw, NULL, NULL},
	{"10", "update mw info", menu_sim_set_mw, NULL, NULL},
	{"11", "mailbox info", menu_sim_get_mb, NULL, NULL},
	{"12", "update mailbox info", menu_sim_set_mb, NULL, NULL},
	{"13", "msisdn", menu_sim_get_msisdn, NULL, NULL},
	{"14", "spn", menu_sim_get_spn, NULL, NULL},
	{"15", "cphs operator name info", menu_sim_get_cphs_netname, NULL, NULL},
	{"16", "authentication", menu_sim_req_authentication, NULL, NULL},
	{"17", "verify pins", menu_sim_verify_pins, NULL, NULL},
	{"18", "verify puks", menu_sim_verify_puks, NULL, NULL},
	{"19", "change pins", menu_sim_change_pins, NULL, NULL},
	{"20", "disable facility", menu_sim_disable_facility, NULL, NULL},
	{"21", "enable facility", menu_sim_enable_facility, NULL, NULL},
	{"22", "facility(en/dis status) info", menu_sim_get_facility, NULL, NULL},
	{"23", "lock status(status,retry count) info", menu_sim_get_lock_info, NULL, NULL},
	{"24", "apdu", menu_sim_transmit_apdu, NULL, NULL},
	{"25", "atr info", menu_sim_get_atr, NULL, NULL},
	{ NULL, NULL, },
};

void register_sim_event(TapiHandle *handle)
{
	int ret;

	ret = tel_register_noti_event(handle, TAPI_NOTI_SIM_STATUS, on_noti_sim_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_SIM_STATUS - event register failed [%d]", ret);
}
