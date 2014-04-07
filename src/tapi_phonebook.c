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
#include "ITapiPhonebook.h"

#include <tapi_phonebook.h>
#include <tel_return.h>

static TelSimPbAccessResult_t __map_pb_result(int result)
{
	switch(result) {
		case TEL_PB_RESULT_SUCCESS :
			return TAPI_SIM_PB_SUCCESS;
		case TEL_PB_RESULT_INVALID_INDEX :
			return TAPI_SIM_PB_INVALID_INDEX;
		case TEL_PB_RESULT_ACCESS_RESTRICTED :
			return TAPI_SIM_PB_ACCESS_CONDITION_NOT_SATISFIED;
		default :
			return TAPI_SIM_PB_FAIL;
	}
}

static void on_response_get_sim_pb_count(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimPbAccessResult_t pb_result = __map_pb_result(result);

	if (pb_result == TAPI_SIM_PB_SUCCESS) {
		TelPbInfo *pb_cnt_buf = data;
		TelSimPbStorageInfo_t pb_cnt = {0, };

		pb_cnt.StorageFileType = pb_cnt_buf->pb_type;
		if (pb_cnt_buf->pb_type == TEL_PB_USIM) {
			pb_cnt.TotalRecordCount = (unsigned short)pb_cnt_buf->info_u.usim.max_count;
			pb_cnt.UsedRecordCount = (unsigned short)pb_cnt_buf->info_u.usim.used_count;
		} else {
			pb_cnt.TotalRecordCount = (unsigned short)pb_cnt_buf->info_u.sim.max_count;
			pb_cnt.UsedRecordCount = (unsigned short)pb_cnt_buf->info_u.sim.used_count;
		}
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, pb_result, &pb_cnt, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, pb_result, NULL, evt_cb_data->user_data);
		}
	}
	g_free(evt_cb_data);
}

static void on_response_get_sim_pb_meta_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimPbAccessResult_t pb_result = __map_pb_result(result);

	if (pb_result == TAPI_SIM_PB_SUCCESS) {
		TelSimPbEntryInfo_t pb_info = {0, };
		TelPbInfo *pb_info_buf = data;

		pb_info.StorageFileType = pb_info_buf->pb_type;
		pb_info.PbTextLenMax = (unsigned short)pb_info_buf->info_u.sim.max_text_len;
		pb_info.PbNumLenMax = (unsigned short)pb_info_buf->info_u.sim.max_num_len;
		pb_info.PbIndexMin = 1;
		pb_info.PbIndexMax = (unsigned short)pb_info_buf->info_u.sim.max_count;

		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, pb_result, &pb_info, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, pb_result, NULL, evt_cb_data->user_data);
		}
	}
	g_free(evt_cb_data);
}

static void on_response_get_sim_pb_usim_meta_info(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimPbAccessResult_t pb_result = __map_pb_result(result);

	if (pb_result == TAPI_SIM_PB_SUCCESS) {
		TelSimPbCapabilityInfo_t usim_pb_info = {0, };
		TelPbInfo *usim_pb_info_buf = data;
		unsigned short i = 0, j = 0;
		unsigned int pb_anr[3] = {
			TAPI_PB_3G_ANR1,
			TAPI_PB_3G_ANR2,
			TAPI_PB_3G_ANR3
		};
		unsigned int pb_email[4] = {
			TAPI_PB_3G_EMAIL1,
			TAPI_PB_3G_EMAIL2,
			TAPI_PB_3G_EMAIL3,
			TAPI_PB_3G_EMAIL4
		};

		/*NAME*/
		usim_pb_info.FileTypeInfo[j].field_type = TAPI_PB_3G_NAME;
		usim_pb_info.FileTypeInfo[j].used_count = (unsigned short)usim_pb_info_buf->info_u.usim.used_count;
		usim_pb_info.FileTypeInfo[j].text_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_text_len;
		usim_pb_info.FileTypeInfo[j].index_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_count;
		j++;

		/*NUMBER*/
		usim_pb_info.FileTypeInfo[j].field_type = TAPI_PB_3G_NUMBER;
		usim_pb_info.FileTypeInfo[j].used_count = (unsigned short)usim_pb_info_buf->info_u.usim.used_count;
		usim_pb_info.FileTypeInfo[j].text_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_num_len;
		usim_pb_info.FileTypeInfo[j].index_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_count;
		j++;

		/*SNE*/
		usim_pb_info.FileTypeInfo[j].field_type = TAPI_PB_3G_SNE;
		usim_pb_info.FileTypeInfo[j].text_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_sne_len;
		usim_pb_info.FileTypeInfo[j].index_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_count;
		j++;

		/*ANR*/
		for (i = 0; i < usim_pb_info_buf->info_u.usim.max_anr_count; i++) {
			usim_pb_info.FileTypeInfo[j].field_type = pb_anr[i];
			usim_pb_info.FileTypeInfo[j].text_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_anr_len;
			usim_pb_info.FileTypeInfo[j].index_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_count;
			j++;
		}

		/*EMAIL*/
		for (i = 0; i < usim_pb_info_buf->info_u.usim.max_email_count; i++) {
			usim_pb_info.FileTypeInfo[j].field_type = pb_email[i];
			usim_pb_info.FileTypeInfo[j].text_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_email_len;
			usim_pb_info.FileTypeInfo[j].index_max = (unsigned short)usim_pb_info_buf->info_u.usim.max_count;
			j++;
		}
		usim_pb_info.FileTypeCount = j;

		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, pb_result, &usim_pb_info, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, pb_result, NULL, evt_cb_data->user_data);
		}
	}
	g_free(evt_cb_data);
}

static void on_response_read_sim_pb_record(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;
	TelSimPbAccessResult_t pb_result = __map_pb_result(result);

	if (pb_result == TAPI_SIM_PB_SUCCESS) {
		TelPbReadRecord *pb_rec_buf = data;
		TelSimPbRecord_t pb_rec = {0,};
		unsigned int i;

		pb_rec.index = (unsigned short)pb_rec_buf->index;
		pb_rec.next_index = (unsigned short)pb_rec_buf->next_index;
		pb_rec.phonebook_type = pb_rec_buf->pb_type;
		pb_rec.dcs = TAPI_SIM_TEXT_ENC_ASCII;

		if (pb_rec_buf->pb_type == TEL_PB_USIM) {
			g_strlcpy((gchar *)pb_rec.name, pb_rec_buf->rec_u.usim.name, TEL_PB_TEXT_MAX_LEN+1);
			g_strlcpy((gchar *)pb_rec.number, pb_rec_buf->rec_u.usim.number, TEL_PB_NUMBER_MAX_LEN+1);
			g_strlcpy((gchar *)pb_rec.sne, pb_rec_buf->rec_u.usim.sne, TEL_PB_TEXT_MAX_LEN+1);
			for(i = 0; i < pb_rec_buf->rec_u.usim.anr_count; i++) {
				switch (i) {
				case 0 :
					g_strlcpy((gchar *)pb_rec.anr1, pb_rec_buf->rec_u.usim.anr[i].number, TEL_PB_NUMBER_MAX_LEN+1);
					if (pb_rec.anr1[0] == '+')
						pb_rec.anr1_ton = TAPI_SIM_TON_INTERNATIONAL;
					else
						pb_rec.anr1_ton = TAPI_SIM_TON_UNKNOWN;
					break;
				case 1 :
					g_strlcpy((gchar *)pb_rec.anr2, pb_rec_buf->rec_u.usim.anr[i].number, TEL_PB_NUMBER_MAX_LEN+1);
					if (pb_rec.anr2[0] == '+')
						pb_rec.anr2_ton = TAPI_SIM_TON_INTERNATIONAL;
					else
						pb_rec.anr2_ton = TAPI_SIM_TON_UNKNOWN;
					break;
				case 2 :
					g_strlcpy((gchar *)pb_rec.anr3, pb_rec_buf->rec_u.usim.anr[i].number, TEL_PB_NUMBER_MAX_LEN+1);
					if (pb_rec.anr3[0] == '+')
						pb_rec.anr3_ton = TAPI_SIM_TON_INTERNATIONAL;
					else
						pb_rec.anr3_ton = TAPI_SIM_TON_UNKNOWN;
					break;
				}
			}
			for(i = 0; i < pb_rec_buf->rec_u.usim.email_count; i++) {
				switch (i) {
				case 0 :
					g_strlcpy((gchar *)pb_rec.email1, pb_rec_buf->rec_u.usim.email[i], TEL_PB_TEXT_MAX_LEN+1);
					break;
				case 1 :
					g_strlcpy((gchar *)pb_rec.email2, pb_rec_buf->rec_u.usim.email[i], TEL_PB_TEXT_MAX_LEN+1);
					break;
				case 2 :
					g_strlcpy((gchar *)pb_rec.email3, pb_rec_buf->rec_u.usim.email[i], TEL_PB_TEXT_MAX_LEN+1);
					break;
				case 3 :
					g_strlcpy((gchar *)pb_rec.email4, pb_rec_buf->rec_u.usim.email[i], TEL_PB_TEXT_MAX_LEN+1);
					break;
				}
			}
		} else {
			g_strlcpy((gchar *)pb_rec.name, pb_rec_buf->rec_u.sim.name, TEL_PB_TEXT_MAX_LEN+1);
			g_strlcpy((gchar *)pb_rec.number, pb_rec_buf->rec_u.sim.number, TEL_PB_NUMBER_MAX_LEN+1);
		}
		if (pb_rec.number[0] == '+')
			pb_rec.ton = TAPI_SIM_TON_INTERNATIONAL;
		else
			pb_rec.ton = TAPI_SIM_TON_UNKNOWN;
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, pb_result, &pb_rec, evt_cb_data->user_data);
		}
	} else {
		if (evt_cb_data->cb) {
			evt_cb_data->cb((TapiHandle *)handle, pb_result, NULL, evt_cb_data->user_data);
		}
	}
	g_free(evt_cb_data);
}

static void on_response_update_sim_pb_record(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb(handle, __map_pb_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

static void on_response_delete_sim_pb_record(TelHandle *handle, int result, void *data, void *user_data)
{
	struct user_data_buf *evt_cb_data = user_data;

	if (evt_cb_data->cb) {
		evt_cb_data->cb(handle, __map_pb_result(result), NULL, evt_cb_data->user_data);
	}
	g_free(evt_cb_data);
}

EXPORT_API int tel_get_sim_pb_init_info(TapiHandle *handle, int *init_completed, TelSimPbList_t *pb_list)
{
	TelPbList pb_list_buf = {0,};
	TelReturn ret;

	dbg("Func Entrance");

	ret = tapi_pb_get_sim_pb_init_info((TelHandle *)handle, (gboolean *)init_completed, &pb_list_buf);

	if (ret == TEL_RETURN_SUCCESS) {
		pb_list->b_adn = pb_list_buf.adn;
		pb_list->b_fdn = pb_list_buf.fdn;
		pb_list->b_sdn = pb_list_buf.sdn;
		pb_list->b_3g = pb_list_buf.usim;
		pb_list->b_aas = 0;
		pb_list->b_gas = 0;
	}
	CHECK_RETURN(ret, NULL);
}

EXPORT_API int tel_get_sim_pb_count(TapiHandle *handle, TelSimPbType_t pb_type, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_pb_get_sim_pb_info((TelHandle *)handle, (TelPbType)pb_type, on_response_get_sim_pb_count, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sim_pb_meta_info(TapiHandle *handle, TelSimPbType_t pb_type, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	if (pb_type >= TAPI_SIM_PB_3GSIM) {
		err("Invalid input");
		return TAPI_API_INVALID_INPUT;
	}

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_pb_get_sim_pb_info((TelHandle *)handle, (TelPbType)pb_type, on_response_get_sim_pb_meta_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_get_sim_pb_usim_meta_info(TapiHandle *handle, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_pb_get_sim_pb_info((TelHandle *)handle, TEL_PB_USIM, on_response_get_sim_pb_usim_meta_info, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_read_sim_pb_record(TapiHandle *handle, TelSimPbType_t pb_type, unsigned short index, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelPbRecordInfo record = {0, };

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	record.index = index;
	record.pb_type = pb_type;

	ret = tapi_pb_read_sim_pb_record((TelHandle *)handle, &record, on_response_read_sim_pb_record, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_update_sim_pb_record(TapiHandle *handle, const TelSimPbRecord_t *req_data, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelPbUpdateRecord record = {0, };
	unsigned char i = 0;

	dbg("Func Entrance");

	if (!req_data) {
		err("request data is null");
		return TAPI_API_INVALID_INPUT;
	}

	record.index = req_data->index;
	record.pb_type = req_data->phonebook_type;
	if (record.pb_type == TEL_PB_USIM) {
		g_strlcpy(record.rec_u.usim.name, (const gchar *)req_data->name, TEL_PB_TEXT_MAX_LEN+1);
		g_strlcpy(record.rec_u.usim.number, (const gchar *)req_data->number, TEL_PB_NUMBER_MAX_LEN+1);
		g_strlcpy(record.rec_u.usim.sne, (const gchar *)req_data->sne, TEL_PB_TEXT_MAX_LEN+1);

		/*ANR*/
		if (strlen((const char *)req_data->anr1) > 0) {
			g_strlcpy(record.rec_u.usim.anr[i].number, (const gchar *)req_data->anr1, TEL_PB_NUMBER_MAX_LEN+1);
			i++;
		}
		if (strlen((const char *)req_data->anr2) > 0) {
			g_strlcpy(record.rec_u.usim.anr[i].number, (const gchar *)req_data->anr2, TEL_PB_NUMBER_MAX_LEN+1);
			i++;
		}
		if (strlen((const char *)req_data->anr3) > 0) {
			g_strlcpy(record.rec_u.usim.anr[i].number, (const gchar *)req_data->anr3, TEL_PB_NUMBER_MAX_LEN+1);
			i++;
		}
		record.rec_u.usim.anr_count = i;
		i = 0;

		/*EMAIL*/
		if (strlen((const char *)req_data->email1) > 0) {
			g_strlcpy(record.rec_u.usim.email[i], (const gchar *)req_data->email1, TEL_PB_TEXT_MAX_LEN+1);
			i++;
		}
		if (strlen((const char *)req_data->email2) > 0) {
			g_strlcpy(record.rec_u.usim.email[i], (const gchar *)req_data->email2, TEL_PB_TEXT_MAX_LEN+1);
			i++;
		}
		if (strlen((const char *)req_data->email3) > 0) {
			g_strlcpy(record.rec_u.usim.email[i], (const gchar *)req_data->email3, TEL_PB_TEXT_MAX_LEN+1);
			i++;
		}
		if (strlen((const char *)req_data->email4) > 0) {
			g_strlcpy(record.rec_u.usim.email[i], (const gchar *)req_data->email4, TEL_PB_TEXT_MAX_LEN+1);
			i++;
		}
		record.rec_u.usim.email_count = i;

	} else if (record.pb_type == TEL_PB_FDN || record.pb_type == TEL_PB_ADN || record.pb_type == TEL_PB_SDN) {
		g_strlcpy(record.rec_u.sim.name, (const gchar *)req_data->name, TEL_PB_TEXT_MAX_LEN+1);
		g_strlcpy(record.rec_u.sim.number, (const gchar *)req_data->number, TEL_PB_NUMBER_MAX_LEN+1);
	} else {
		err("unsupported phonebook type");
		return TAPI_API_INVALID_INPUT;
	}

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	ret = tapi_pb_update_sim_pb_record((TelHandle *)handle, &record, on_response_update_sim_pb_record, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}

EXPORT_API int tel_delete_sim_pb_record(TapiHandle *handle, TelSimPbType_t pb_type, unsigned short index, tapi_response_cb cb, void *user_data)
{
	TelReturn ret;
	struct user_data_buf *cb_data = NULL;
	TelPbRecordInfo record = {0, };

	dbg("Func Entrance");

	MAKE_RESP_CB_DATA_BUF(cb_data, cb, user_data, NULL, 0);

	record.index = index;
	record.pb_type = pb_type;

	ret = tapi_pb_delete_sim_pb_record((TelHandle *)handle, &record, on_response_delete_sim_pb_record, (void *)cb_data);

	CHECK_RETURN(ret, cb_data);
}
