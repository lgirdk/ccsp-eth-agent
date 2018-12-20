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

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

/**************************************************************************

    module: cosa_ethernet_apis.h

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------


    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/11/2011    initial revision.

**************************************************************************/


#ifndef  _COSA_ETHERNET_API_H
#define  _COSA_ETHERNET_API_H

#include "../middle_layer_src/cosa_apis.h"
#include "../middle_layer_src/plugin_main_apis.h"
#include "ccsp_hal_ethsw.h"

/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/

typedef struct _Eth_host_t
{
    UCHAR eth_macAddr[6];
    INT eth_port;
    BOOL eth_Active;
    struct _Eth_host_t* next;
}Eth_host_t;

typedef  struct
_COSA_DATAMODEL_ETH_WAN_AGENT_CONTENT
{
    BOOLEAN                Enable;
    ULONG             	    Port;
}
COSA_DATAMODEL_ETH_WAN_AGENT, *PCOSA_DATAMODEL_ETH_WAN_AGENT;

struct _COSA_DML_ETH_LOG_STATUS
{
    ULONG       Log_Period;
    BOOLEAN     Log_Enable;	
}_struct_pack_;

typedef struct _COSA_DML_ETH_LOG_STATUS COSA_DML_ETH_LOG_STATUS, *PCOSA_DML_ETH_LOG_STATUS;

void Ethernet_Hosts_Sync( void );
INT CosaDmlEth_AssociatedDevice_callback(eth_device_t *eth_dev);

#endif

