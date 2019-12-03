/*
 * Copyright (C) 2018 "IoT.bzh"
 * Author Jonathan Aillet <jonathan.aillet@iot.bzh>
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
#include <string.h>

#include <filescan-utils.h>
#include <wrap-json.h>

#include <afb/afb-binding.h>

#include <ctl-config.h>

#define		EVENT_GENERATOR_BINDING_API	"event-generator"

/*******************************************************************************
 *	Controller section handler table				       *
 ******************************************************************************/

static CtlSectionT ctrlSectionsDefault[] =
{
	{ .key = "plugins",		.loadCB = PluginConfig },
	{ .key = "onload",		.loadCB = OnloadConfig },
	{ .key = "controls",		.loadCB = ControlConfig },
	{ .key = "events",		.loadCB = EventConfig },
	{ .key = NULL }
};

/*******************************************************************************
 *	Static verbs							       *
 ******************************************************************************/

void ping(afb_req_t request);

static afb_verb_t staticVerbs[] =
{
	/* VERB'S NAME			FUNCTION TO CALL			SHORT DESCRIPTION */
	{ .verb = "ping",		.callback = ping,			.info = "Ping for test"},
	{ .verb = NULL }		// Marker for end of the array
};

void ping(afb_req_t request)
{
	static int count = 0;
	int c;

	json_object *countJ;

	c = __sync_add_and_fetch(&count, 1, __ATOMIC_RELAXED);

	AFB_REQ_INFO(request, "ping count = %d", c);

	countJ = json_object_new_int(c);
	if(! countJ) {
		afb_req_fail(request, "count_json_int", "Didn't succeed to allocate count json integer");
		return;
	}

	afb_req_success(request, countJ, "Ping count");
}

/*******************************************************************************
 *	Binding init							       *
 ******************************************************************************/

static int exampleControllerBindingInit(afb_api_t apiHandle)
{
	int ret;

	char *error = NULL, *info = NULL;

	json_object *returnedJ = NULL;

	CtlConfigT *ctrlConfig;

	AFB_API_INFO(apiHandle, "Example controller binding initialization begining ...");

	if(! apiHandle)
		return -1;

	ctrlConfig = (CtlConfigT *) afb_api_get_userdata(apiHandle);
	if(! ctrlConfig)
		return -2;

	ret = CtlConfigExec(apiHandle, ctrlConfig);
	if(ret < 0) {
		AFB_API_ERROR(apiHandle, "Error %i caught when trying to apply current internal hal controller sections", ret);
		return -3;
	}
	else if (ret > 0) {
		AFB_API_WARNING(apiHandle, "Warning %i raised when trying to apply current internal hal controller sections", ret);
	}

	ret = afb_api_call_sync(apiHandle, EVENT_GENERATOR_BINDING_API, "subscribe", NULL, &returnedJ, &error, &info);
	if(ret || error) {
		AFB_API_ERROR(apiHandle,
			      "Error %i caught when trying to subscribe to '%s' verb of '%s' api (with error='%s', info='%s', and json='%s')",
			      ret,
			      "subscribe",
			      EVENT_GENERATOR_BINDING_API,
			      error ? error : "none",
			      info ? info : "none",
			      returnedJ ? json_object_get_string(returnedJ) : "none");
		return -4;
	}

	AFB_API_INFO(apiHandle,
		     "'%s' call to '%s' api succeed (with info='%s' and json='%s')",
		     "subscribe",
		     EVENT_GENERATOR_BINDING_API,
		     info ? info : "none",
		     returnedJ ? json_object_get_string(returnedJ) : "none");

	AFB_API_INFO(apiHandle, "Example controller binding initialization executed correctly");

	return 0;
}

static int exampleControllerBindingPreInit(void *cbdata, afb_api_t apiHandle)
{
	int ret;
	CtlConfigT *ctrlConfig;

	AFB_API_INFO(apiHandle, "Example controller binding pre-initialization begining ...");

	if(! cbdata || ! apiHandle)
		return -1;

	ctrlConfig = (CtlConfigT*) cbdata;

	afb_api_set_userdata(apiHandle, ctrlConfig);

	if(afb_api_set_verbs_v3(apiHandle, staticVerbs)) {
		AFB_API_ERROR(apiHandle, "Load Section : fail to register static verbs");
		return -2;
	}

	ret = CtlLoadSections(apiHandle, ctrlConfig, ctrlSectionsDefault);
	if(ret < 0) {
		AFB_API_ERROR(apiHandle, "Error %i caught when trying to load controller sections", ret);
		return -4;
	}
	else if (ret > 0) {
		AFB_API_WARNING(apiHandle, "Warning %i raised when trying to load controller sections", ret);
	}

	afb_api_on_event(apiHandle, CtrlDispatchApiEvent);

	afb_api_on_init(apiHandle, exampleControllerBindingInit);

	AFB_API_INFO(apiHandle, "Example controller binding pre-initialization executed correctly");

	return 0;
}

int afbBindingEntry(afb_api_t apiHandle)
{
	char *dirList, *configPath;

	CtlConfigT *ctrlConfig;

	AFB_API_INFO(apiHandle, "Example controller binding entry function begining ...");

	dirList = getenv("CONTROL_CONFIG_PATH");
	if(! dirList)
		dirList = CONTROL_CONFIG_PATH;

	configPath = CtlConfigSearch(apiHandle, dirList, "example");
	if(! configPath) {
		AFB_API_ERROR(apiHandle, "No example-(binder-middle-name)*.json config file(s) found in %s, ", dirList);
		return -1;
	}

	ctrlConfig = CtlLoadMetaData (apiHandle, configPath);
	if(! ctrlConfig) {
		AFB_API_ERROR(apiHandle, "An error happened when tried to parse %s json file using controller function", configPath);
		return -2;
	}

	if(! ctrlConfig->api) {
		AFB_API_ERROR(apiHandle, "An error happened when tried to parse %s json file using controller function", configPath);
		return -3;
	}

	afb_api_new_api(apiHandle, ctrlConfig->api, ctrlConfig->info, 1, exampleControllerBindingPreInit, ctrlConfig);

	AFB_API_INFO(apiHandle, "Example controller binding entry function executed correctly");

	return 0;
}