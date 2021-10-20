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

    module: cosa_ethernet_dml.h

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------


    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/14/2011    initial revision.

**************************************************************************/


#ifndef  _COSA_ETHERNET_DML_H
#define  _COSA_ETHERNET_DML_H

#include "cosa_apis.h"
#include "plugin_main_apis.h"
#include "cosa_ethernet_internal.h"

/***********************************************************************

 APIs for Object:

    Ethernet.

    *  Ethernet_GetParamStringValue
    *  Ethernet_SetParamStringValue

***********************************************************************/


BOOL
Ethernet_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
Ethernet_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );
/***********************************************************************
 APIs for Object:

    Device.Ethernet.X_RDKCENTRAL-COM_WAN.

    *  EthWan_GetParamBoolValue
    *  EthWan_SetParamBoolValue
    *  EthWan_GetParamUlongValue

***********************************************************************/
BOOL
EthWan_GetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
EthWan_GetParamUlongValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      pUlong
    );
#if !defined(AUTOWAN_ENABLE)
BOOL
EthWan_SetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );
#endif
/***********************************************************************
 APIs for Object:

    DeviceInfo.X_RDKCENTRAL-COM_xOpsDeviceMgmt.Logging.

    *  EthLogging_GetParamBoolValue
    *  EthLogging_GetParamUlongValue
    *  EthLogging_SetParamBoolValue
    *  EthLogging_SetParamUlongValue
    *  EthLogging_Validate
    *  EthLogging_Commit
    *  EthLogging_Rollback

***********************************************************************/
BOOL
EthLogging_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
EthLogging_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      pUlong
    );

ULONG
EthLogging_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
EthLogging_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
EthLogging_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValuepUlong
    );

BOOL
EthLogging_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
EthLogging_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
EthLogging_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

#if defined(AUTOWAN_ENABLE)
ULONG
EthernetWAN_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );
BOOL
EthernetWAN_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );
#endif
BOOL
EthernetWAN_GetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
EthernetWAN_SetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
AutowanFeatureSupport_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

#if defined(FEATURE_RDKB_WAN_AGENT)
/***********************************************************************

 APIs for Object:

#ifdef FEATURE_RDKB_WAN_AGENT
    Ethernet.X_RDKCENTRAL-COM_Interface.{i}
#elif FEATURE_RDKB_WAN_MANAGER
    Ethernet.X_RDK_Interface.{i}

    *  EthInterface_GetEntryCount
    *  EthInterface_GetEntry
    *  EthInterface_GetParamBoolValue
    *  EthInterface_SetParamBoolValue
    *  EthInterface_SetParamStringValue
    *  EthInterface_Validate
    *  EthInterface_Commit
    *  EthInterface_Rollback

***********************************************************************/
ULONG
EthInterface_GetEntryCount
    (
        ANSC_HANDLE
    );

ANSC_HANDLE
EthInterface_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    );

BOOL
EthInterface_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

ULONG
EthInterface_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
EthInterface_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
EthInterface_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

BOOL
EthInterface_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
EthInterface_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
EthInterface_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

BOOL
EthInterface_GetParamUlongValue
    (
    ANSC_HANDLE hInsContext,
    char *ParamName,
    ULONG *puLong
    );

BOOL
EthInterface_SetParamUlongValue
    (
    ANSC_HANDLE hInsContext,
    char *ParamName,
    ULONG uValue
    );

#elif defined(FEATURE_RDKB_WAN_MANAGER)
/***********************************************************************

 APIs for Object:

    Ethernet.X_RDK_Interface.{i}

    *  EthRdkInterface_GetEntryCount
    *  EthRdkInterface_GetEntry
    *  EthRdkInterface_GetParamBoolValue
    *  EthRdkInterface_SetParamBoolValue
    *  EthRdkInterface_SetParamStringValue
    *  EthRdkInterface_Validate
    *  EthRdkInterface_Commit
    *  EthRdkInterface_Rollback

***********************************************************************/
ULONG
EthRdkInterface_GetEntryCount
    (
        ANSC_HANDLE
    );

ANSC_HANDLE
EthRdkInterface_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    );

BOOL
EthRdkInterface_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

ULONG
EthRdkInterface_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
EthRdkInterface_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
EthRdkInterface_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );
ANSC_HANDLE
EthRdkInterface_AddEntry
    (
        ANSC_HANDLE hInsContext,
        ULONG* pInsNumber
    );

ULONG
EthRdkInterface_DelEntry
    (
        ANSC_HANDLE hInsContext,
        ANSC_HANDLE hInstance
    );

BOOL
EthRdkInterface_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
EthRdkInterface_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
EthRdkInterface_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

BOOL
EthRdkInterface_GetParamUlongValue
    (
    ANSC_HANDLE hInsContext,
    char *ParamName,
    ULONG *puLong
    );

BOOL
EthRdkInterface_SetParamUlongValue
    (
    ANSC_HANDLE hInsContext,
    char *ParamName,
    ULONG uValue
    );
#endif  // FEATURE_RDKB_WAN_MANAGER
#endif
