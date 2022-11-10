/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
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
#include <stdint.h>
#include "ccsp_trace.h"
#include "cosa_rbus_handler_apis.h"

#if  defined  (WAN_FAILOVER_SUPPORTED) ||  defined(RBUS_BUILD_FLAG_ENABLE) || defined (_HUB4_PRODUCT_REQ_) || defined (_PLATFORM_RASPBERRYPI_)
rbusHandle_t handle;
#endif

#if defined (WAN_FAILOVER_SUPPORTED)
EthAgent_Link_Status ethAgent_Link_Status;

unsigned int gSubscribersCount = 0;

/***********************************************************************

  Data Elements declaration:

 ***********************************************************************/
rbusDataElement_t ethAgentRbusDataElements[NUM_OF_RBUS_PARAMS] = {

    {ETHWAN_LINK_STATUS_TR181, RBUS_ELEMENT_TYPE_EVENT, {getBoolHandler, NULL, NULL, NULL, eventSubHandler, NULL}},

};

/***********************************************************************

  Get Handler APIs for objects of type RBUS_BOOL:

 ***********************************************************************/
rbusError_t getBoolHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t *opts)
{
	char const *name = rbusProperty_GetName(property);
	(void)handle;
	(void)opts;
	rbusValue_t value;
	rbusValue_Init(&value);

	if (strcmp(name, ETHWAN_LINK_STATUS_TR181) == 0)
	{
		CcspTraceWarning(("Getting EWAN link status value, new value = '%d'\n", ethAgent_Link_Status.EWanLinkStatus));
		rbusValue_SetBoolean(value, ethAgent_Link_Status.EWanLinkStatus);
	}
	else
	{
		CcspTraceWarning(("EthAgent rbus get handler invalid input\n"));
		return RBUS_ERROR_INVALID_INPUT;
	}
	rbusProperty_SetValue(property, value);
	rbusValue_Release(value);

	return RBUS_ERROR_SUCCESS;
}
/***********************************************************************

  Event subscribe handler API for objects:

 ***********************************************************************/
rbusError_t eventSubHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char *eventName, rbusFilter_t filter, int32_t interval, bool *autoPublish)
{
	(void)handle;
	(void)filter;
	(void)interval;

	*autoPublish = false;

	if (!strcmp(ETHWAN_LINK_STATUS_TR181, eventName))
	{
		if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
		{
			gSubscribersCount += 1;
		}
		else
		{
			if (gSubscribersCount > 0)
			{
				gSubscribersCount -= 1;
			}
		}
		CcspTraceWarning(("Subscribers count changed, new value=%d\n", gSubscribersCount));
	}
	else
	{
		CcspTraceWarning(("provider: eventSubHandler unexpected eventName %s\n", eventName));
	}
	return RBUS_ERROR_SUCCESS;
}

/*******************************************************************************

  initLinkStatus(): Initialize EthAgent_Link_Status struct with default values

 ********************************************************************************/
void initLinkStatus()
{
    ethAgent_Link_Status.EWanLinkStatus = false;
    CcspTraceWarning(("Initialized EWAN link status with default values.\n"));
}


/*******************************************************************************

  sendUpdateEvent(): publish event after event value gets updated

 ********************************************************************************/
rbusError_t sendBoolUpdateEvent(char* event_name , bool eventNewData, bool eventOldData)
{
	rbusEvent_t event;
	rbusObject_t data;
	rbusValue_t value;
	rbusValue_t oldVal;
	rbusValue_t byVal;
	rbusError_t ret = RBUS_ERROR_SUCCESS;
	
	//initialize and set previous value for the event
	rbusValue_Init(&oldVal);
	rbusValue_SetBoolean(oldVal, eventOldData);
	//initialize and set new value for the event
	rbusValue_Init(&value);
	rbusValue_SetBoolean(value, eventNewData);
	//initialize and set responsible component name for value change
	rbusValue_Init(&byVal);
	rbusValue_SetString(byVal, RBUS_COMPONENT_NAME);
	//initialize and set rbusObject with desired values
	rbusObject_Init(&data, NULL);
	rbusObject_SetValue(data, "value", value);
	rbusObject_SetValue(data, "oldValue", oldVal);
	rbusObject_SetValue(data, "by", byVal);
	//set data to be transferred
	event.name = event_name;
	event.data = data;
	event.type = RBUS_EVENT_VALUE_CHANGED;
	//publish the event
	ret = rbusEvent_Publish(handle, &event);
	if(ret != RBUS_ERROR_SUCCESS) {
			CcspTraceWarning(("rbusEvent_Publish for %s failed: %d\n", event_name, ret));
	}
	//release all initialized rbusValue objects
	rbusValue_Release(value);
	rbusValue_Release(oldVal);
	rbusValue_Release(byVal);
	rbusObject_Release(data);
	return ret;
}

/*******************************************************************************

  publishEWanLinkStatus(): publish EWanLinkStatus event after event value gets updated

 ********************************************************************************/
 
void publishEWanLinkStatus(bool link_status)
{
	rbusError_t ret = RBUS_ERROR_SUCCESS;
	bool oldEWanLinkStatus = ethAgent_Link_Status.EWanLinkStatus;
	//update EWanLinkStatus with new value
	ethAgent_Link_Status.EWanLinkStatus = link_status;
	CcspTraceInfo(("Publishing EWAN link status with updated value=%s\n", ethAgent_Link_Status.EWanLinkStatus ? "true" : "false"));
	if (gSubscribersCount > 0)
	{
		ret = sendBoolUpdateEvent(ETHWAN_LINK_STATUS_TR181, ethAgent_Link_Status.EWanLinkStatus, oldEWanLinkStatus);
		if(ret == RBUS_ERROR_SUCCESS) {
			CcspTraceInfo(("Published EWAN link status with updated value.\n"));
		}
		
	}
}
#endif //WAN_FAILOVER_SUPPORTED

#if defined (_HUB4_PRODUCT_REQ_) || defined (_PLATFORM_RASPBERRYPI_)
BOOL EthAgent_Rbus_discover_components(char const *pModuleList)
{
    rbusError_t rc = RBUS_ERROR_SUCCESS;
    int componentCnt = 0;
    char **pComponentNames;
    BOOL ret = FALSE;
    char ModuleList[1024] = {0};
    char const *rbusModuleList[7];
    int count = 0;
    const char delimit[2] = " ";
    char *token;

    strcpy(ModuleList,pModuleList);

    /* get the first token */
    token = strtok(ModuleList, delimit);

    /* walk through other tokens */
    while( token != NULL ) {
        printf( " %s\n", token );
        rbusModuleList[count]=token;
        count++;
        token = strtok(NULL, delimit);
    }

    for(int i=0; i<count;i++)
    {
        CcspTraceInfo(("EthAgent_Rbus_discover_components rbusModuleList[%s]\n", rbusModuleList[i]));
    }

    rc = rbus_discoverComponentName (handle, count, rbusModuleList, &componentCnt, &pComponentNames);

    if(RBUS_ERROR_SUCCESS != rc)
    {
        CcspTraceInfo(("Failed to discover components. Error Code = %d\n", rc));
        return ret;
    }

    for (int i = 0; i < componentCnt; i++)
    {
        free(pComponentNames[i]);
    }

    free(pComponentNames);

    if(componentCnt == count)
    {
        ret = TRUE;
    }

    CcspTraceInfo( ("EthAgent_Rbus_discover_components (%d-%d)ret[%s]\n",componentCnt,count,(ret)?"TRUE":"FALSE"));

    return ret;
}

#endif //_HUB4_PRODUCT_REQ_

#if  defined  (WAN_FAILOVER_SUPPORTED) ||  defined(RBUS_BUILD_FLAG_ENABLE) || defined (_HUB4_PRODUCT_REQ_) || defined(_PLATFORM_RASPBERRYPI_)
/***********************************************************************

  ethAgentRbusInit(): Initialize Rbus and data elements

 ***********************************************************************/
rbusError_t ethAgentRbusInit()
{
	int rc = RBUS_ERROR_SUCCESS;
	rc = rbus_open(&handle, RBUS_COMPONENT_NAME);
	if (rc != RBUS_ERROR_SUCCESS)
	{
		CcspTraceWarning(("EthAgent rbus initialization failed\n"));
		rc = RBUS_ERROR_NOT_INITIALIZED;
		return rc;
	}

#if  defined  (WAN_FAILOVER_SUPPORTED)
	// Register data elements
	rc = rbus_regDataElements(handle, NUM_OF_RBUS_PARAMS, ethAgentRbusDataElements);
#endif

	if (rc != RBUS_ERROR_SUCCESS)
	{
		CcspTraceWarning(("rbus register data elements failed\n"));
		rc = rbus_close(handle);
		return rc;
	}

#if  defined  (WAN_FAILOVER_SUPPORTED)	
	initLinkStatus();
#endif

	return rc;
}
#endif
