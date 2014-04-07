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
#include <gio/gio.h>

#include <tapi_common.h>
#include <TapiUtility.h>

#include "menu.h"
#include "network.h"
#include "sat.h"
#include "sms.h"
#include "sim.h"
#include "phonebook.h"
#include "modem.h"
#include "call.h"
#include "ss.h"
#include "gps.h"

/*
extern struct menu_data menu_gprs[];
extern struct menu_data menu_custom[];
extern struct menu_data menu_svcmode[];
*/

extern struct menu_data menu_modem[];
extern struct menu_data menu_sat[];
extern struct menu_data menu_net[];
extern struct menu_data menu_sms[];
extern struct menu_data menu_sim[];
extern struct menu_data menu_phonebook[];
extern struct menu_data menu_ss[];
extern struct menu_data menu_call[];
extern struct menu_data menu_gps[];

GDBusConnection *conn = NULL;
TapiHandle *handle = NULL;

static char data_modem_name[MENU_DATA_SIZE + 1] = "";
static char data_property_int[MENU_DATA_SIZE + 1] = "org.tizen.telephony.Modem:power";
static char data_property_string[MENU_DATA_SIZE + 1] = "";

static int get_modem_info(MManager *mm, struct menu_data *menu)
{
	char **cp_list = NULL;
	int i = 0;

	cp_list = tel_get_cp_name_list();
	if (!cp_list) {
		msg("failed.");
		return 0;
	}

	while (cp_list[i]) {
		msg("[%02d] strlen(%d), '%s'", i, strlen(cp_list[i]), cp_list[i]);
		i++;
	}

	free(cp_list);

	return 0;
}

static int init(MManager *mm, struct menu_data *menu)
{
	if (handle) {
		msg("already handle exist");
		return 0;
	}

	if (strlen(data_modem_name) < 1) {
		msg("call tel_init(NULL)");
		handle = tel_init(NULL);
	}
	else {
		msg("call tel_init(\"%s\")", data_modem_name);
		handle = tel_init(data_modem_name);
	}

	if (!handle)
		msg("handle is null");

	menu_manager_set_user_data(mm, handle);

	return 0;
}

static int deinit(MManager *mm, struct menu_data *menu)
{
	if (!handle) {
		msg("handle is null");
		return 0;
	}

	msg("call tel_deinit()");
	tel_deinit(handle);
	handle = NULL;

	menu_manager_set_user_data(mm, handle);

	return 0;
}

static int get_property_int(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result_int = -1;
	int ret;

	msg("call tel_get_property_int()");

	ret = tel_get_property_int(handle, data_property_int, &result_int);
	if (ret != TAPI_API_SUCCESS) {
		msg("failed. (return = %d)", ret);
	}

	msg(" - result = %d", result_int);

	return 0;
}

static struct menu_data menu_common[] = {
	{ "1", "tel_get_modem_info",		NULL,	get_modem_info,		NULL},
	{ "2", "tel_init",		NULL,	init,	NULL},
	{ "2s", " - modem_name", NULL, NULL, data_modem_name},
	{ "3", "tel_deinit",	NULL,	deinit,	NULL},
	{ "4", "tel_get_property_int",		NULL,	get_property_int,	NULL},
	{ "4s", " - property",	NULL,	NULL,	data_property_int},
	{ "5", "tel_get_property_string",		NULL,	get_property_int,	NULL},
	{ "5s", " - property",	NULL,	NULL,	data_property_string},
	{ NULL, NULL, },
};

static struct menu_data menu_main[] = {
	{ "1", "Common",	menu_common,	NULL,		NULL},
	{ "2", "Modem",		menu_modem,	NULL,		NULL},
	{ "3", "Network",	menu_net,	NULL,		NULL},
	{ "4", "SIM",		menu_sim,	NULL,		NULL},
	{ "5", "Phonebook",		menu_phonebook,	NULL,		NULL},
	{ "6", "SMS",		menu_sms,	NULL,		NULL},
	{ "7", "SS",		menu_ss,	NULL,		NULL},
	{ "8", "SAT",		menu_sat,	NULL,		NULL},
	{ "9", "Call",		menu_call,	NULL,		NULL},
	{ "a", "Gps",		menu_gps,	NULL,		NULL},
/*
	{ "9", "GPRS",		menu_gprs,	NULL,		NULL},
	{ "a", "SOUND",		menu_xxx,	NULL,		NULL},
	{ "b", "Custom",	menu_custom,NULL,		NULL},
	{ "c", "Service Mode",	menu_svcmode,NULL,		NULL},
*/
	{ NULL, NULL, },
};

int main(int arg, char **argv)
{
	GMainLoop *mainloop;
	GIOChannel *channel = g_io_channel_unix_new(STDIN_FILENO);
	MManager *manager;

	g_type_init();
	mainloop = g_main_loop_new(NULL, FALSE);

	msg("");
	msg("* Telephony Test application ");
	msg("* Build On: %s  %s", __DATE__, __TIME__);

	manager = menu_manager_new(menu_main, mainloop);

	if (strlen(data_modem_name) < 1)
		handle = tel_init(NULL);
	else
		handle = tel_init(data_modem_name);

	msg("");

	if (!handle) {
		msg("tel_init(%s) failed.", data_modem_name);
	}
	else {
		msg("tel_init(%s) success.", data_modem_name);
	}

	menu_manager_set_user_data(manager, handle);


	register_network_event(handle);
	register_modem_event(handle);
	register_sat_event(handle);
	register_sim_event(handle);
	register_phonebook_event(handle);
	register_sms_event(handle);
	register_call_event(handle);
	register_ss_event(handle);
	register_gps_event(handle);


	menu_manager_run(manager);

	g_io_add_watch(channel, (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL), on_menu_manager_keyboard, manager);

	g_main_loop_run(mainloop);

	msg("******* Bye bye *******");

	return 0;
}

