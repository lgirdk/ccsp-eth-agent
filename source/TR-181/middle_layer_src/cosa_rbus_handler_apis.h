/*
 * If not stated otherwise in this file or this component's Licenses.txt file
 * the following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#ifndef  RBUS_HANDLER_APIS_H
#define  RBUS_HANDLER_APIS_H

#if defined (WAN_FAILOVER_SUPPORTED) ||  defined(RBUS_BUILD_FLAG_ENABLE) || defined (_HUB4_PRODUCT_REQ_) || defined (_PLATFORM_RASPBERRYPI_)
#include <stdbool.h>
#include <rbus/rbus.h>

#define NUM_OF_RBUS_PARAMS	1
#define RBUS_COMPONENT_NAME	"RbusEthAgent"

#if defined (WAN_FAILOVER_SUPPORTED)
#define ETHWAN_LINK_STATUS_TR181	"Device.X_RDKCENTRAL-COM_EthernetWAN.LinkStatus"

typedef struct 
_EthAgent_Link_Status_
{
    bool EWanLinkStatus;
	
} EthAgent_Link_Status;

rbusError_t getBoolHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts);

rbusError_t eventSubHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish);

void initLinkStatus();

rbusError_t sendBoolUpdateEvent(char* event_name , bool eventNewData, bool eventOldData);

void publishEWanLinkStatus(bool link_status);

#endif //WAN_FAILOVER_SUPPORTED
rbusError_t ethAgentRbusInit();
#if defined (_HUB4_PRODUCT_REQ_) || defined (_PLATFORM_RASPBERRYPI_)
BOOL EthAgent_Rbus_discover_components(char const *pModuleList);
#endif //_HUB4_PRODUCT_REQ_
#endif // WAN_FAILOVER_SUPPORTED RBUS_BUILD_FLAG_ENABLE _HUB4_PRODUCT_REQ_
#endif
