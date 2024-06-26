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
#include "cosa_ethernet_apis.h"
#include <syscfg/syscfg.h>
#include "cosa_ethernet_internal.h"
#include "cosa_ethernet_interface_dml.h"

extern ANSC_HANDLE g_EthObject;
ANSC_STATUS is_usg_in_bridge_mode(BOOL *pBridgeMode);

ANSC_STATUS
CosaDmlEthWanSetEnable
    (
        BOOL                       bEnable
    );

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
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(pBool);
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
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(bValue);
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
    UNREFERENCED_PARAMETER(hInsContext);
   
    errno_t rc       = -1;
    int     ind      = -1;
    rc = strcmp_s("Enabled",strlen("Enabled"),ParamName,&ind);
    ERR_CHK(rc);
    //cmxb7-5072
    #if defined (FEATURE_RDKB_WAN_MANAGER)
    if((!ind) && (rc == EOK))
    {
        *pBool = isEthWanEnabled();
        return TRUE;
    }
    #else
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    if ((!ind) && (rc == EOK))
    {
        *pBool =  pMyObject->EthWanCfg.Enable;
        return TRUE;
    }
    #endif
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
    UNREFERENCED_PARAMETER(hInsContext);
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
#if !defined(AUTOWAN_ENABLE)
BOOL
EthWan_SetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                         bValue
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
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
			#if 0
			//RDKB-27656 : Bridge Mode must not set to true using WEBPA & dmcli in ETHWAN mode
			BOOL bridgeInd = FALSE;
                        /*Coverity Fix CID :139267 CHECKED_RETURN */
			if( ANSC_STATUS_SUCCESS !=  is_usg_in_bridge_mode(&bridgeInd) )
                        {
		    	   CcspTraceWarning(("is_usg_in_bridge_mode is not success\n"));
                           return  FALSE;
                        }
			
                        if(bridgeInd)
			{
				CcspTraceWarning(("EthernetWAN mode is not supported in bridge mode. Disable Bridge mode to enable Ethernet WAN mode \n"));
				return FALSE;
			}
			#endif
			if( ANSC_STATUS_SUCCESS == CosaDmlEthWanSetEnable( bValue )  )
			{
				pMyObject->EthWanCfg.Enable = bValue;
				CcspTraceWarning((" EthWan_SetParamBoolValue Ethernet WAN is '%d'\n", bValue));
				return TRUE;
			}
		}
    }
    return FALSE;
}
#endif
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

#ifdef AUTOWAN_ENABLE
ULONG
EthernetWAN_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(pUlSize);
    UNREFERENCED_PARAMETER(pValue);
    UNREFERENCED_PARAMETER(ParamName);
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
		/*  CID 162007 CID 162005 Array compared against 0  fix */
		if (buf[0] != '\0')
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
		/*  CID 162007 CID 162005 Array compared against 0  fix */
		if (buf[0] != '\0')
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
		if (buf[0] != '\0')
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
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(pString);
    UNREFERENCED_PARAMETER(ParamName);
    BOOL  bValue = FALSE;
    int wan_mode = 0;
    errno_t rc = -1;
    int            ind = -1;

#if defined (FEATURE_RDKB_WAN_MANAGER)
    int wan_oper_state = WAN_OPER_STATE_NONE;
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    PCOSA_DATAMODEL_ETH_WAN_AGENT pEthWanCfgObj = NULL;
    if (pMyObject)
    {
        pEthWanCfgObj = (PCOSA_DATAMODEL_ETH_WAN_AGENT)&pMyObject->EthWanCfg;
        if (pEthWanCfgObj)
        {
            wan_oper_state = pEthWanCfgObj->wanOperState;
        }
    }
#endif
    /* check the parameter name and set the corresponding value */
    rc = strcmp_s( "SelectedOperationalMode",strlen("SelectedOperationalMode"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
#if defined (FEATURE_RDKB_WAN_MANAGER)
        if (wan_oper_state == WAN_OPER_STATE_RESET_INPROGRESS)
        {
		    CcspTraceWarning(("WanOperationalMode Change is inprogress - pls try %s=%s to set after sometime \n", ParamName,pString));
            return FALSE;
        }
        else
        {
            if (pEthWanCfgObj)
            {
                pEthWanCfgObj->wanOperState = WAN_OPER_STATE_NONE;
            }
        }
#endif
        rc = strcmp_s("DOCSIS",strlen("DOCSIS"),pString,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
	#if !defined (_XER5_PRODUCT_REQ_) && !defined(_SCER11BEL_PRODUCT_REQ_)
            bValue = FALSE;
            wan_mode = WAN_MODE_DOCSIS;
	#else
	     CcspTraceWarning(("DOCSIS operationalMode is not supported in XER5/XER10 platform\n"));
             return FALSE;
	#endif
        }
        else if((strcmp_s("Ethernet",strlen("Ethernet"),pString,&ind) == EOK) && (ind == 0))
        {
	    #if 0
            is_usg_in_bridge_mode(&bridge_mode_enabled);
            ERR_CHK(rc);
            if ((rc == EOK) && bridge_mode_enabled) {
               CcspTraceWarning(("EthernetWAN mode is not supported in bridge mode.\n"));
               return FALSE;
	    }
	    #endif
		bValue = TRUE;
		wan_mode = WAN_MODE_ETH;
        }
	else if((strcmp_s("Auto",strlen("Auto"),pString,&ind) == EOK) && (ind == 0))
	{
	#if !defined (_XER5_PRODUCT_REQ_) && !defined(_SCER11BEL_PRODUCT_REQ_)
	    wan_mode = WAN_MODE_AUTO;
	#else
	    CcspTraceWarning(("Auto operationalMode is not supported in XER5/XER10 platform\n"));
            return FALSE;
	#endif
	}
	else
	{
		CcspTraceWarning(("SelectedOperationalMode - %s is not valid\n", pString));
		return FALSE;
	}
#if defined (FEATURE_RDKB_WAN_MANAGER)
    if (pEthWanCfgObj)
    {
        char buf[8];
        if (syscfg_get(NULL, "selected_wan_mode", buf, sizeof(buf)) == 0)
        {
            pEthWanCfgObj->PrevSelMode = atoi(buf);
        }
    }
#endif

	if (syscfg_set_u_commit(NULL, "selected_wan_mode", wan_mode) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
#if !defined(WAN_MANAGER_UNIFICATION_ENABLED)
            /*SelectedOperationalMode is set from WanManager policy. Configure ethWan without checking the previous mode.  */
            char buf[8];
            int cur_wan_mode= 0;

            if (syscfg_get(NULL, "curr_wan_mode", buf, sizeof(buf)) == 0)
            {
                cur_wan_mode = atoi(buf);
                if(cur_wan_mode == wan_mode)
                {
                    CcspTraceWarning(("SelectedOperationalMode - %s is same as CurrentWanMode\n", pString));
                    return TRUE;
                }
            }
#endif /* WAN_MANAGER_UNIFICATION_ENABLED */
            rc = strcmp_s("Auto",strlen("Auto"),pString,&ind);
            ERR_CHK(rc);
            if ((ind) && (rc == EOK))
            {
                if( ANSC_STATUS_SUCCESS != CosaDmlEthWanSetEnable( bValue ) )
                {
                    return FALSE;
                }
                //pMyObject->EthWanCfg.Enable = bValue;
                CcspTraceWarning(("SelectedOperationalMode is %s\n", pString));
            }
		return TRUE;
        }
    }

#if defined (FEATURE_RDKB_WAN_MANAGER)
    rc = strcmp_s("RequestPhyStatus",strlen("RequestPhyStatus"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
        if (pMyObject)
        {
            PCOSA_DATAMODEL_ETH_WAN_AGENT pEthWanCfg = (PCOSA_DATAMODEL_ETH_WAN_AGENT)&pMyObject->EthWanCfg;
            if (pEthWanCfg)
            {
                rc = strcpy_s(pEthWanCfg->wanInstanceNumber,sizeof(pEthWanCfg->wanInstanceNumber),pString);
                if(rc != EOK)
                {
                    ERR_CHK(rc);
                    return FALSE;
                }

#if !defined(WAN_MANAGER_UNIFICATION_ENABLED)
                if (pEthWanCfg->MonitorPhyStatusAndNotify != TRUE)
                {
                    pEthWanCfg->MonitorPhyStatusAndNotify = TRUE;
                    CosaDmlEthWanPhyStatusMonitor(pMyObject);
                }
#endif
            }
        }

       return TRUE;
    }

    rc = strcmp_s("PostCfgWanFinalize",strlen("PostCfgWanFinalize"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        CosaDmlIfaceFinalize(pString,TRUE);
        return TRUE;
    }
#endif


    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}
#endif

BOOL EthernetWAN_SetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
#if defined (FEATURE_RDKB_WAN_MANAGER)

    errno_t rc       = -1;
    int     ind      = -1;
    UNREFERENCED_PARAMETER(hInsContext);
    rc = strcmp_s("ConfigureWan",strlen("ConfigureWan"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        if (ANSC_STATUS_SUCCESS != CosaDmlConfigureEthWan(bValue))
            return FALSE;
        return TRUE;
    }
#else
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(bValue);

#endif
    return FALSE;
}

BOOL EthernetWAN_GetParamBoolValue

    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pBool);
    return TRUE;
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
    UNREFERENCED_PARAMETER(hInsContext);
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
    UNREFERENCED_PARAMETER(hInsContext);
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
    UNREFERENCED_PARAMETER(hInsContext);
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

        if (syscfg_set_commit(NULL, "eth_log_enabled", buf) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
                pMyObject->LogStatus.Log_Enable = bValue;
		CosaEthTelemetryxOpsLogSettingsSync();
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
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    errno_t        rc = -1;
    int            ind = -1;
    rc = strcmp_s( "xOpsDMEthLogPeriod",strlen( "xOpsDMEthLogPeriod"),ParamName,&ind);
    ERR_CHK(rc);
    if( (ind == 0) && (rc == EOK))

    {
        if (syscfg_set_u_commit(NULL, "eth_log_period", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
                pMyObject->LogStatus.Log_Period = uValue;
		CosaEthTelemetryxOpsLogSettingsSync();
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
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(pReturnParamName);
    UNREFERENCED_PARAMETER(puLength);
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
    UNREFERENCED_PARAMETER(hInsContext);
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
    UNREFERENCED_PARAMETER(hInsContext);
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
    UNREFERENCED_PARAMETER(hInsContext);
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

#if defined(FEATURE_RDKB_WAN_MANAGER)
/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        EthRdkInterface_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/
ANSC_HANDLE
EthRdkInterface_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    PCOSA_CONTEXT_LINK_OBJECT       pCxtLink          = NULL;

    pSListEntry       = AnscSListGetEntryByIndex(&pMyObject->Q_EthList, nIndex);

    if ( pSListEntry )
    {
        pCxtLink      = ACCESS_COSA_CONTEXT_LINK_OBJECT(pSListEntry);
        *pInsNumber   = pCxtLink->InstanceNumber;
    }
    return (ANSC_HANDLE)pSListEntry;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        EthRdkInterface_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
ULONG
EthRdkInterface_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DATAMODEL_ETHERNET   pMyObject   = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    return AnscSListQueryDepth( &pMyObject->Q_EthList );
}

ULONG EthRdkInterface_DelEntry ( ANSC_HANDLE hInsContext, ANSC_HANDLE hInstance )
{
    ANSC_STATUS               returnStatus      = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_ETHERNET    pEthLink = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    PCOSA_CONTEXT_LINK_OBJECT      pEthCxtLink       = (PCOSA_CONTEXT_LINK_OBJECT)hInstance;
    PCOSA_DML_ETH_PORT_CONFIG             p_EthLink         = (PCOSA_DML_ETH_PORT_CONFIG)pEthCxtLink->hContext;
    UNREFERENCED_PARAMETER(hInsContext);

    pEthCxtLink = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    if ( ( p_EthLink->Enable == TRUE ) ||
         ( ANSC_STATUS_SUCCESS != CosDmlEthPortUpdateGlobalInfo((PANSC_HANDLE)pEthLink,  p_EthLink->Name, ETH_DEL_TABLE ) ) )
    {
       CcspTraceError(("[%s][%d] Active or Internal Interface cant be deleted. \n", __FUNCTION__, __LINE__));
       return ANSC_STATUS_FAILURE;
    }
    if ( pEthCxtLink->bNew )
    {
        /* Set bNew to FALSE to indicate this node is not going to save to SysRegistry */
        pEthCxtLink->bNew = FALSE;
    }
    if ( AnscSListPopEntryByLink(&pEthLink->Q_EthList, &pEthCxtLink->Linkage) )
    {
        AnscFreeMemory(pEthCxtLink->hContext);
        AnscFreeMemory(pEthCxtLink);
    }
    return returnStatus;
}
ANSC_HANDLE EthRdkInterface_AddEntry( ANSC_HANDLE hInsContext, ULONG* pInsNumber )
{
    PCOSA_DATAMODEL_ETHERNET    gpEthLink      = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    PCOSA_DML_ETH_PORT_CONFIG   p_EthLink      = NULL;
    PCOSA_CONTEXT_LINK_OBJECT   pEthCxtLink    = NULL;
    UNREFERENCED_PARAMETER(hInsContext);

    p_EthLink = (PCOSA_DML_ETH_PORT_CONFIG)AnscAllocateMemory(sizeof(COSA_DML_ETH_PORT_CONFIG));
    if ( !p_EthLink )
    {
        return NULL;
    }
    pEthCxtLink = (PCOSA_CONTEXT_LINK_OBJECT)AnscAllocateMemory(sizeof(COSA_CONTEXT_LINK_OBJECT));
    if ( !pEthCxtLink )
    {
        goto EXIT;
    }
    /* now we have this link content */
    pEthCxtLink->hContext = (ANSC_HANDLE)p_EthLink;
    pEthCxtLink->bNew     = TRUE;
    /* Get InstanceNumber and Default values */
    InitEthIfaceEntry(NULL, p_EthLink);
    pEthCxtLink->InstanceNumber = p_EthLink->ulInstanceNumber ;
     *pInsNumber                 = p_EthLink->ulInstanceNumber ;
    CosaSListPushEntryByInsNum(&gpEthLink->Q_EthList, (PCOSA_CONTEXT_LINK_OBJECT)pEthCxtLink);
    CosDmlEthPortUpdateGlobalInfo((PANSC_HANDLE)gpEthLink, p_EthLink->Name, ETH_ADD_TABLE );
    return (ANSC_HANDLE)pEthCxtLink;
EXIT:
    AnscFreeMemory(p_EthLink);
    return NULL;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthRdkInterface_GetParamBoolValue
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
EthRdkInterface_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
   )
{
    /* check the parameter name and return the corresponding value */
    PCOSA_CONTEXT_LINK_OBJECT   pCxtLink      = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink      = (PCOSA_DML_ETH_PORT_CONFIG)pCxtLink->hContext;
    /* check the parameter name and return the corresponding value */
    /*TODO:
     * The Upstream And WanValidated should not be used if WAN_MANAGER_UNIFICATION_ENABLED is Enabled.
     * Need to be Removed the upstream and WanValidated once whomever is using it, will be moved to WAN_MANAGER_UNIFICATION_ENABLED.
     */
    if (strcmp(ParamName, "Enable") == 0)
    {
        *pBool = pEthLink->Enable;
        return TRUE;
    }
    if (strcmp(ParamName, "Upstream") == 0)
    {
        *pBool = pEthLink->Upstream;
        return TRUE;
    }
    if (strcmp(ParamName, "AddToLanBridge") == 0)
    {
        *pBool = pEthLink->AddToLanBridge;
        return TRUE;
    }
    if (strcmp(ParamName, "WanValidated") == 0)
    {
        *pBool = pEthLink->WanValidated;
        return TRUE;
    }
    if (strcmp(ParamName, "WanConfigPort") == 0)
    {
        UINT WanPort = 0;

        if(0 != CcspHalExtSw_getEthWanPort(&WanPort))
        {
            AnscTraceInfo(("Failed to get WanPort[%u] in CPE \n",WanPort));
        }

        if(WanPort == pEthLink->ulInstanceNumber
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
                && CcspHalExtSw_getCurrentWanHWConf()
#endif
          )
        {
            *pBool = TRUE;
        }
        else
        {
            *pBool = FALSE;
        }
        return TRUE;
    }
     return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        EthRdkInterface_GetParamStringValue
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
ULONG
EthRdkInterface_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCOSA_CONTEXT_LINK_OBJECT   pCxtLink      = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink      = (PCOSA_DML_ETH_PORT_CONFIG)pCxtLink->hContext;
    if (strcmp(ParamName, "Name") == 0)
    {
       /* collect value */
       if ( ( sizeof( pEthLink->Name ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue,  pEthLink->Name);
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pEthLink->Name );
           return 1;
       }
    }
    if (strcmp(ParamName, "LowerLayers") == 0)
    {
       /* collect value */
       if ( ( sizeof( pEthLink->LowerLayers ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue,  pEthLink->LowerLayers);
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pEthLink->LowerLayers );
           return 1;
       }
    }
    return -1;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthRdkInterface_SetParamStringValue
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
EthRdkInterface_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pString);
    PCOSA_CONTEXT_LINK_OBJECT   pCxtLink        = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink        = (PCOSA_DML_ETH_PORT_CONFIG)pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Name") == 0)
    {
       /* collect value */
       if ( ANSC_STATUS_SUCCESS == CosaDmlEthPortSetName(pEthLink->Name, pString))
       {
           AnscCopyString( pEthLink->Name, pString);
           return TRUE;
       }
       else
       {
           return FALSE;
       }
    }
    if (strcmp(ParamName, "LowerLayers") == 0)
    {
       /* collect value */
       if ( ANSC_STATUS_SUCCESS == CosaDmlEthPortSetLowerLayers(pEthLink->Name, pString))
       {
	   /* CID 281826 Copy into fixed size buffer */
	   if(strlen(pString) < (sizeof(pEthLink->LowerLayers) -1))
	   {
		AnscCopyString( pEthLink->LowerLayers, pString);
		return TRUE;
	   }
	   else
	   {
		return FALSE;
	   }
       }
       else
       {
           return FALSE;
       }
    }
    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthRdkInterface_GetParamUlongValue
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
EthRdkInterface_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_CONTEXT_LINK_OBJECT       pCxtLink      = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink        = (PCOSA_DML_ETH_PORT_CONFIG)pCxtLink->hContext;
    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Status") == 0)
    {
        COSA_DML_ETH_LINK_STATUS linkstatus;
        if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetLinkStatus(pEthLink->Name, &linkstatus))
        {
            pEthLink->LinkStatus = linkstatus;
        }
        *puLong = pEthLink->LinkStatus;
        return TRUE;
    }
    /*TODO:
     * The WanStatus should not be used if WAN_MANAGER_UNIFICATION_ENABLED is Enabled.
     * Need to be Removed the WanStatus once whomever is using it, will be moved to WAN_MANAGER_UNIFICATION_ENABLED.
     */
    if (strcmp(ParamName, "WanStatus") == 0)
    {
        COSA_DML_ETH_WAN_STATUS wan_status;
        if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetWanStatus(pEthLink->Name, &wan_status))
        {
            pEthLink->WanStatus = wan_status;
        }
        *puLong = pEthLink->WanStatus;
        return TRUE;
    }
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
    if (strcmp(ParamName, "PortCapability") == 0)
    {
        *puLong = pEthLink->PortCapability;
        return TRUE;
    }
#endif  //FEATURE_RDKB_AUTO_PORT_SWITCH
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthRdkInterface_SetParamUlongValue
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
EthRdkInterface_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_CONTEXT_LINK_OBJECT   pCxtLink      = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink      = (PCOSA_DML_ETH_PORT_CONFIG)pCxtLink->hContext;
    /*TODO:
     * The WanStatus should not be used if WAN_MANAGER_UNIFICATION_ENABLED is Enabled.
     * Need to be Removed the WanStatus once whomever is using it, will be moved to WAN_MANAGER_UNIFICATION_ENABLED.
     */
    if (strcmp(ParamName, "WanStatus") == 0)
    {
        if (uValue == pEthLink->WanStatus)
        {
            return TRUE; //No need to proceed when same value comes
        }
        pEthLink->WanStatus = uValue;
        CosaDmlEthPortSetWanStatus(pEthLink->Name, pEthLink->WanStatus);
        return TRUE;
    }
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
    if (strcmp(ParamName, "PortCapability") == 0)
    {
        ULONG prevValue = pEthLink->PortCapability;
        pEthLink->PortCapability = uValue;
        if(prevValue == uValue)
        {
            CcspTraceInfo(("%s The new PortCapability value matched with the set value, so not updating \n",__FUNCTION__));
            return TRUE;
        }
        if ( ANSC_STATUS_FAILURE == CosaDmlEthPortSetPortCapability(pEthLink) )
        {
            pEthLink->PortCapability = prevValue;
            CcspTraceWarning(("%s Failed to update PortCapability\n",__FUNCTION__));
        }
        return TRUE;
    }
#endif  //FEATURE_RDKB_AUTO_PORT_SWITCH
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthRdkInterface_SetParamBoolValue
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
EthRdkInterface_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_CONTEXT_LINK_OBJECT   pCxtLink      = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink      = (PCOSA_DML_ETH_PORT_CONFIG)pCxtLink->hContext;
    /* check the parameter name and set the corresponding value */
    /*TODO:
     * The Upstream And WanValidated should not be used if WAN_MANAGER_UNIFICATION_ENABLED is Enabled.
     * Need to be Removed the upstream and WanValidated once whomever is using it, will be moved to WAN_MANAGER_UNIFICATION_ENABLED.
     */
    if (strcmp(ParamName, "Upstream") == 0)
    {
        if( bValue == pEthLink->Upstream )
        {
            return TRUE;	//No need to proceed when same value comes
        }
        pEthLink->Upstream = bValue;
#if !defined(WAN_MANAGER_UNIFICATION_ENABLED)
        CosaDmlEthPortSetUpstream( pEthLink->Name , pEthLink->Upstream );
#endif
        return TRUE;
    }
    if (strcmp(ParamName, "Enable") == 0)
    {
        if( bValue == pEthLink->Enable )
        {
            return TRUE;	//No need to proceed when same value comes
        }
        CosaDmlTriggerExternalEthPortLinkStatus(pEthLink->Name, bValue);
        pEthLink->Enable = bValue;
        return TRUE;
    }
    if (strcmp(ParamName, "WanValidated") == 0)
    {
        if (bValue == pEthLink->WanValidated)
        {
            return TRUE;
        }
        pEthLink->WanValidated = bValue;
        CosaDmlEthPortSetWanValidated(( pEthLink->ulInstanceNumber - 1 ), pEthLink->WanValidated );
        return TRUE;
    }
    if (strcmp(ParamName, "AddToLanBridge") == 0)
    {
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
        // check if port is WAN capable, before removing/adding into bridge
        UINT WanPort = 0;
	/* CID 133789 Unchecked return value */
	if(0 != CcspHalExtSw_getEthWanPort(&WanPort))
	{
	    AnscTraceInfo(("Failed to get WanPort[%u] in CPE \n",WanPort));
	}
        if(WanPort != pEthLink->ulInstanceNumber)
        {
            CcspTraceError(("%s %d: Cannot set AddToLanBridge for non-WAN capable port\n", __FUNCTION__, __LINE__));
            return FALSE;
        }

        if (ANSC_STATUS_SUCCESS == EthMgr_AddPortToLanBridge (pEthLink, bValue))
        {
            CcspTraceInfo(("%s %d: successfully set AddtoBridge = %d for interface %s \n", __FUNCTION__, __LINE__, bValue, pEthLink->Name));
            pEthLink->AddToLanBridge = bValue;
            return TRUE;
        }
        CcspTraceError(("%s %d: unable to set AddtoBridge = %d for interface %s \n", __FUNCTION__, __LINE__, bValue, pEthLink->Name));
#else
        return TRUE;
#endif
    }
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthRdkInterface_Validate
        (
            ANSC_HANDLE                 hInsContext,
            char*                       pReturnParamName,
            ULONG*                      puLength
        )
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
EthRdkInterface_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(pReturnParamName);
    UNREFERENCED_PARAMETER(puLength);
    return TRUE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        EthRdkInterface_Commit
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
EthRdkInterface_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    return 0;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        EthRdkInterface_Rollback
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
EthRdkInterface_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    return 0;
}

#elif defined(FEATURE_RDKB_WAN_AGENT)
/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        EthInterface_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/
ANSC_HANDLE
EthInterface_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    if ( ( pMyObject->pEthLink  ) && ( nIndex < pMyObject->ulTotalNoofEthInterfaces ) )
    {
        PCOSA_DML_ETH_PORT_CONFIG      pEthlink = NULL;
        pEthlink = pMyObject->pEthLink + nIndex;
        pEthlink->ulInstanceNumber = nIndex + 1;
        *pInsNumber = pEthlink->ulInstanceNumber;
        CosaDmlEthGetPortCfg(nIndex, pEthlink);
        return pEthlink;
    }
    return NULL; /* return the invlalid handle */
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        EthInterface_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
ULONG
EthInterface_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DATAMODEL_ETHERNET   pMyObject   = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    return pMyObject->ulTotalNoofEthInterfaces;

}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthInterface_GetParamBoolValue
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
EthInterface_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
   )
{
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink   = (PCOSA_DML_ETH_PORT_CONFIG)hInsContext;
    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_Upstream") == 0)
    {
        *pBool = pEthLink->Upstream;
        return TRUE;
    }
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_WanValidated") == 0)
    {
        *pBool = pEthLink->WanValidated;
        return TRUE;
    }

     return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        EthInterface_GetParamStringValue
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
ULONG
EthInterface_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCOSA_DML_ETH_PORT_CONFIG pEthLink = (PCOSA_DML_ETH_PORT_CONFIG)hInsContext;
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_Name") == 0)
    {
       /* collect value */
       if ( ( sizeof( pEthLink->Name ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue,  pEthLink->Name);
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pEthLink->Name );
           return 1;
       }
    }
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthInterface_SetParamStringValue
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
EthInterface_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pString);
    /* check the parameter name and return the corresponding value */
    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Interface_GetParamUlongValue
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
EthInterface_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink   = (PCOSA_DML_ETH_PORT_CONFIG)hInsContext;
    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_Status") == 0)
    {
        COSA_DML_ETH_LINK_STATUS linkstatus;
        if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetLinkStatus(pEthLink->ulInstanceNumber - 1, &linkstatus))
        {
            pEthLink->LinkStatus = linkstatus;
        }

        *puLong = pEthLink->LinkStatus;
        return TRUE;
    }

    if (strcmp(ParamName, "X_RDKCENTRAL-COM_WanStatus") == 0)
    {
        COSA_DML_ETH_WAN_STATUS wan_status;
        if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetWanStatus(pEthLink->ulInstanceNumber - 1, &wan_status))
        {
            pEthLink->WanStatus = wan_status;
        }

        *puLong = pEthLink->WanStatus;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthInterface_SetParamUlongValue
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
EthInterface_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink   = (PCOSA_DML_ETH_PORT_CONFIG)hInsContext;
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_WanStatus") == 0)
    {
        if (uValue == pEthLink->WanStatus)
        {
            return TRUE;	//No need to proceed when same value comes
        }
        pEthLink->WanStatus = uValue;
        CosaDmlEthPortSetWanStatus(pEthLink->ulInstanceNumber - 1, pEthLink->WanStatus);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthInterface_SetParamBoolValue
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
EthInterface_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DML_ETH_PORT_CONFIG   pEthLink   = (PCOSA_DML_ETH_PORT_CONFIG)hInsContext;
    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_Upstream") == 0)
    {
        if( bValue == pEthLink->Upstream )
        {
            return TRUE;	//No need to proceed when same value comes
        }

        pEthLink->Upstream = bValue;
#if !defined(WAN_MANAGER_UNIFICATION_ENABLED)
	CosaDmlEthPortSetUpstream(( pEthLink->ulInstanceNumber - 1 ), pEthLink->Upstream );
#endif
        return TRUE;
    }

    if (strcmp(ParamName, "X_RDKCENTRAL-COM_WanValidated") == 0)
    {
        if (bValue == pEthLink->WanValidated)
        {
            return TRUE;
        }

        pEthLink->WanValidated = bValue;
        CosaDmlEthPortSetWanValidated(( pEthLink->ulInstanceNumber - 1 ), pEthLink->WanValidated );

        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EthInterface_Validate
        (
            ANSC_HANDLE                 hInsContext,
            char*                       pReturnParamName,
            ULONG*                      puLength
        )
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
EthInterface_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(pReturnParamName);
    UNREFERENCED_PARAMETER(puLength);
    return TRUE;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        Interface_Commit
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
EthInterface_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    return 0;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        EthInterface_Rollback
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
EthInterface_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    return 0;
}
#endif //FEATURE_RDKB_WAN_MANAGER
