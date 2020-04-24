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
#include "safec_lib_common.h"
#include "cap.h"
#include <linux/version.h>
#ifdef FEATURE_SUPPORT_ONBOARD_LOGGING
#include "cimplog.h"


#define LOGGING_MODULE           "ETHAGENT"
#define OnboardLog(...)          onboarding_log(LOGGING_MODULE, __VA_ARGS__)
#else
#define OnboardLog(...)
#endif
/**************************************************************************
                        DATA STRUCTURE DEFINITIONS
**************************************************************************/

/**************************************************************************
                        GLOBAL VARIABLES
**************************************************************************/
#if defined (ENABLE_ETH_WAN)
/* ETH WAN Fallback Interface Name - Should eventually move away from Compile Time */
#if defined (_XB7_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_)
#define ETHWAN_DEF_INTF_NAME "eth3"
#elif defined (INTEL_PUMA7)
#define ETHWAN_DEF_INTF_NAME "nsgmii0"
#else
#define ETHWAN_DEF_INTF_NAME "eth0"
#endif

/* ETH WAN Physical Interface Number Assignment - Should eventually move away from Compile Time */
/* ETh WAN HAL is 0 based */
#if defined (_2_5G_ETHERNET_SUPPORT_)
#if defined (ETH_6_PORTS)
#define ETHWAN_DEF_INTF_NUM 5
#elif defined (ETH_4_PORTS)
#define ETHWAN_DEF_INTF_NUM 3
#endif
#else
/* As Of Now All 1Gbps Devices Use Physical Port #1 for ETH WAN */
#define ETHWAN_DEF_INTF_NUM 0
#endif

#endif //#if defined (ENABLE_ETH_WAN)


extern  ANSC_HANDLE bus_handle;
extern  ANSC_HANDLE g_EthObject;
extern void* g_pDslhDmlAgent;
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))
extern cap_user appcaps;
static bool bNonrootEnabled = false;
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


#define ETH_LOGVALUE_FILE "/tmp/eth_telemetry_xOpsLogSettings.txt"

void CosaEthTelemetryxOpsLogSettingsSync()
{
    FILE *fp = fopen(ETH_LOGVALUE_FILE, "w");
    if (fp != NULL) {
	char buff[64] = {0};     
	syscfg_get(NULL, "eth_log_period", buff, 32);
	syscfg_get(NULL,"eth_log_enabled", &buff[32], 32);
	fprintf(fp,"%s,%s\n", &buff[0], &buff[32]);
	fclose(fp);
    }
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
     errno_t                         rc           = -1;
     rc =  memset_s( pMyObject,sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ),  0,  sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ) );
     ERR_CHK(rc);

#if defined (ENABLE_ETH_WAN)
	CcspHalExtSw_getEthWanEnable( &pMyObject->Enable );	
	CcspHalExtSw_getEthWanPort( &pMyObject->Port );
#endif /*  ENABLE_ETH_WAN */


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
			OnboardLog("Device reboot due to reason WAN_Mode_Change\n");
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
#if defined (ENABLE_ETH_WAN)
        BOOL bGetStatus = FALSE;
        CcspHalExtSw_getEthWanEnable(&bGetStatus);
        char command[50] = {0};
        errno_t rc = -1;
	if (bEnable != bGetStatus)
	{
	   if(bEnable == FALSE)
	   {
		system("ifconfig erouter0 down");
		// NOTE: Eventually ETHWAN_DEF_INTF_NAME should be replaced with GWP_GetEthWanInterfaceName()
		sprintf(command, "ip link set erouter0 name %s", ETHWAN_DEF_INTF_NAME);
		CcspTraceWarning(("****************value of command = %s**********************\n", command));
		system(command);
		system("ip link set dummy-rf name erouter0");
		rc =  memset_s(command,sizeof(command),0,sizeof(command));
                ERR_CHK(rc);
		sprintf(command, "ifconfig %s up;ifconfig erouter0 up", ETHWAN_DEF_INTF_NAME);
		CcspTraceWarning(("****************value of command = %s**********************\n", command));
		system(command);
		
	   } 
	}

	CcspHalExtSw_setEthWanPort ( ETHWAN_DEF_INTF_NUM );

	if ( ANSC_STATUS_SUCCESS == CcspHalExtSw_setEthWanEnable( bEnable ) ) 
	{
	 	pthread_t tid;		
		char buf[ 8 ] = {0};
		snprintf( buf, sizeof( buf ), "%s", bEnable ? "true" : "false" );

                /* Linux version < 0 , switch to root */
                if( (LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)) && (getuid() != 0))  { 
		   AnscTraceWarning(( "Linux version < 4.3 and CcspEthAgent is running as non-root user\n" ));
                   gain_root_privilege();
                   bNonrootEnabled = true;
                }
		if(bEnable)
		{
			system("touch /nvram/ETHWAN_ENABLE");
		}
		else
		{
			system("rm /nvram/ETHWAN_ENABLE");
		}
                if(bNonrootEnabled){
		    init_capability();
		    drop_root_caps(&appcaps);
		    update_process_caps(&appcaps);
		    bNonrootEnabled = false;
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
#endif /* defined (ENABLE_ETH_WAN) */
}

ANSC_STATUS
CosaDmlEthGetLogStatus
    (
        PCOSA_DML_ETH_LOG_STATUS  pMyObject
    )
{
    char buf[16] = {0};
    errno_t rc = -1;
    int ind = -1;
    pMyObject->Log_Enable = FALSE;
    pMyObject->Log_Period = 3600;

    if (syscfg_get(NULL, "eth_log_enabled", buf, sizeof(buf)) == 0)
    {
        if (buf != NULL)
        {
            rc = strcmp_s("true",strlen("true"),buf,&ind);
            ERR_CHK(rc);
            pMyObject->Log_Enable = (ind) ? FALSE : TRUE;
            
        }
    }

     rc = memset_s(buf,sizeof(buf), 0, sizeof(buf));
     ERR_CHK(rc);
    if (syscfg_get( NULL, "eth_log_period", buf, sizeof(buf)) == 0)
    {
        if (buf != NULL)
        {
            pMyObject->Log_Period =  atoi(buf);
        }
    }

    return ANSC_STATUS_SUCCESS;
}

