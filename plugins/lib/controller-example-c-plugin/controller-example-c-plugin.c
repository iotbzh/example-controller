/*
 * Copyright (C) 2018 "IoT.bzh"
 * Author : Thierry Bultel <thierry.bultel@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include <ctl-config.h>

#define CONTROLLER_EXAMPLE_C_PLUGIN_NAME "controller-example-c-plugin"

CTLP_CAPI_REGISTER(CONTROLLER_EXAMPLE_C_PLUGIN_NAME)

CTLP_ONLOAD(plugin, callbacks)
{
	AFB_API_INFO(plugin->api, "%s Plugin Registered correctly: uid='%s' 'info='%s'", CONTROLLER_EXAMPLE_C_PLUGIN_NAME, plugin->uid, plugin->info);
	return 0;
}

CTLP_INIT(plugin, callbacks)
{
	AFB_API_INFO(plugin->api, "%s Plugin initialized correctly: uid='%s' 'info='%s'", CONTROLLER_EXAMPLE_C_PLUGIN_NAME, plugin->uid, plugin->info);
	return 0;
}

CTLP_CAPI(onload_function, source, argsJ, queryJ)
{
	AFB_API_INFO(source->api, "%s plugin : %s function called correctly", CONTROLLER_EXAMPLE_C_PLUGIN_NAME, __func__);
	return 0;
}

CTLP_CAPI(control_function, source, argsJ, queryJ)
{
	AFB_API_INFO(source->api, "%s plugin : %s function called correctly", CONTROLLER_EXAMPLE_C_PLUGIN_NAME, __func__);
	afb_req_success_f(source->request, NULL, "%s plugin : %s function called correctly", CONTROLLER_EXAMPLE_C_PLUGIN_NAME, __func__);
	return 0;
}

CTLP_CAPI(event_function, source, argsJ, queryJ)
{
	AFB_API_INFO(source->api, "%s plugin : %s function called correctly", CONTROLLER_EXAMPLE_C_PLUGIN_NAME, __func__);
	return 0;
}