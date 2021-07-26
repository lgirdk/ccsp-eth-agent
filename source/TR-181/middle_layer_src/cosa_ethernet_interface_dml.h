/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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

    module: cosa_ethernet_interface_dml.h

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


#ifndef  _COSA_ETHERNET_INTERFACE_DML_H
#define  _COSA_ETHERNET_INTERFACE_DML_H

/***********************************************************************

 APIs for Object:

    Ethernet.Interface.{i}.

    *  Interface_GetEntryCount
    *  Interface_GetEntry
    *  Interface_GetParamBoolValue
    *  Interface_GetParamIntValue
    *  Interface_GetParamUlongValue
    *  Interface_GetParamStringValue
    *  Interface_SetParamBoolValue
    *  Interface_SetParamIntValue
    *  Interface_SetParamUlongValue
    *  Interface_SetParamStringValue
    *  Interface_Validate
    *  Interface_Commit
    *  Interface_Rollback

***********************************************************************/
#include "cosa_apis.h"

ULONG Interface_GetEntryCount ( ANSC_HANDLE );

ANSC_HANDLE Interface_GetEntry ( ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber );

BOOL Interface_GetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool );

BOOL Interface_GetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int* pInt );

BOOL Interface_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* pUlong );

ULONG Interface_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize );

BOOL Interface_SetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue );

BOOL Interface_SetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int value );

BOOL Interface_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValuepUlong );

BOOL Interface_SetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* strValue );

BOOL Interface_Validate ( ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength );

ULONG Interface_Commit ( ANSC_HANDLE hInsContext );

ULONG Interface_Rollback ( ANSC_HANDLE hInsContext );

ULONG AssociatedDevice1_GetEntryCount ( ANSC_HANDLE );

ANSC_HANDLE AssociatedDevice1_GetEntry ( ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber );

ULONG AssociatedDevice1_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize );

BOOL AssociatedDevice1_IsUpdated ( ANSC_HANDLE );

ULONG AssociatedDevice1_Synchronize ( ANSC_HANDLE );

/***********************************************************************

 APIs for Object:

    Ethernet.Interface.{i}.Stats.

    *  Stats_GetParamBoolValue
    *  Stats_GetParamIntValue
    *  Stats_GetParamUlongValue
    *  Stats_GetParamStringValue

***********************************************************************/
BOOL Stats_GetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool );

BOOL Stats_GetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int* pInt );

BOOL Stats_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* pUlong );

ULONG Stats_GetParamStringValue ( ANSC_HANDLE hInsContext, char*  ParamName, char*  pValue, ULONG* pUlSize );

int _getMac ( char* ifName, char* mac );

#endif
