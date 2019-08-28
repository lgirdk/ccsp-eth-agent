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

    module: cosa_ethernet_apis.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  CosaDmlEthInit
        *  CosaDmlEthPortGetNumberOfEntries
        *  CosaDmlEthPortGetEntry
        *  CosaDmlEthPortSetCfg
        *  CosaDmlEthPortGetCfg
        *  CosaDmlEthPortGetDinfo
        *  CosaDmlEthPortGetStats
        *  CosaDmlEthLinkGetNumberOfEntries
        *  CosaDmlEthLinkGetEntry
        *  CosaDmlEthLinkAddEntry
        *  CosaDmlEthLinkDelEntry
        *  CosaDmlEthLinkSetCfg
        *  CosaDmlEthLinkGetCfg
        *  CosaDmlEthLinkGetDinfo
        *  CosaDmlEthLinkGetStats
        *  CosaDmlEthVlanTerminationGetNumberOfEntries
        *  CosaDmlEthVlanTerminationGetEntry
        *  CosaDmlEthVlanTerminationAddEntry
        *  CosaDmlEthVlanTerminationDelEntry
        *  CosaDmlEthVlanTerminationSetCfg
        *  CosaDmlEthVlanTerminationGetCfg
        *  CosaDmlEthVlanTerminationGetDinfo
        *  CosaDmlEthVlanTerminationGetStats
    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/11/2011    initial revision.

**************************************************************************/
#include <stdio.h>
#include <string.h>
#include "cosa_ethernet_apis.h"



/**************************************************************************
                        DATA STRUCTURE DEFINITIONS
**************************************************************************/

/**************************************************************************
                        GLOBAL VARIABLES
**************************************************************************/



extern  ANSC_HANDLE bus_handle;
extern  ANSC_HANDLE g_EthObject;
extern void* g_pDslhDmlAgent;

void Notify_To_LMLite(Eth_host_t *host)
{
    parameterValStruct_t notif_val[1];
    char compo[256] = "eRT.com.cisco.spvtg.ccsp.lmlite"; 
    char bus[256] = "/com/cisco/spvtg/ccsp/lmlite";
    char param_name[256] = "Device.Hosts.X_RDKCENTRAL-COM_EthHost_Sync";
    char param_value[25];
    char* faultParam = NULL;
    int ret = CCSP_FAILURE;

    sprintf
    (
        param_value,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        host->eth_macAddr[0],
        host->eth_macAddr[1],
        host->eth_macAddr[2],
        host->eth_macAddr[3],
        host->eth_macAddr[4],
        host->eth_macAddr[5]
    );

    sprintf(param_value+17,"%s",",");
    if(host->eth_Active)
    	sprintf(param_value+18,"%s","true");
    else
	sprintf(param_value+18,"%s","false");

    notif_val[0].parameterName =  param_name ;
    notif_val[0].parameterValue = param_value;
    notif_val[0].type = ccsp_string;
			
    ret = CcspBaseIf_setParameterValues(
              bus_handle,
              compo,
              bus,
              0,
              0,
              notif_val,
              1,
              TRUE,
              &faultParam
              );

    if( ret != CCSP_SUCCESS )
    {
        CcspTraceWarning(("%s : FAILED to sync with LMLite ret: %d \n",__FUNCTION__,ret));
    }

}

void Ethernet_Hosts_Sync( void )
{
	CcspHalExtSw_SendNotificationForAllHosts( );
	CcspTraceInfo(("%s-%d Sync With LMLite\n",__FUNCTION__,__LINE__));	
}

INT CosaDmlEth_AssociatedDevice_callback(eth_device_t *eth_dev)
{
	Eth_host_t Eth_Host;
    char mac_id[18] = {0};
    BOOL bridgeId = FALSE;
    sprintf
    (
        mac_id,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        eth_dev->eth_devMacAddress[0],
        eth_dev->eth_devMacAddress[1],
        eth_dev->eth_devMacAddress[2],
        eth_dev->eth_devMacAddress[3],
        eth_dev->eth_devMacAddress[4],
        eth_dev->eth_devMacAddress[5]
    );

        CcspTraceWarning(("<EthCB> mac:%s stat:%s\n",
										mac_id,
										(eth_dev->eth_Active) ? "Connected" : "Disconnected" ));

	    AnscCopyMemory(Eth_Host.eth_macAddr,eth_dev->eth_devMacAddress,6);
        Eth_Host.eth_Active = eth_dev->eth_Active;
	    Eth_Host.eth_port = eth_dev->eth_port;		
		if((ANSC_STATUS_SUCCESS == is_usg_in_bridge_mode(&bridgeId)) && (FALSE == bridgeId))
	    	Notify_To_LMLite(&Eth_Host);
}

ANSC_STATUS
CosaDmlEthWanGetCfg
    (
        PCOSA_DATAMODEL_ETH_WAN_AGENT  pMyObject
    )
{
	memset( pMyObject,  0,  sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ) );

#if (defined (_COSA_BCM_ARM_) && !defined(_CBR_PRODUCT_REQ_))
	CcspHalExtSw_getEthWanEnable( &pMyObject->Enable );	
	CcspHalExtSw_getEthWanPort( &pMyObject->Port );
#endif /*  _COSA_BCM_ARM_   &&   _CBR_PRODUCT_REQ_*/


    return ANSC_STATUS_SUCCESS;
}

void CosaDmlEthWanChangeHandling( void* buff )
{
    CCSP_MESSAGE_BUS_INFO *bus_info 		  = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
	parameterValStruct_t param_val[ 1 ] 	  = { "Device.X_CISCO_COM_DeviceControl.RebootDevice", "Device", ccsp_string };
	char 				 pComponentName[ 64 ] = "eRT.com.cisco.spvtg.ccsp.pam";
	char 				 pComponentPath[ 64 ] = "/com/cisco/spvtg/ccsp/pam";
	char				*faultParam 		  = NULL;
    int   				 ret             	  = 0;

	pthread_detach(pthread_self());	

/* Set the reboot reason */
                        char buf[8];
                        snprintf(buf,sizeof(buf),"%d",1);

                        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_LastRebootReason", "WAN_Mode_Change") != 0)
                        {
                                AnscTraceWarning(("RDKB_REBOOT : RebootDevice syscfg_set failed GUI\n"));
                        }
                        else
                        {
                                if (syscfg_commit() != 0)
                                {
                                        AnscTraceWarning(("RDKB_REBOOT : RebootDevice syscfg_commit failed for ETHWAN mode\n"));
                                }
                        }


                        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_LastRebootCounter", buf) != 0)
                        {
                                AnscTraceWarning(("syscfg_set failed\n"));
                        }
                        else
                        {
                                if (syscfg_commit() != 0)
                                {
                                        AnscTraceWarning(("syscfg_commit failed\n"));
                                }
                        }

	/* Need to do reboot the device here */
    ret = CcspBaseIf_setParameterValues
			(
				bus_handle, 
				pComponentName, 
				pComponentPath,
				0, 
				0x0,   /* session id and write id */
				&param_val, 
                1, 
				TRUE,   /* Commit  */
				&faultParam
			);	

	if ( ( ret != CCSP_SUCCESS ) && \
		 ( faultParam )
		)
	{
	    AnscTraceWarning(("%s Failed to SetValue for param '%s'\n",__FUNCTION__,faultParam ) );
	    bus_info->freefunc( faultParam );
	} 
}

ANSC_STATUS
CosaDmlEthWanSetEnable
    (
        BOOL                       bEnable
    )
{
#if (defined (_COSA_BCM_ARM_) && !defined(_CBR_PRODUCT_REQ_) && !defined(_PLATFORM_RASPBERRYPI_))
        BOOL bGetStatus = FALSE;
        CcspHalExtSw_getEthWanEnable(&bGetStatus);
	if (bEnable != bGetStatus)
	{
	   if(bEnable == FALSE)
	   {
		system("ifconfig erouter0 down");
#ifdef _XB7_PRODUCT_REQ_        
		system("ip link set erouter0 name eth3");
#else
		system("ip link set erouter0 name eth0");
#endif
		system("ip link set dummy-rf name erouter0");
		system("ifconfig eth0 up;ifconfig erouter0 up");
		
	   } 
	}
#ifdef _XB7_PRODUCT_REQ_
			CcspHalExtSw_setEthWanPort ( 3 ); // need to set it to 3 eth3 interface for XB7
#else
			CcspHalExtSw_setEthWanPort ( 0 ); // need to set it to 0 eth0 interface after  TCXB6-4234 getting fixed
#endif
	if ( ANSC_STATUS_SUCCESS == CcspHalExtSw_setEthWanEnable( bEnable ) ) 
	{
	 	pthread_t tid;		
		char buf[ 8 ];
		memset( buf, 0, sizeof( buf ) );
		snprintf( buf, sizeof( buf ), "%s", bEnable ? "true" : "false" );
		if(bEnable)
		{
			system("touch /nvram/ETHWAN_ENABLE");
		}
		else
		{
			system("rm /nvram/ETHWAN_ENABLE");
		}

		if ( syscfg_set( NULL, "eth_wan_enabled", buf ) != 0 )
		{
			AnscTraceWarning(( "syscfg_set failed for eth_wan_enabled\n" ));
			return ANSC_STATUS_FAILURE;
		}
		else
		{
			if ( syscfg_commit() != 0 )
			{
				AnscTraceWarning(( "syscfg_commit failed for eth_wan_enabled\n" ));
				return ANSC_STATUS_FAILURE;
			}
			//CcspHalExtSw_setEthWanPort ( 1 );
			pthread_create ( &tid, NULL, &CosaDmlEthWanChangeHandling, NULL );
		}
	}
    return ANSC_STATUS_SUCCESS;
#else
	return ANSC_STATUS_FAILURE;
#endif /* (defined (_COSA_BCM_ARM_) && !defined(_CBR_PRODUCT_REQ_)) */
}

ANSC_STATUS
CosaDmlEthGetLogStatus
    (
        PCOSA_DML_ETH_LOG_STATUS  pMyObject
    )
{
    char buf[16] = {0};

    pMyObject->Log_Enable = FALSE;
    pMyObject->Log_Period = 3600;

    if (syscfg_get(NULL, "eth_log_enabled", buf, sizeof(buf)) == 0)
    {
        if (buf != NULL)
        {
            pMyObject->Log_Enable = (strcmp(buf,"true") ? FALSE : TRUE);
        }
    }

    memset(buf, 0, sizeof(buf));
    if (syscfg_get( NULL, "eth_log_period", buf, sizeof(buf)) == 0)
    {
        if (buf != NULL)
        {
            pMyObject->Log_Period =  atoi(buf);
        }
    }

    return ANSC_STATUS_SUCCESS;
}

