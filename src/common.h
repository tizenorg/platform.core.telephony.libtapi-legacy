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

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

__BEGIN_DECLS

#define CHECK_DEINIT(error) \
	if (error) { \
		dbg("dbus error = %d (%s)", error->code, error->message); \
		if (error->code == G_IO_ERROR_CANCELLED \
				&& error->domain == G_IO_ERROR) { \
			g_error_free(error); \
			free(evt_cb_data); \
			return; \
		} \
	}

#define MAKE_RESP_CB_DATA_BUF(data, callback, usr_data, legacy_usr_data, legacy_usr_data_len) \
	data = g_new0(struct user_data_buf, 1); \
	data->cb = callback; \
	data->user_data = usr_data; \
	data->legacy_user_data = g_memdup(legacy_usr_data, legacy_usr_data_len)

#define CHECK_RETURN(ret, cb_data) \
	do { \
		int result; \
		switch(ret) { \
		case TEL_RETURN_SUCCESS : \
			result = TAPI_API_SUCCESS; \
			break; \
		case TEL_RETURN_INVALID_PARAMETER : \
			result = TAPI_API_INVALID_INPUT; \
			break; \
		case TEL_RETURN_MEMORY_FAILURE : \
			result = TAPI_API_SYSTEM_OUT_OF_MEM; \
			break; \
		case TEL_RETURN_OPERATION_NOT_SUPPORTED : \
			result = TAPI_API_NOT_SUPPORTED; \
			break; \
		default : \
			result = TAPI_API_OPERATION_FAILED; \
		} \
		if (result != TAPI_API_SUCCESS && cb_data != NULL) { \
			g_free(((struct user_data_buf *)cb_data)->legacy_user_data); \
			g_free(cb_data); \
		} \
		return result; \
	} while(0)

struct tapi_handle {
	void *data;
};

struct tapi_resp_data {
	tapi_response_cb cb_fn;
	void* user_data;
	struct tapi_handle *handle;
};

struct tapi_evt_cb {
	char *evt_id;
	tapi_notification_cb cb_fn;
	void* user_data;
	struct tapi_handle *handle;
};

struct user_data_buf {
	void *user_data;
	void *legacy_user_data;
	tapi_response_cb cb;
};

__END_DECLS

#endif /* _TAPI_COMMON_H_ */

/**
 *  @}
 */
