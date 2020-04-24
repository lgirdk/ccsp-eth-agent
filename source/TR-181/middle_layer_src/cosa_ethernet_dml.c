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
#include "safec_lib_common.h"

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
    errno_t rc = -1;
    int ind = -1;
    /* check the parameter name and return the corresponding value */
    rc = strcmp_s("X_RDKCENTRAL-COM_EthHost_Sync",strlen("X_RDKCENTRAL-COM_EthHost_Sync"),ParamName,&ind);
    ERR_CHK(rc);
    if((rc == EOK) && (ind == 0))
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
    errno_t rc = -1;
    int ind = -1;
    rc = strcmp_s("X_RDKCENTRAL-COM_EthHost_Sync",strlen("X_RDKCENTRAL-COM_EthHost_Sync"),ParamName,&ind);
    ERR_CHK(rc);
     if ((!ind) && (rc == EOK))
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
    errno_t rc       = -1;
    int     ind      = -1;
    rc = strcmp_s("Enabled",strlen("Enabled"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        *pBool =  pMyObject->EthWanCfg.Enable;
	CcspTraceWarning(("EthWan_GetParamBoolValue Ethernet WAN is '%d'\n", *pBool));
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
    errno_t rc       = -1;
    int     ind      = -1;
    rc = strcmp_s("Port",strlen("Port"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
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
#if !defined(AUTOWAN_ENABLE)
    errno_t rc       = -1;
    int     ind      = -1;
    rc = strcmp_s("Enabled",strlen("Enabled"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	    	if ( bValue == pMyObject->EthWanCfg.Enable )
	    	{
		    	CcspTraceWarning((" EthWan_SetParamBoolValue Ethernet WAN is '%d'\n", bValue));
	    		return TRUE;
	    	}
		else
		{
			//RDKB-27656 : Bridge Mode must not set to true using WEBPA & dmcli in ETHWAN mode
			BOOL bridgeInd = FALSE;
			is_usg_in_bridge_mode(&bridgeInd);
			if(bridgeInd)
			{
				CcspTraceWarning(("EthernetWAN mode is not supported in bridge mode. Disable Bridge mode to enable Ethernet WAN mode \n"));
				return FALSE;
			}
			if( ANSC_STATUS_SUCCESS == CosaDmlEthWanSetEnable( bValue )  )
			{
				pMyObject->EthWanCfg.Enable = bValue;
				CcspTraceWarning((" EthWan_SetParamBoolValue Ethernet WAN is '%d'\n", bValue));
				return TRUE; 
			}
		}
    }
#endif
    return FALSE;
}
/**********************************************************************  
    caller:     owner of this object 
    prototype: 
        ULONG
        EthernetWAN_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );
    description:
        This function is called to retrieve string parameter value; 
    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;
                char*                       ParamName,
                The parameter name;
                char*                       pValue,
                The string value buffer;
                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;
    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.
**********************************************************************/
#define WAN_MODE_AUTO		0
#define WAN_MODE_ETH		1
#define WAN_MODE_DOCSIS		2
#define WAN_MODE_UNKNOWN	3

ULONG
EthernetWAN_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
#ifdef AUTOWAN_ENABLE
    char buf[8]={0};
    int wan_mode = 0;
    errno_t rc       = -1;
    int     ind      = -1;
    /* check the parameter name and return the corresponding value */
    rc = strcmp_s( "SelectedOperationalMode",strlen("SelectedOperationalMode"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	    if (syscfg_get(NULL, "selected_wan_mode", buf, sizeof(buf)) == 0)
	    {
		if (buf != NULL)
		{
			if ( _ansc_strlen(buf) >= *pUlSize )
			{
			    *pUlSize = _ansc_strlen(buf);
			    return 1;
			}
			
			wan_mode = atoi(buf);
			if(wan_mode == WAN_MODE_DOCSIS)
			{
				rc = strcpy_s(pValue,*pUlSize,"DOCSIS");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                 }
			}
			else if (wan_mode == WAN_MODE_ETH)
			{
				rc = strcpy_s(pValue,*pUlSize, "Ethernet");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                 }

			}
			else
			{
				rc = strcpy_s(pValue,*pUlSize, "Auto");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                 }

			}
		}
	    }

        return 0;
    }
    rc = strcmp_s("LastKnownOperationalMode", strlen("LastKnownOperationalMode"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK)) 
    {
	    if (syscfg_get(NULL, "last_wan_mode", buf, sizeof(buf)) == 0)
	    {
		if (buf != NULL)
		{
			if ( _ansc_strlen(buf) >= *pUlSize )
			{
			    *pUlSize = _ansc_strlen(buf);
			    return 1;
			}

			wan_mode = atoi(buf);
			if(wan_mode == WAN_MODE_DOCSIS)
			{
                                rc = strcpy_s(pValue,*pUlSize,"DOCSIS");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                 }

			}
			else if (wan_mode == WAN_MODE_ETH)
			{
                                rc = strcpy_s(pValue,*pUlSize, "Ethernet");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                 }

			}
			else
			{
                                rc = strcpy_s(pValue,*pUlSize, "Unknown");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                 }

			}
		}
		else
		{
			AnscCopyString(pValue, "Unknown");
		}
	    }
            else
            {
		AnscCopyString(pValue, "Unknown");
            }

        return 0;
    }
    rc =  strcmp_s( "CurrentOperationalMode",strlen("CurrentOperationalMode"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
     
    {
	    if (syscfg_get(NULL, "curr_wan_mode", buf, sizeof(buf)) == 0)
	    {
		if (buf != NULL)
		{
			if ( _ansc_strlen(buf) >= *pUlSize )
			{
			    *pUlSize = _ansc_strlen(buf);
			    return 1;
			}

			wan_mode = atoi(buf);
			if(wan_mode == WAN_MODE_DOCSIS)
			{
				rc = strcpy_s(pValue, *pUlSize,"DOCSIS");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                }
			}
			else if (wan_mode == WAN_MODE_ETH)
			{
				rc = strcpy_s(pValue,*pUlSize, "Ethernet");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                }

			}
			else
			{
				rc = strcpy_s(pValue,*pUlSize, "Unknown");
                                if(rc != EOK)
                                {
                                   ERR_CHK(rc);
                                   return -1;
                                }

			}
		}
	    }

        return 0;
    }
#endif
    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  
    caller:     owner of this object 
    prototype: 
        BOOL
        EthernetWAN_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );
    description:
        This function is called to set string parameter value; 
    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;
                char*                       ParamName,
                The parameter name;
                char*                       pString
                The updated string value;
    return:     TRUE if succeeded.
**********************************************************************/
BOOL
EthernetWAN_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
#ifdef AUTOWAN_ENABLE
    BOOL  bValue = FALSE;
    char buf[8]={0};
    int wan_mode = 0;
    errno_t rc = -1;
    int            ind = -1;
    /* check the parameter name and set the corresponding value */
    rc = strcmp_s( "SelectedOperationalMode",strlen("SelectedOperationalMode"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	rc = strcmp_s("DOCSIS",strlen("DOCSIS"),pString,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		bValue = FALSE;
	}
	else
        {
           rc = strcmp_s("Ethernet",strlen("Ethernet"),pString,&ind);
           ERR_CHK(rc);
           if ((!ind) && (rc == EOK))
           {
		bValue = TRUE;
	   }
        }

        rc = strcmp_s("DOCSIS",strlen("DOCSIS"),pString,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		wan_mode = WAN_MODE_DOCSIS;
	}
	else
        {
           rc = strcmp_s("Ethernet",strlen("Ethernet"),pString,&ind);
           ERR_CHK(rc);
           if ((!ind) && (rc == EOK))
	   {
		wan_mode = WAN_MODE_ETH;
	   }
	   else
	   {
		wan_mode = WAN_MODE_AUTO;
	   }
        }
        snprintf(buf, sizeof(buf), "%d", wan_mode);
	if (syscfg_set(NULL, "selected_wan_mode", buf) != 0) 
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
            	int cur_wan_mode= 0;
                rc =  memset_s(buf,sizeof(buf), 0, sizeof(buf));
                ERR_CHK(rc);
		    if (syscfg_get(NULL, "curr_wan_mode", buf, sizeof(buf)) == 0)
		    {
			if (buf != NULL)
			{
			
				cur_wan_mode = atoi(buf);
				if(cur_wan_mode == wan_mode)
				{
					CcspTraceWarning(("SelectedOperationalMode - %s is same as CurrentWanMode\n", pString));
					return TRUE;
				}

			}
		    }
                rc = strcmp_s("Auto",strlen("Auto"),pString,&ind);
                ERR_CHK(rc);
                if ((ind) && (rc == EOK))
		{
		    if( ANSC_STATUS_SUCCESS == CosaDmlEthWanSetEnable( bValue ) )
		    {
			//pMyObject->EthWanCfg.Enable = bValue;
			CcspTraceWarning(("SelectedOperationalMode is %s\n", pString));
			
		    }
		}
		return TRUE; 
            }
        }

    }
#endif
    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
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
    errno_t rc       = -1;
    int     ind      = -1;
    
    rc = strcmp_s("xOpsDMEthLogEnabled",strlen("xOpsDMEthLogEnabled"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
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
    errno_t        rc = -1;
    int            ind = -1;
    rc = strcmp_s( "xOpsDMEthLogPeriod",strlen( "xOpsDMEthLogPeriod"),ParamName,&ind);
    ERR_CHK(rc);
    if( (ind == 0) && (rc == EOK))
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
    errno_t rc = -1;
    int            ind = -1;
    rc = strcmp_s( "xOpsDMEthLogEnabled",strlen( "xOpsDMEthLogEnabled"),ParamName,&ind);
    ERR_CHK(rc);
    if( (ind == 0) && (rc == EOK))
    {
        if(bValue)
        {
            rc = strcpy_s(buf,sizeof(buf),"true");
            if(rc != EOK)
           {
              ERR_CHK(rc);
              return FALSE;
           }
     }
     else
     {
             rc =  strcpy_s(buf,sizeof(buf),"false");
             if(rc != EOK)
             {
                ERR_CHK(rc);
                return FALSE;
             }
     }  
        

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
		CosaEthTelemetryxOpsLogSettingsSync();
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
    errno_t        rc = -1;
    int            ind = -1;
    rc = strcmp_s( "xOpsDMEthLogPeriod",strlen( "xOpsDMEthLogPeriod"),ParamName,&ind);
    ERR_CHK(rc);
    if( (ind == 0) && (rc == EOK))

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
		CosaEthTelemetryxOpsLogSettingsSync();
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

/**********************************************************************
    caller:     owner of this object

    prototype:
        BOOL
        AutowanFeatureSupport_GetParamBoolValue
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
AutowanFeatureSupport_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    errno_t        rc =    -1;
    int            ind = -1;
    /*This parameter is created for the purpose of whether the AUTOWAN feature is enabled inside the UI.*/
    rc = strcmp_s("X_RDKCENTRAL-COM_AutowanFeatureSupport", strlen("X_RDKCENTRAL-COM_AutowanFeatureSupport"), ParamName,&ind);
    ERR_CHK(rc);
    if( (ind == 0) && (rc == EOK))
    {
#if defined(AUTOWAN_ENABLE)
        *pBool =  1;
#else
        *pBool =  0;
#endif /* AUTOWAN_ENABLE */
	 return TRUE;
    }
    return FALSE;
}

