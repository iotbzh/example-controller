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

#define		EVENT_GENERATOR_BINDING_API			"event-generator"
#define		EVENT_GENERATOR_BINDING_INFO			"A minimal binding used to send events using verb"
#define		EVENT_GENERATOR_BINDING_EVENT_FOR_C_PLUGIN	"generated-event-for-c-plugin"
#define		EVENT_GENERATOR_BINDING_EVENT_FOR_LUA_PLUGIN	"generated-event-for-lua-plugin"

enum SubscribeUnsubscribeType {
	SUBSCRIPTION = 0,
	UNSUBSCRIPTION = 1
};

struct subscriptionList {
	afb_event_t eventForCPlugin;
	afb_event_t eventForLuaPlugin;
};

/*******************************************************************************
 *	Event handler							       *
 ******************************************************************************/

void handleEvents(afb_api_t apiHandle, const char *evtLabel, json_object *eventJ)
{
	AFB_API_ERROR(apiHandle,
		      "Event received, api is not supposed to receive any, event label '%s', event json '%s'",
		      evtLabel,
		      json_object_get_string(eventJ));
}

/*******************************************************************************
 *	Static verbs							       *
 ******************************************************************************/

void ping(afb_req_t request);
void sendEvents(afb_req_t request);
void subscribe(afb_req_t request);
void unsubscribe(afb_req_t request);

static afb_verb_t staticVerbs[] =
{
	/* VERB'S NAME			FUNCTION TO CALL			SHORT DESCRIPTION */
	{ .verb = "ping",		.callback = ping,			.info = "Ping for test"},
	{ .verb = "send-events",	.callback = sendEvents,			.info = "Send event"},
	{ .verb = "subscribe",		.callback = subscribe,			.info = "Subscribe to event"},
	{ .verb = "unsubscribe",	.callback = unsubscribe,		.info = "Unsubscribe from event"},
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

void sendEvents(afb_req_t request)
{
	int ret;

	afb_api_t apiHandle;

	struct subscriptionList *subList;

	AFB_REQ_INFO(request, "Received request to generate events ...");

	if(! request) {
		AFB_REQ_ERROR(request, "Invalid afb request");
		afb_req_fail_f(request, "invalid_request", "Invalid afb request");
		return;
	}

	apiHandle = afb_req_get_api(request);
	if(! apiHandle) {
		AFB_REQ_ERROR(request, "Invalid afb api handle");
		afb_req_fail_f(request, "invalid_api_handle", "Invalid afb api handle");
		return;
	}

	subList = (struct subscriptionList *) afb_api_get_userdata(apiHandle);
	if(! subList) {
		AFB_REQ_ERROR(request, "Invalid afb binding user data");
		afb_req_fail_f(request, "invalid_user_data", "Invalid afb binding user data");
		return;
	}

	ret = afb_event_push(subList->eventForCPlugin, NULL);
	if(ret < 0) {
		AFB_REQ_ERROR(request,
			      "An error happened when tried to push a %s event",
			      EVENT_GENERATOR_BINDING_EVENT_FOR_C_PLUGIN);
		afb_req_fail_f(request,
			       "afb_push_event",
			       "An error happened when tried to push a %s event",
			       EVENT_GENERATOR_BINDING_EVENT_FOR_C_PLUGIN);
		return;
	}

	AFB_REQ_INFO(request,
		     "Event %s pushed to %s",
		     EVENT_GENERATOR_BINDING_EVENT_FOR_C_PLUGIN,
		     ret ? "at least one api" : "no api");

	ret = afb_event_push(subList->eventForLuaPlugin, NULL);
	if(ret < 0) {
		AFB_REQ_ERROR(request,
			      "An error happened when tried to push a %s event",
			      EVENT_GENERATOR_BINDING_EVENT_FOR_LUA_PLUGIN);
		afb_req_fail_f(request,
			       "afb_push_event",
			       "An error happened when tried to push a %s event",
			       EVENT_GENERATOR_BINDING_EVENT_FOR_LUA_PLUGIN);
		return;
	}

	AFB_REQ_INFO(request,
		     "Event %s pushed to %s",
		     EVENT_GENERATOR_BINDING_EVENT_FOR_LUA_PLUGIN,
		     ret ? "at least one api" : "no api");

	AFB_REQ_INFO(request, "Events generated correctly");

	afb_req_success_f(request, NULL, "Events generated correctly");
}

void subscribeUnsubscribe(afb_req_t request, enum SubscribeUnsubscribeType subscribeUnsubscribe)
{
	int ret;
	
	afb_api_t apiHandle;

	struct subscriptionList *subList;

	AFB_REQ_INFO(request,
		     "Received request to %s to internal events ...",
		     (subscribeUnsubscribe == SUBSCRIPTION) ? "subscribe" : "unsubscribe");

	if(! request) {
		AFB_REQ_ERROR(request, "Invalid afb request");
		afb_req_fail_f(request, "invalid_request", "Invalid afb request");
		return;
	}

	apiHandle = afb_req_get_api(request);
	if(! apiHandle) {
		AFB_REQ_ERROR(request, "Invalid afb api handle");
		afb_req_fail_f(request, "invalid_api_handle", "Invalid afb api handle");
		return;
	}

	subList = (struct subscriptionList *) afb_api_get_userdata(apiHandle);
	if(! subList) {
		AFB_REQ_ERROR(request, "Invalid afb binding user data");
		afb_req_fail_f(request, "invalid_user_data", "Invalid afb binding user data");
		return;
	}

	if(subscribeUnsubscribe == SUBSCRIPTION)
		ret = afb_req_subscribe(request, subList->eventForCPlugin);
	else
		ret = afb_req_unsubscribe(request, subList->eventForCPlugin);
	if(ret) {
		AFB_REQ_ERROR(request,
			      "An error happened when tried to %s to %s event",
			      (subscribeUnsubscribe == SUBSCRIPTION) ? "subscribe" : "unsubscribe",
			      EVENT_GENERATOR_BINDING_EVENT_FOR_C_PLUGIN);
		afb_req_fail_f(request,
			       "afb_subscription_error",
			       "An error happened when tried to %s to %s event",
			       (subscribeUnsubscribe == SUBSCRIPTION) ? "subscribe" : "unsubscribe",
			       EVENT_GENERATOR_BINDING_EVENT_FOR_C_PLUGIN);
		return;
	}

	if(subscribeUnsubscribe == SUBSCRIPTION)
		ret = afb_req_subscribe(request, subList->eventForLuaPlugin);
	else
		ret = afb_req_unsubscribe(request, subList->eventForLuaPlugin);
	if(ret) {
		AFB_REQ_ERROR(request,
			      "An error happened when tried to %s to %s event",
			      (subscribeUnsubscribe == SUBSCRIPTION) ? "subscribe" : "unsubscribe",
			      EVENT_GENERATOR_BINDING_EVENT_FOR_LUA_PLUGIN);
		afb_req_fail_f(request,
			       "afb_subscription_error",
			       "An error happened when tried to %s to %s event",
			       (subscribeUnsubscribe == SUBSCRIPTION) ? "subscribe" : "unsubscribe",
			       EVENT_GENERATOR_BINDING_EVENT_FOR_LUA_PLUGIN);
		return;
	}

	AFB_REQ_INFO(request,
		     "%s done correctly",
		     (subscribeUnsubscribe == SUBSCRIPTION) ? "Subscription" : "Unsubscription");

	afb_req_success_f(request,
			  NULL,
			  "%s done correctly",
			  (subscribeUnsubscribe == SUBSCRIPTION) ? "Subscription" : "Unsubscription");
}

void subscribe(afb_req_t request)
{
	subscribeUnsubscribe(request, SUBSCRIPTION);
}

void unsubscribe(afb_req_t request)
{
	subscribeUnsubscribe(request, UNSUBSCRIPTION);
}

/*******************************************************************************
 *	Binding init							       *
 ******************************************************************************/

static int eventGeneratorBindingInit(afb_api_t apiHandle)
{
	struct subscriptionList *subList;

	AFB_API_INFO(apiHandle, "Example controller binding initialization begining ...");

	if(! apiHandle)
		return -1;

	subList = malloc(sizeof(struct subscriptionList));
	if(! subList)
		return -2;

	subList->eventForCPlugin = afb_api_make_event(apiHandle, EVENT_GENERATOR_BINDING_EVENT_FOR_C_PLUGIN);
	if(! subList->eventForCPlugin)
		return -3;

	subList->eventForLuaPlugin = afb_api_make_event(apiHandle, EVENT_GENERATOR_BINDING_EVENT_FOR_LUA_PLUGIN);
	if(! subList->eventForLuaPlugin)
		return -4;

	afb_api_set_userdata(apiHandle, (void *) subList);

	AFB_API_INFO(apiHandle, "Example controller binding initialization executed correctly");

	return 0;
}

static int eventGeneratorBindingPreInit(void *cbdata, afb_api_t apiHandle)
{
	AFB_API_INFO(apiHandle, "Example controller binding pre-initialization begining ...");

	if(! apiHandle)
		return -1;

	if(afb_api_set_verbs_v3(apiHandle, staticVerbs)) {
		AFB_API_ERROR(apiHandle, "Load Section : fail to register static verbs");
		return -2;
	}

	afb_api_on_event(apiHandle, handleEvents);

	afb_api_on_init(apiHandle, eventGeneratorBindingInit);

	AFB_API_INFO(apiHandle, "Example controller binding pre-initialization executed correctly");

	return 0;
}

int afbBindingEntry(afb_api_t apiHandle)
{
	AFB_API_INFO(apiHandle, "Example controller binding entry function begining ...");

	afb_api_new_api(apiHandle, EVENT_GENERATOR_BINDING_API, EVENT_GENERATOR_BINDING_INFO, 1, eventGeneratorBindingPreInit, NULL);

	AFB_API_INFO(apiHandle, "Event generator binding entry function executed correctly");

	return 0;
}