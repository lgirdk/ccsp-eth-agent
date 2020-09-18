/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#ifndef _ETH_JSON_APIS_H
#define _ETH_JSON_APIS_H

#include "json_hal_client.h"

#define UP "up"
#define DOWN "down"
#define DISCONNECTED "disconnected"

typedef int (*ethsw_ethLinkEventCallback)(char* ifname, char* state);
void eth_hal_registerLinkEventCallback(ethsw_ethLinkEventCallback callback_proc);

/* eth_hal_init :  */
/**
* Description: Do what needed to intialize the Eth hal.
* Parameters : None
*
* @return The status of the operation.
* @retval RETURN_OK if successful.
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous.
* @sideeffect None.

*
* @note This function must not suspend and must not invoke any blocking system 
* calls. It should probably just send a message to a driver event handler task. 
*
*/
int eth_hal_init( void );

#endif
