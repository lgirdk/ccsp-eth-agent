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

    module: cosa_ethernet_dml.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/17/2011    initial revision.

**************************************************************************/

//#include "dml_tr181_custom_cfg.h"
#include "cosa_ethernet_dml.h"

extern ANSC_HANDLE g_EthObject;

/***********************************************************************
 IMPORTANT NOTE:

 According to TR69 spec:
 On successful receipt of a SetParameterValues RPC, the CPE MUST apply 
 the changes to all of the specified Parameters atomically. That is, either 
 all of the value changes are applied together, or none of the changes are 
 applied at all. In the latter case, the CPE MUST return a fault response 
 indicating the reason for the failure to apply the changes. 
 
 The CPE MUST NOT apply any of the specified changes without applying all 
 of them.

 In order to set parameter values correctly, the back-end is required to
 hold the updated values until "Validate" and "Commit" are called. Only after
 all the "Validate" passed in different objects, the "Commit" will be called.
 Otherwise, "Rollback" will be called instead.

 The sequence in COSA Data Model will be:

 SetParamBoolValue/SetParamIntValue/SetParamUlongValue/SetParamStringValue
 -- Backup the updated values;

 if( Validate_XXX())
 {
     Commit_XXX();    -- Commit the update all together in the same object
 }
 else
 {
     Rollback_XXX();  -- Remove the update at backup;
 }
 
***********************************************************************/
/***********************************************************************

 APIs for Object:

    Ethernet.

    *  Ethernet_GetParamBoolValue
    *  Ethernet_SetParamBoolValue


***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Ethernet_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Ethernet_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    /* check the parameter name and return the corresponding value */
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_EthHost_Sync", TRUE))
    {
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Ethernet_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/

BOOL
Ethernet_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_EthHost_Sync", TRUE))
    {
        Ethernet_Hosts_Sync();
        return TRUE;
    }
    CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return FALSE;
}
                                       
/***********************************************************************
 APIs for Object:

    Device.Ethernet.X_RDKCENTRAL-COM_WAN.

    *  EthWan_GetParamBoolValue
    *  EthWan_SetParamBoolValue
    *  EthWan_GetParamUlongValue

***********************************************************************/

/**********************************************************************
    caller:     owner of this object

    prototype:
        BOOL
        EthWan_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:
        This function is called to retrieve Boolean parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.
**********************************************************************/
BOOL
EthWan_GetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    if ( AnscEqualString ( ParamName, "Enabled", TRUE ) )
    {
        *pBool =  pMyObject->EthWanCfg.Enable;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        BOOL
        EthWan_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.
**********************************************************************/
BOOL
EthWan_GetParamUlongValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{    
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    if ( AnscEqualString ( ParamName, "Port", TRUE ) )
    {
        *puLong =  pMyObject->EthWanCfg.Port;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:

        BOOL
        EthWan_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.
**********************************************************************/
BOOL
EthWan_SetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                         bValue
    )
{
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    if ( AnscEqualString (ParamName, "Enabled", TRUE) )
    {
	    	if ( bValue == pMyObject->EthWanCfg.Enable )
	    	{
	    		return TRUE;
	    	}
		else
		{
			if( ANSC_STATUS_SUCCESS == CosaDmlEthWanSetEnable( bValue )  )
			{
				pMyObject->EthWanCfg.Enable = bValue;
				return TRUE; 
			}
		}
    }

    return FALSE;
}

/***********************************************************************
APIs for Object:

    DeviceInfo.X_RDKCENTRAL_COM_xOpsDeviceMgmt.Logging. //GOK_TODO

    *  EthLogging_GetParamBoolValue
    *  EthLogging_GetParamUlongValue
    *  EthLogging_SetParamBoolValue
    *  EthLogging_SetParamUlongValue
    *  EthLogging_Validate
    *  EthLogging_Commit
    *  EthLogging_Rollback

***********************************************************************/

/**********************************************************************
    caller:     owner of this object

    prototype:
        BOOL
        EthLogging_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:
        This function is called to retrieve Boolean parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.
**********************************************************************/
BOOL
EthLogging_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    if (AnscEqualString(ParamName, "xOpsDMEthLogEnabled", TRUE))
    {
        *pBool =  pMyObject->LogStatus.Log_Enable;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        BOOL
        EthLogging_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.
**********************************************************************/
BOOL
EthLogging_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{    
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    if (AnscEqualString(ParamName, "xOpsDMEthLogPeriod", TRUE))
    {
        *puLong =  pMyObject->LogStatus.Log_Period;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:

        BOOL
        EthLogging_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.
**********************************************************************/
BOOL
EthLogging_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    char buf[8]={0};
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    if (AnscEqualString(ParamName, "xOpsDMEthLogEnabled", TRUE))
    {
        if(bValue)
            strcpy(buf,"true");
        else
            strcpy(buf,"false");

        if (syscfg_set(NULL, "eth_log_enabled", buf) != 0) 
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        } 
        else
        {
            if (syscfg_commit() != 0) 
            {
                AnscTraceWarning(("syscfg_commit failed\n"));
            }
            else
            {
                pMyObject->LogStatus.Log_Enable = bValue;
            }
        }
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        BOOL
        EthLogging_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG                       uValue
            );

    description:
        This function is called to set ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.
**********************************************************************/
BOOL
EthLogging_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    char buf[16]={0};
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    if (AnscEqualString(ParamName, "xOpsDMEthLogPeriod", TRUE))
    {
        sprintf(buf, "%d", uValue);

        if (syscfg_set(NULL, "eth_log_period", buf) != 0) 
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        } 
        else
        {
            if (syscfg_commit() != 0)
            {
                AnscTraceWarning(("syscfg_commit failed\n"));
            }
            else
            {
                pMyObject->LogStatus.Log_Period = uValue;
            }
        }
        return TRUE;
        }

    return FALSE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        BOOL
        EthLogging_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:
        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation.

                ULONG*                      puLength
                The output length of the param name.

    return:     TRUE if there's no validation.
**********************************************************************/
BOOL
EthLogging_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    return TRUE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        EthLogging_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:
        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.
**********************************************************************/
ULONG
EthLogging_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        EthLogging_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:
        This function is called to roll back the update whenever there's a
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.
**********************************************************************/
ULONG
EthLogging_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

