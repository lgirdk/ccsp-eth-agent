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

    module: cosa_ethernet_internal.h

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

#ifndef  _COSA_ETHERNET_INTERNAL_H
#define  _COSA_ETHERNET_INTERNAL_H

#include "cosa_apis.h"
#include "plugin_main_apis.h"
#include "cosa_ethernet_apis.h"
#include "poam_irepfo_interface.h"
#include "sys_definitions.h"
#include "ccsp_hal_ethsw.h"

#define PSM_ETHMANAGER_CFG_UPSTREAM      "dmsb.ethagent.if.%d.Upstream"
#define PSM_ETHMANAGER_CFG_ADDTOBRIDGE   "dmsb.ethagent.if.%d.AddToLanBridge"

#ifdef FEATURE_RDKB_WAN_MANAGER
#define DML_ETHIF_INIT(Eth)                           \
    (Eth)->Enable = FALSE;                            \
    (Eth)->Upstream = FALSE;                          \
    (Eth)->WanStatus = ETH_WAN_DOWN;                  \
    (Eth)->LinkStatus = ETH_LINK_STATUS_DOWN;         \
    (Eth)->WanValidated = FALSE;

#define COSA_DATAMODEL_ETHERNET_CLASS_CONTENT          \
    /* duplication of the base object class content */ \
    COSA_BASE_CONTENT                                  \
    COSA_DATAMODEL_ETH_WAN_AGENT EthWanCfg;            \
    COSA_DML_ETH_LOG_STATUS LogStatus;                 \
    UINT ulTotalNoofEthInterfaces;                 \
    SLIST_HEADER Q_EthList;                        \
    COSA_DML_ETH_PORT_FULL EthernetPortFullTable[MAXINSTANCE]; \
    ULONG ulPtNextInstanceNumber;                  \
    ULONG MaxInstanceNumber;
#else
#define COSA_DATAMODEL_ETHERNET_CLASS_CONTENT          \
     /* duplication of the base object class content */ \
     COSA_BASE_CONTENT                                  \
     COSA_DATAMODEL_ETH_WAN_AGENT EthWanCfg;            \
     COSA_DML_ETH_LOG_STATUS LogStatus;                 \
     COSA_DML_ETH_PORT_FULL EthernetPortFullTable[MAXINSTANCE]; \
     UINT ulTotalNoofEthInterfaces;                 \
     PCOSA_DML_ETH_PORT_CONFIG pEthLink;
#endif //FEATURE_RDKB_WAN_MANAGER

typedef  struct
_COSA_DATAMODEL_ETHERNET_CLASS_CONTENT
{
    COSA_DATAMODEL_ETHERNET_CLASS_CONTENT
}COSA_DATAMODEL_ETHERNET, *PCOSA_DATAMODEL_ETHERNET;

void CosaEthTelemetryxOpsLogSettingsSync();

/*
    Standard function declaration
*/
ANSC_HANDLE CosaEthInterfaceCreate ( VOID );

ANSC_STATUS CosaEthInterfaceInitialize ( ANSC_HANDLE hThisObject );

ANSC_STATUS CosaEthInterfaceRemove ( ANSC_HANDLE hThisObject );

ANSC_STATUS CosaEthPortGetAssocDevices ( UCHAR *mac, CHAR *maclist, int numMacAddr );

ANSC_HANDLE
CosaEthernetCreate
    (
        VOID
    );

ANSC_STATUS
CosaEthernetInitialize
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
CosaEthernetRemove
    (
        ANSC_HANDLE                 hThisObject
    );

#if defined (FEATURE_RDKB_WAN_MANAGER)
ANSC_STATUS InitEthIfaceEntry(ANSC_HANDLE hDml, PCOSA_DML_ETH_PORT_CONFIG pEntry);
#endif
#ifdef FEATURE_RDKB_WAN_UPSTREAM
ANSC_STATUS CosaDmlSetWanOEMode( PCOSA_DML_ETH_PORT_FULL phContext, PCOSA_DML_ETH_PORT_CONFIG pEthlink );
#endif
#endif
