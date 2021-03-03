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
#include <errno.h>
#include <mqueue.h>

#include "cosa_ethernet_apis.h"
#include "safec_lib_common.h"
#include "cap.h"
#include <linux/version.h>
#include "cosa_ethernet_internal.h"
#include "ccsp_hal_ethsw.h"

#if defined (FEATURE_RDKB_WAN_MANAGER)
#include "cosa_ethernet_manager.h"
#endif //FEATURE_RDKB_WAN_MANAGER

#include "syscfg.h"
#ifdef FEATURE_SUPPORT_ONBOARD_LOGGING
#include "cimplog.h"


#define LOGGING_MODULE           "ETHAGENT"
#define OnboardLog(...)          onboarding_log(LOGGING_MODULE, __VA_ARGS__)
#else
#define OnboardLog(...)
#endif

#if defined (FEATURE_RDKB_WAN_AGENT) || defined(FEATURE_RDKB_WAN_MANAGER)
#include "cosa_ethernet_manager.h"
#if defined (FEATURE_RDKB_WAN_MANAGER)
#define TOTAL_NUMBER_OF_INTERNAL_INTERFACES 4
#define DATAMODEL_PARAM_LENGTH 256
#include "eth_hal.h"
#endif //FEATURE_RDKB_WAN_MANAGER
#define TOTAL_NUMBER_OF_INTERFACES 4
#define COSA_ETH_EVENT_QUEUE_NAME "/ETH_event_queue"
#define MAX_QUEUE_MSG_SIZE (512) 
#define MAX_QUEUE_LENGTH (100)
#define EVENT_MSG_MAX_SIZE (256)
#define CHECK(x)                                                 \
    do                                                           \
    {                                                            \
        if (!(x))                                                \
        {                                                        \
            CcspTraceError(("%s:%d: ", __FUNCTION__, __LINE__)); \
            perror(#x);                                          \
            return;                                              \
        }                                                        \
    } while (0)
#endif //FEATURE_RDKB_WAN_AGENT || defined(FEATURE_RDKB_WAN_MANAGER)

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
#elif defined (_CBR2_PRODUCT_REQ_)
#define ETHWAN_DEF_INTF_NAME "eth5"
#elif defined (INTEL_PUMA7)
#define ETHWAN_DEF_INTF_NAME "nsgmii0"
#else
#define ETHWAN_DEF_INTF_NAME "eth0"
#endif

#endif //#if defined (ENABLE_ETH_WAN)

#define ETH_HOST_PARAMVALUE_TRUE "true"
#define ETH_HOST_PARAMVALUE_FALSE "false"
#define ETH_HOST_MAC_LENGTH 17
ANSC_STATUS is_usg_in_bridge_mode(BOOL *pBridgeMode);
void CcspHalExtSw_SendNotificationForAllHosts( void );
extern  ANSC_HANDLE bus_handle;
extern  ANSC_HANDLE g_EthObject;
extern void* g_pDslhDmlAgent;
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))
extern cap_user appcaps;
#if defined (ENABLE_ETH_WAN)
    static bool bNonrootEnabled = false;
#endif

#if defined (FEATURE_RDKB_WAN_AGENT) || defined (FEATURE_RDKB_WAN_MANAGER)
typedef enum _COSA_ETH_MSGQ_MSG_TYPE
{
    MSG_TYPE_WAN = 1,

} COSA_ETH_MSGQ_MSG_TYPE;

typedef struct _CosaEthEventQData
{
    char Msg[EVENT_MSG_MAX_SIZE];   //Msg structure for the specific event
    COSA_ETH_MSGQ_MSG_TYPE MsgType; // WAN = 1
} CosaEthEventQData;

typedef struct _CosaETHMSGQWanData
{
    CHAR Name[64];
    COSA_DML_ETH_LINK_STATUS LinkStatus;
} CosaETHMSGQWanData;


PCOSA_DML_ETH_PORT_GLOBAL_CONFIG gpstEthGInfo = NULL;
static pthread_mutex_t gmEthGInfo_mutex = PTHREAD_MUTEX_INITIALIZER;
static ANSC_STATUS CosDmlEthPortPrepareGlobalInfo();
static ANSC_STATUS CosaDmlEthGetParamValues(char *pComponent, char *pBus, char *pParamName, char *pReturnVal);
static ANSC_STATUS CosaDmlEthGetParamNames(char *pComponent, char *pBus, char *pParamName, char a2cReturnVal[][256], int *pReturnSize);
static ANSC_STATUS CosaDmlEthPortSendLinkStatusToEventQueue(CosaETHMSGQWanData *MSGQWanData);
static ANSC_STATUS CosaDmlEthGetLowerLayersInstanceInOtherAgent(COSA_ETH_NOTIFY_ENUM enNotifyAgent, char *pLowerLayers, INT *piInstanceNumber);
static ANSC_STATUS CosaDmlGetWanOEInterfaceName(char *pInterface, unsigned int length);
static void CosaDmlEthTriggerEventHandlerThread(void);
static void *CosaDmlEthEventHandlerThread(void *arg);
static ANSC_STATUS CosaDmlEthPortGetIndexFromIfName( char *ifname, INT *IfIndex );
static INT CosaDmlEthGetTotalNoOfInterfaces ( VOID );
#endif //#if defined (FEATURE_RDKB_WAN_AGENT) || defined (FEATURE_RDKB_WAN_MANAGER)

#if defined (FEATURE_RDKB_WAN_AGENT)
static ANSC_STATUS CosaDmlEthSetParamValues(char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, unsigned int bCommitFlag);
#elif defined (FEATURE_RDKB_WAN_MANAGER)
static ANSC_STATUS CosaDmlEthSetParamValues(const char *pComponent, const char *pBus, const char *pParamName, const char *pParamVal, enum dataType_e type, unsigned int bCommitFlag);
INT gTotal = TOTAL_NUMBER_OF_INTERNAL_INTERFACES;
#endif

void Notify_To_LMLite(Eth_host_t *host)
{
    parameterValStruct_t notif_val[1];
    char compo[256] = "eRT.com.cisco.spvtg.ccsp.lmlite"; 
    char bus[256] = "/com/cisco/spvtg/ccsp/lmlite";
    char param_name[256] = "Device.Hosts.X_RDKCENTRAL-COM_EthHost_Sync";
    char param_value[25] = {0};
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

    if(host->eth_Active)
    {
       /*Coverity Fix CID:67001 DC.STRING_BUFFER */
        int ret_paramval;
        ret_paramval = snprintf(param_value + ETH_HOST_MAC_LENGTH, sizeof(param_value) - ETH_HOST_MAC_LENGTH, ",%s", ETH_HOST_PARAMVALUE_TRUE);
        if ((ret_paramval < 0) || (ret_paramval >= (int)sizeof(param_value)))
        {
            CcspTraceWarning(("%s : FAILED due to error on snprintf return value: %d in true statement ret: %d \n", __FUNCTION__, ret_paramval, ret));
            return;
        }
    }
    else
    {
       /*Coverity Fix CID:67001 DC.STRING_BUFFER */
        int ret_paramval;
        ret_paramval = snprintf(param_value + ETH_HOST_MAC_LENGTH, sizeof(param_value) - ETH_HOST_MAC_LENGTH, ",%s", ETH_HOST_PARAMVALUE_FALSE);
        if ((ret_paramval < 0) || (ret_paramval >= (int)sizeof(param_value)))
        {
            CcspTraceWarning(("%s : FAILED due to error on snprintf return value: %d in false statement ret: %d \n", __FUNCTION__, ret_paramval, ret));
            return;
        }
    }

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
	char log_period[32] = {0};
	char log_enable[32] = {0};
	memset(log_period,0,sizeof(log_period));
	memset(log_enable,0,sizeof(log_enable));
	if(syscfg_get(NULL, "eth_log_period", log_period, sizeof(log_period))!= 0 || (log_period[0] == '\0'))
	{
		CcspTraceWarning(("eth_log_period syscfg_get failed\n"));
		sprintf(log_period,"3600");
	}
	if(syscfg_get(NULL,"eth_log_enabled", log_enable, sizeof(log_enable)) != 0 || (log_enable[0] == '\0'))
	{
		CcspTraceWarning(("eth_log_enabled syscfg_get failed\n"));
		sprintf(log_enable,"false");
	}
	fprintf(fp,"%s,%s\n", log_period, log_enable);
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
    /*Coverity  Fix  CID:60418 MISSING_RETURN */
    return ANSC_STATUS_SUCCESS;
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
	CcspHalExtSw_getEthWanPort((UINT *) &pMyObject->Port );
#endif /*  ENABLE_ETH_WAN */


    return ANSC_STATUS_SUCCESS;
}

void* CosaDmlEthWanChangeHandling( void* buff )
{
    CCSP_MESSAGE_BUS_INFO *bus_info 		  = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
	parameterValStruct_t param_val[ 1 ] 	  = {{ "Device.X_CISCO_COM_DeviceControl.RebootDevice", "Device", ccsp_string }};
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
				param_val, 
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
    return buff;
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
                /*Coverity Fix  CID: 132495 DC.STRING_BUFFER*/
		snprintf(command,sizeof(command), "ifconfig %s up;ifconfig erouter0 up", ETHWAN_DEF_INTF_NAME);
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
    UNREFERENCED_PARAMETER(bEnable);
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

ANSC_STATUS
CosaDmlEthInit(
    ANSC_HANDLE hDml,
    PANSC_HANDLE phContext)
{
    UNREFERENCED_PARAMETER(hDml);
#if defined (FEATURE_RDKB_WAN_AGENT) || defined (FEATURE_RDKB_WAN_MANAGER)
    char PhyStatus[16] = {0};
    char WanOEInterface[16] = {0};
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)phContext;
    int ifIndex;
#if defined (FEATURE_RDKB_WAN_AGENT)
    //Initialise ethsw-hal to get event notification from lower layer.
    if (CcspHalEthSwInit() != RETURN_OK)
    {
        CcspTraceError(("Hal initialization failed \n"));
        return ANSC_STATUS_FAILURE;
    }
    //ETH Port Init.
    CosaDmlEthPortInit((PANSC_HANDLE)pMyObject);

    if(CosaDmlGetWanOEInterfaceName(WanOEInterface, sizeof(WanOEInterface)) == ANSC_STATUS_SUCCESS) {
        if(GWP_GetEthWanLinkStatus() == 1) {
            if(CosaDmlEthGetPhyStatusForWanAgent(WanOEInterface, PhyStatus) == ANSC_STATUS_SUCCESS) {
                if(strcmp(PhyStatus, "Up") != 0) {
                    CosaDmlEthSetPhyStatusForWanAgent(WanOEInterface, "Up");
                    CcspTraceError(("Successfully updated PhyStatus to UP for %s interface \n", WanOEInterface));
                    /** We need also update `linkStatus` in global data to inform linkstatus is up
                     * for the EthAgent state machine. This is required for SM, when its being started
                     * by setting `upstream` from WanAgent.
                     */
                    if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetIndexFromIfName(WanOEInterface, &ifIndex))
                    {
                        CcspTraceError(("%s Successfully get index for this %s interface\n", __FUNCTION__, WanOEInterface));
                        pthread_mutex_lock(&gmEthGInfo_mutex);
                        gpstEthGInfo[ifIndex].LinkStatus = ETH_LINK_STATUS_UP;
                        pthread_mutex_unlock(&gmEthGInfo_mutex);
                    }
                }
            }      
        }
    }
#elif defined (FEATURE_RDKB_WAN_MANAGER)
     //Initialise eth-hal to get event notification from lower layer.
    if (eth_hal_init() != RETURN_OK)
    {
        CcspTraceError(("Hal initialization failed \n"));
        return ANSC_STATUS_FAILURE;
    }
    //ETH Port Init.
    CosaDmlEthPortInit((PANSC_HANDLE)pMyObject);
    if(CosaDmlGetWanOEInterfaceName(WanOEInterface, sizeof(WanOEInterface)) == ANSC_STATUS_SUCCESS) {
        if(GWP_GetEthWanLinkStatus() == 1) {
            if(CosaDmlEthGetPhyStatusForWanManager(WanOEInterface, PhyStatus) == ANSC_STATUS_SUCCESS) {
                if(strcmp(PhyStatus, "Up") != 0) {
                    CosaDmlEthSetPhyStatusForWanManager(WanOEInterface, "Up");
                    CcspTraceError(("Successfully updated PhyStatus to UP for %s interface \n", WanOEInterface));
                    /** We need also update `linkStatus` in global data to inform linkstatus is up
                     * for the EthAgent state machine. This is required for SM, when its being started
                     * by setting `upstream` from WanManager.
                     */
                    if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetIndexFromIfName(WanOEInterface, &ifIndex))
                    {
                        CcspTraceError(("%s Successfully get index for this %s interface\n", __FUNCTION__, WanOEInterface));
                        pthread_mutex_lock(&gmEthGInfo_mutex);
                        gpstEthGInfo[ifIndex].LinkStatus = ETH_LINK_STATUS_UP;
                        pthread_mutex_unlock(&gmEthGInfo_mutex);
                    }
                }
            }
        }
    }
#endif

    /* Trigger Event Handler thread.
     * Monitor for the link status event and notify the other agents,
     */
    CosaDmlEthTriggerEventHandlerThread();
#else
    UNREFERENCED_PARAMETER(phContext);
#endif //#if defined (FEATURE_RDKB_WAN_AGENT) || FEATURE_RDKB_WAN_MANAGER
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlEthPortInit(
    PANSC_HANDLE phContext)
{
#if defined (FEATURE_RDKB_WAN_AGENT)
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)phContext;
    PCOSA_DML_ETH_PORT_CONFIG pETHlinkTemp = NULL;
    INT iTotalInterfaces = 0;
    INT iLoopCount = 0;

    iTotalInterfaces = CosaDmlEthGetTotalNoOfInterfaces();
    pETHlinkTemp = (PCOSA_DML_ETH_PORT_CONFIG)AnscAllocateMemory(sizeof(COSA_DML_ETH_PORT_CONFIG) * iTotalInterfaces);

    if (NULL == pETHlinkTemp)
    {
        CcspTraceError(("Failed to allocate memeory \n"));
        return ANSC_STATUS_FAILURE;
    }

    pMyObject->ulTotalNoofEthInterfaces = iTotalInterfaces;
    memset(pETHlinkTemp, 0, sizeof(COSA_DML_ETH_PORT_CONFIG) * iTotalInterfaces);

    //Fill line static information and initialize default values
    for (iLoopCount = 0; iLoopCount < iTotalInterfaces; iLoopCount++)
    {
        pETHlinkTemp[iLoopCount].WanStatus = ETH_WAN_DOWN;
        pETHlinkTemp[iLoopCount].LinkStatus = ETH_LINK_STATUS_DOWN;
        pETHlinkTemp[iLoopCount].ulInstanceNumber = iLoopCount + 1;
        // Get  Name.
        snprintf(pETHlinkTemp[iLoopCount].Name, sizeof(pETHlinkTemp[iLoopCount].Name), "eth%d", iLoopCount);
        snprintf(pETHlinkTemp[iLoopCount].Path, sizeof(pETHlinkTemp[iLoopCount].Path), "%s%d", ETHERNET_IF_PATH, iLoopCount + 1);
        pETHlinkTemp[iLoopCount].Upstream = FALSE;
        pETHlinkTemp[iLoopCount].WanValidated = FALSE;
    }

    //Assign the memory address to oringinal structure
    pMyObject->pEthLink = pETHlinkTemp;
    //Prepare global information.
    CosDmlEthPortPrepareGlobalInfo();
#elif defined (FEATURE_RDKB_WAN_MANAGER)
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)phContext;
    PCOSA_DML_ETH_PORT_CONFIG pETHTemp = NULL;
    PCOSA_CONTEXT_LINK_OBJECT pEthCxtLink    = NULL;
    INT iTotalInterfaces = 0;
    INT iLoopCount = 0;
    iTotalInterfaces = CosaDmlEthGetTotalNoOfInterfaces();
    pMyObject->ulTotalNoofEthInterfaces = iTotalInterfaces;
    for (iLoopCount = 0; iLoopCount < iTotalInterfaces; iLoopCount++)
    {
        pETHTemp = (PCOSA_DML_ETH_PORT_CONFIG)AnscAllocateMemory(sizeof(COSA_DML_ETH_PORT_CONFIG));
        if (NULL == pETHTemp)
        {
            CcspTraceError(("Failed to allocate memory \n"));
            return ANSC_STATUS_FAILURE;
        }
        pEthCxtLink = (PCOSA_CONTEXT_LINK_OBJECT)AnscAllocateMemory(sizeof(COSA_CONTEXT_LINK_OBJECT));
        if ( !pEthCxtLink )
        {
            CcspTraceError(("pEthCxtLink Failed to allocate memory \n"));
            return ANSC_STATUS_FAILURE;
        }
        memset(pETHTemp, 0, sizeof(COSA_DML_ETH_PORT_CONFIG));
        //Fill line static information and initialize default values
        DML_ETHIF_INIT(pETHTemp);
        pETHTemp->ulInstanceNumber = iLoopCount + 1;
        pMyObject->ulPtNextInstanceNumber=  pETHTemp->ulInstanceNumber + 1;
        // Get  Name.
        snprintf(pETHTemp->Name, sizeof(pETHTemp->Name), "eth%d", iLoopCount);
        snprintf(pETHTemp->LowerLayers, sizeof(pETHTemp->LowerLayers), "%s%d", ETHERNET_IF_LOWERLAYERS, iLoopCount + 1);
        pEthCxtLink->hContext = (ANSC_HANDLE)pETHTemp;
        pEthCxtLink->bNew     = TRUE;
        pEthCxtLink->InstanceNumber = pETHTemp->ulInstanceNumber ;
        CosaSListPushEntryByInsNum(&pMyObject->Q_EthList, (PCOSA_CONTEXT_LINK_OBJECT)pEthCxtLink);
   }
    //Prepare global information.
    CosDmlEthPortPrepareGlobalInfo();
#else
    UNREFERENCED_PARAMETER(phContext);
#endif //#if defined (FEATURE_RDKB_WAN_AGENT)
    return ANSC_STATUS_SUCCESS;
}
#if defined (FEATURE_RDKB_WAN_MANAGER)
ANSC_STATUS CosDmlEthPortUpdateGlobalInfo(PANSC_HANDLE phContext, char *ifname, COSA_DML_ETH_TABLE_OPER Oper )
{
    INT iTotal = 0;
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)phContext;
    if (ifname == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    /* Add new Entry - gpstEthInfo */
    if(Oper == ETH_ADD_TABLE)
    {
        pMyObject->ulTotalNoofEthInterfaces++;
        iTotal=pMyObject->ulTotalNoofEthInterfaces;
        INT newIndex = iTotal - 1;
        pthread_mutex_lock(&gmEthGInfo_mutex);
        gpstEthGInfo = (PCOSA_DML_ETH_PORT_GLOBAL_CONFIG)AnscReAllocateMemory(gpstEthGInfo, sizeof(COSA_DML_ETH_PORT_GLOBAL_CONFIG) * iTotal);
        //Return failure if allocation failiure
        if (NULL == gpstEthGInfo)
        {
            CcspTraceError(("%s AnscReallocateMemory Failed for adding ifname[%s]\n", __FUNCTION__,ifname));
            pthread_mutex_unlock(&gmEthGInfo_mutex);
            return ANSC_STATUS_FAILURE;
        }
        gTotal++;  // Increment gTotal
        /* Populate Default Values */
        gpstEthGInfo[newIndex].Upstream = FALSE;
        gpstEthGInfo[newIndex].WanStatus = ETH_WAN_DOWN;
        gpstEthGInfo[newIndex].LinkStatus = ETH_LINK_STATUS_DOWN;
        gpstEthGInfo[newIndex].WanValidated = TRUE; //Make default as True.
        gpstEthGInfo[newIndex].Enable = FALSE; //Make default as False.
        snprintf(gpstEthGInfo[newIndex].Name, sizeof(gpstEthGInfo[newIndex].Name), "eth%d", newIndex);
        snprintf(gpstEthGInfo[newIndex].Path, sizeof(gpstEthGInfo[newIndex].Path), "%s%d", ETHERNET_IF_PATH, newIndex + 1 );
        snprintf(gpstEthGInfo[newIndex].LowerLayers, sizeof(gpstEthGInfo[newIndex].LowerLayers), "%s%d", ETHERNET_IF_LOWERLAYERS, newIndex + 1 );
        pthread_mutex_unlock(&gmEthGInfo_mutex);
    }
    else if(Oper == ETH_DEL_TABLE )
    {
        /* Delete an Entry from gpstEthInfo */
        ANSC_STATUS   retStatus;
        INT           IfIndex = -1;
        retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &IfIndex);
        if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == IfIndex ) )
        {
            CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
            return ANSC_STATUS_FAILURE;
        }
        if ( IfIndex < TOTAL_NUMBER_OF_INTERNAL_INTERFACES )
        {
            CcspTraceError(("%s Default Eth Port Cant be deleted %s\n", __FUNCTION__,ifname));
            return ANSC_STATUS_FAILURE;
        }
        if((IfIndex + 1) == (INT)pMyObject->ulTotalNoofEthInterfaces)
        {
            /* Delete Last Entry in gpstEthGInfo */
            pMyObject->ulTotalNoofEthInterfaces--;
            iTotal = pMyObject->ulTotalNoofEthInterfaces;
            pthread_mutex_lock(&gmEthGInfo_mutex);
            gpstEthGInfo = (PCOSA_DML_ETH_PORT_GLOBAL_CONFIG)AnscReAllocateMemory(gpstEthGInfo, sizeof(COSA_DML_ETH_PORT_GLOBAL_CONFIG) * iTotal);
            //Return failure if allocation failiure
            if (NULL == gpstEthGInfo)
            {
                CcspTraceError(("%s AnscReallocateMemory Failed for deleting (last entry) ifname[%s]\n", __FUNCTION__,ifname));
                pthread_mutex_unlock(&gmEthGInfo_mutex);
                return ANSC_STATUS_FAILURE;
            }
            gTotal--;
            pthread_mutex_unlock(&gmEthGInfo_mutex);
        }
        else
        {
            /* Delete Entry which is in middle of gpstEthGInfo Array*/
            INT           lastIndex = pMyObject->ulTotalNoofEthInterfaces -1;
            COSA_DML_ETH_PORT_GLOBAL_CONFIG tmpEthGInfo;
            pMyObject->ulTotalNoofEthInterfaces--;
            iTotal = pMyObject->ulTotalNoofEthInterfaces;
            pthread_mutex_lock(&gmEthGInfo_mutex);
            /* Take a backup - last Entry */
            tmpEthGInfo.Upstream = gpstEthGInfo[lastIndex].Upstream;
            tmpEthGInfo.WanStatus = gpstEthGInfo[lastIndex].WanStatus;
            tmpEthGInfo.LinkStatus = gpstEthGInfo[lastIndex].LinkStatus;
            tmpEthGInfo.WanValidated = gpstEthGInfo[lastIndex].WanValidated;
            tmpEthGInfo.Enable = gpstEthGInfo[lastIndex].Enable;
            strncpy(tmpEthGInfo.Name, gpstEthGInfo[lastIndex].Name, sizeof(gpstEthGInfo[lastIndex].Name));
            strncpy(tmpEthGInfo.Path, gpstEthGInfo[lastIndex].Path, sizeof(gpstEthGInfo[lastIndex].Path));
            strncpy(tmpEthGInfo.LowerLayers, gpstEthGInfo[lastIndex].LowerLayers, sizeof(gpstEthGInfo[lastIndex].LowerLayers));
            /* Remove last Entry */
            gpstEthGInfo = (PCOSA_DML_ETH_PORT_GLOBAL_CONFIG)AnscReAllocateMemory(gpstEthGInfo, sizeof(COSA_DML_ETH_PORT_GLOBAL_CONFIG) * iTotal);
            //Return failure if allocation failiure
            if (NULL == gpstEthGInfo)
            {
                CcspTraceError(("%s AnscReallocateMemory Failed for deleting ifname[%s]\n", __FUNCTION__,ifname));
                pthread_mutex_unlock(&gmEthGInfo_mutex);
                return ANSC_STATUS_FAILURE;
            }
            /* Copy last Entry into new place */
            gpstEthGInfo[IfIndex].Upstream = tmpEthGInfo.Upstream;
            gpstEthGInfo[IfIndex].WanStatus = tmpEthGInfo.WanStatus;
            gpstEthGInfo[IfIndex].LinkStatus = tmpEthGInfo.LinkStatus;
            gpstEthGInfo[IfIndex].WanValidated = tmpEthGInfo.WanValidated;
            gpstEthGInfo[IfIndex].Enable = tmpEthGInfo.Enable;
            strncpy(gpstEthGInfo[IfIndex].Name, tmpEthGInfo.Name, sizeof(tmpEthGInfo.Name));
            strncpy(gpstEthGInfo[IfIndex].Path, tmpEthGInfo.Path, sizeof(tmpEthGInfo.Path));
            strncpy(gpstEthGInfo[IfIndex].LowerLayers, tmpEthGInfo.LowerLayers, sizeof(tmpEthGInfo.LowerLayers));
            gTotal--; // Decrement
            pthread_mutex_unlock(&gmEthGInfo_mutex);
        }
    }
    CcspTraceError(("CosDmlEthPortUpdateGlobalInfo Success ifname[%s] Operation[%s] gTotal[%d]\n", ifname,
                     (Oper==ETH_ADD_TABLE)?"ETH_ADD_TABLE":"ETH_DEL_TABLE",gTotal));
    return ANSC_STATUS_SUCCESS;
}
ANSC_STATUS CosaDmlTriggerExternalEthPortLinkStatus(char *ifname, BOOL status)
{
    ANSC_STATUS   retStatus;
    INT           IfIndex = -1;
    CcspTraceInfo(("%s.%d Enter \n",__FUNCTION__,__LINE__));
    if (ifname == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &IfIndex);
    if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == IfIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }
    if (IfIndex < TOTAL_NUMBER_OF_INTERNAL_INTERFACES)
    {
        CcspTraceError(("%s Invalid or Default index[%d] cant be triggerred \n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }
    if (status == TRUE)
    {
        CosaDmlEthPortLinkStatusCallback(ifname,UP);
        CcspTraceInfo(("Successfully updated PhyStatus to Up for [%s] interface \n",ifname));
    }
    else
    {
        CosaDmlEthPortLinkStatusCallback(ifname,DOWN);
        CcspTraceInfo(("Successfully updated PhyStatus to Down for [%s] interface \n",ifname));
    }
    return ANSC_STATUS_SUCCESS;
}
#endif //FEATURE_RDKB_WAN_MANAGER

ANSC_STATUS CosaDmlEthGetPortCfg(INT nIndex, PCOSA_DML_ETH_PORT_CONFIG pEthLink)
{
#if defined (FEATURE_RDKB_WAN_AGENT)
    COSA_DML_ETH_LINK_STATUS linkstatus;
    COSA_DML_ETH_WAN_STATUS wan_status;

    if (pEthLink == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }

    //Get Link status.
    if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetLinkStatus(nIndex, &linkstatus))
    {
        pEthLink->LinkStatus = linkstatus;
    }

    if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetWanStatus(nIndex, &wan_status))
    {
        pEthLink->WanStatus = wan_status;
    }

    snprintf(pEthLink->Name, sizeof(pEthLink->Name), "eth%d", nIndex);
#else
    UNREFERENCED_PARAMETER(nIndex);
    UNREFERENCED_PARAMETER(pEthLink);
#endif //#if defined (FEATURE_RDKB_WAN_AGENT)
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS CosaDmlEthPortSetWanValidated(INT IfIndex, BOOL WanValidated)
{
    UNREFERENCED_PARAMETER(IfIndex); 
    UNREFERENCED_PARAMETER(WanValidated); 
    return ANSC_STATUS_SUCCESS;
}

#if defined (FEATURE_RDKB_WAN_AGENT)
ANSC_STATUS CosaDmlEthPortGetWanStatus(INT IfIndex, COSA_DML_ETH_WAN_STATUS *wan_status)
{
#ifdef FEATURE_RDKB_WAN_AGENT
    if (IfIndex < 0 || IfIndex > 4)
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }

    //Get WAN status.
    pthread_mutex_lock(&gmEthGInfo_mutex);
    *wan_status = gpstEthGInfo[IfIndex].WanStatus;
    pthread_mutex_unlock(&gmEthGInfo_mutex);
#else 
    UNREFERENCED_PARAMETER(IfIndex); 
    UNREFERENCED_PARAMETER(wan_status); 
#endif     
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS CosaDmlEthPortSetWanStatus(INT IfIndex, COSA_DML_ETH_WAN_STATUS wan_status)
{
#ifdef FEATURE_RDKB_WAN_AGENT	
    if (IfIndex < 0 || IfIndex > 4)
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }

    //Set WAN status.
    pthread_mutex_lock(&gmEthGInfo_mutex);
    gpstEthGInfo[IfIndex].WanStatus = wan_status;
    pthread_mutex_unlock(&gmEthGInfo_mutex);

#else
    UNREFERENCED_PARAMETER(IfIndex);
    UNREFERENCED_PARAMETER(wan_status);
#endif //#if defined (FEATURE_RDKB_WAN_AGENT)
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS CosaDmlEthPortGetLinkStatus(INT IfIndex, COSA_DML_ETH_LINK_STATUS *LinkStatus)
{
#ifdef FEATURE_RDKB_WAN_AGENT	
    if (IfIndex < 0 || IfIndex > 4)
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }

    pthread_mutex_lock(&gmEthGInfo_mutex);
    *LinkStatus = gpstEthGInfo[IfIndex].LinkStatus;
    pthread_mutex_unlock(&gmEthGInfo_mutex);
#else 
    UNREFERENCED_PARAMETER(LinkStatus); 
    UNREFERENCED_PARAMETER(IfIndex); 
#endif //#if defined (FEATURE_RDKB_WAN_MANAGER)     
    return ANSC_STATUS_SUCCESS;
}
#elif defined (FEATURE_RDKB_WAN_MANAGER)
ANSC_STATUS CosaDmlEthPortGetWanStatus(char *ifname, COSA_DML_ETH_WAN_STATUS *wan_status)
{
    ANSC_STATUS   retStatus;
    INT           IfIndex = -1;
    if (ifname == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &IfIndex);
    if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == IfIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }
    if (IfIndex < 0 )
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }
    //Get WAN status.
    pthread_mutex_lock(&gmEthGInfo_mutex);
    *wan_status = gpstEthGInfo[IfIndex].WanStatus;
    pthread_mutex_unlock(&gmEthGInfo_mutex);
    return ANSC_STATUS_SUCCESS;
}
ANSC_STATUS CosaDmlEthPortSetWanStatus(char *ifname, COSA_DML_ETH_WAN_STATUS wan_status)
{
    ANSC_STATUS   retStatus;
    INT           IfIndex = -1;
    if (ifname == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &IfIndex);
    if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == IfIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }
    if (IfIndex < 0)
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }
    //Set WAN status.
    pthread_mutex_lock(&gmEthGInfo_mutex);
    gpstEthGInfo[IfIndex].WanStatus = wan_status;
    pthread_mutex_unlock(&gmEthGInfo_mutex);
    return ANSC_STATUS_SUCCESS;
}
ANSC_STATUS CosaDmlEthPortSetLowerLayers(char *ifname, char *newLowerLayers)
{
    ANSC_STATUS   retStatus;
    INT           IfIndex = -1;
    if (ifname == NULL || newLowerLayers == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &IfIndex);
    if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == IfIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }
    if (IfIndex < TOTAL_NUMBER_OF_INTERNAL_INTERFACES)
    {
        CcspTraceError(("%s Invalid or Default index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }
    pthread_mutex_lock(&gmEthGInfo_mutex);
    strncpy(gpstEthGInfo[IfIndex].LowerLayers,newLowerLayers, sizeof(gpstEthGInfo[IfIndex].LowerLayers));
    pthread_mutex_unlock(&gmEthGInfo_mutex);
    CcspTraceError(("%s name[%s] updated successfully[%d]\n", __FUNCTION__, newLowerLayers,IfIndex));
    return ANSC_STATUS_SUCCESS;
}
ANSC_STATUS CosaDmlEthPortSetName(char *ifname, char *newIfname)
{
    ANSC_STATUS   retStatus;
    INT           IfIndex = -1;
    if (ifname == NULL || newIfname == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    /* check newIfname already exists */
    retStatus = CosaDmlEthPortGetIndexFromIfName(newIfname, &IfIndex);
    if ( (ANSC_STATUS_FAILURE != retStatus ) || ( -1 != IfIndex ) )
    {
        CcspTraceError(("%s Already interface %s exists\n", __FUNCTION__,newIfname));
        return ANSC_STATUS_FAILURE;
    }
    retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &IfIndex);
    if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == IfIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }
    if (IfIndex < TOTAL_NUMBER_OF_INTERNAL_INTERFACES)
    {
        CcspTraceError(("%s Invalid or Default index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }
    pthread_mutex_lock(&gmEthGInfo_mutex);
    strncpy(gpstEthGInfo[IfIndex].Name,newIfname, sizeof(gpstEthGInfo[IfIndex].Name));
    pthread_mutex_unlock(&gmEthGInfo_mutex);
    CcspTraceError(("%s name[%s] updated successfully[%d]\n", __FUNCTION__, newIfname,IfIndex));
    return ANSC_STATUS_SUCCESS;
}
ANSC_STATUS CosaDmlEthPortGetLinkStatus(char *ifname, COSA_DML_ETH_LINK_STATUS *LinkStatus)
{
    ANSC_STATUS   retStatus;
    INT           IfIndex = -1;
    if (ifname == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &IfIndex);
    if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == IfIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }
    if (IfIndex < 0 )
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }
    pthread_mutex_lock(&gmEthGInfo_mutex);
    *LinkStatus = gpstEthGInfo[IfIndex].LinkStatus;
    pthread_mutex_unlock(&gmEthGInfo_mutex);
    return ANSC_STATUS_SUCCESS;
}
#endif  //FEATURE_RDKB_WAN_AGENT
#if defined (FEATURE_RDKB_WAN_AGENT) || defined (FEATURE_RDKB_WAN_MANAGER)
static INT CosaDmlEthGetTotalNoOfInterfaces(VOID)
{
#ifdef FEATURE_RDKB_WAN_AGENT
    //TODO - READ FROM HAL.
    return TOTAL_NUMBER_OF_INTERFACES;
#elif FEATURE_RDKB_WAN_MANAGER
    return gTotal;
#else
    return 0;
#endif
}
#if defined (FEATURE_RDKB_WAN_AGENT)
ANSC_STATUS CosaDmlEthPortSetUpstream(INT IfIndex, BOOL Upstream)
#else
ANSC_STATUS CosaDmlEthPortSetUpstream( CHAR *ifname, BOOL Upstream )
#endif
{
#if defined (FEATURE_RDKB_WAN_MANAGER)
    ETH_SM_PRIVATE_INFO stSMPrivateInfo = {0};
    ANSC_STATUS   retStatus;
    INT           IfIndex = -1;
    if (ifname == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &IfIndex);
    if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == IfIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }
    if (IfIndex < 0)
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }

    // Set upstream global info.
    pthread_mutex_lock(&gmEthGInfo_mutex);

    /* Check upstream flag is already set to true.
     * In that case, we don't need to start the state machine again.
     */
    if (gpstEthGInfo[IfIndex].Upstream == Upstream)
    {
        CcspTraceInfo(("%s %d - Same upstream value received, no need to do anything \n", __FUNCTION__, __LINE__));
        pthread_mutex_unlock(&gmEthGInfo_mutex);
        return ANSC_STATUS_SUCCESS;
    }

    gpstEthGInfo[IfIndex].Upstream = Upstream;
    snprintf(stSMPrivateInfo.Name, sizeof(stSMPrivateInfo.Name), "%s", gpstEthGInfo[IfIndex].Name);
    pthread_mutex_unlock(&gmEthGInfo_mutex);

    //Start the State machine thread.
    if (TRUE == Upstream)
    {
        /* Create and Start EthAgent state machine */
        CosaEthManager_Start_StateMachine(&stSMPrivateInfo);
        CcspTraceInfo(("%s %d - RDKB_ETH_CFG_CHANGED:ETH state machine started\n", __FUNCTION__, __LINE__));
    }
    return ANSC_STATUS_SUCCESS;
    
#endif //FEATURE_RDKB_WAN_MANAGER
    //Validate index
#ifdef FEATURE_RDKB_WAN_AGENT 
    ETH_SM_PRIVATE_INFO stSMPrivateInfo = {0};
    if (IfIndex < 0)
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }

    // Set upstream global info.
    pthread_mutex_lock(&gmEthGInfo_mutex);

    /* Check upstream flag is already set to true.
     * In that case, we don't need to start the state machine again.
     */
    if (gpstEthGInfo[IfIndex].Upstream == Upstream)
    {
        CcspTraceInfo(("%s %d - Same upstream value received, no need to do anything \n", __FUNCTION__, __LINE__));
        pthread_mutex_unlock(&gmEthGInfo_mutex);
        return ANSC_STATUS_SUCCESS;
    }

    gpstEthGInfo[IfIndex].Upstream = Upstream;
    snprintf(stSMPrivateInfo.Name, sizeof(stSMPrivateInfo.Name), "%s", gpstEthGInfo[IfIndex].Name);
    pthread_mutex_unlock(&gmEthGInfo_mutex);

    //Start the State machine thread.
    if (TRUE == Upstream)
    {
        /* Create and Start EthAgent state machine */
        CosaEthManager_Start_StateMachine(&stSMPrivateInfo);
        CcspTraceInfo(("%s %d - RDKB_ETH_CFG_CHANGED:ETH state machine started\n", __FUNCTION__, __LINE__));
    }
#else 
    UNREFERENCED_PARAMETER(IfIndex); 
    UNREFERENCED_PARAMETER(Upstream); 
#endif //#if defined (FEATURE_RDKB_WAN_MANAGER)     
    return ANSC_STATUS_SUCCESS;
}

INT CosaDmlEthPortLinkStatusCallback(CHAR *ifname, CHAR *state)
{
    if (NULL == ifname || state == NULL)
    {
        CcspTraceError(("Invalid memory \n"));
        return ANSC_STATUS_FAILURE;
    }

    COSA_DML_ETH_LINK_STATUS link_status;

    if (strncmp(state, UP, 2) == 0)
    {
        link_status = ETH_LINK_STATUS_UP;
    }
    else
    {
        link_status = ETH_LINK_STATUS_DOWN;
    }

    INT ifIndex = -1;
    if (CosaDmlEthPortGetIndexFromIfName(ifname, &ifIndex) == ANSC_STATUS_SUCCESS)
    {
        CosaETHMSGQWanData MSGQWanData = {0};

        pthread_mutex_lock(&gmEthGInfo_mutex);
        gpstEthGInfo[ifIndex].LinkStatus = link_status;
        snprintf(MSGQWanData.Name, sizeof(MSGQWanData.Name), "%s", gpstEthGInfo[ifIndex].Name);
        pthread_mutex_unlock(&gmEthGInfo_mutex);
        MSGQWanData.LinkStatus = gpstEthGInfo[ifIndex].LinkStatus;
        //Send message to Queue
        CosaDmlEthPortSendLinkStatusToEventQueue(&MSGQWanData);
    }
    return TRUE;
}

static ANSC_STATUS CosaDmlEthPortGetIndexFromIfName(char *ifname, INT *IfIndex)
{
    INT iTotalInterfaces = CosaDmlEthGetTotalNoOfInterfaces();
    INT iLoopCount;

    if (NULL == ifname || IfIndex == NULL || gpstEthGInfo == NULL)
    {
        CcspTraceError(("Invalid Memory \n"));
        return ANSC_STATUS_FAILURE;
    }

    *IfIndex = -1;
    pthread_mutex_lock(&gmEthGInfo_mutex);
    for (iLoopCount = 0; iLoopCount < iTotalInterfaces; iLoopCount++)
    {
        if ((NULL != &gpstEthGInfo[iLoopCount]) && (0 == strcmp(gpstEthGInfo[iLoopCount].Name, ifname)))
        {
            *IfIndex = iLoopCount;
            pthread_mutex_unlock(&gmEthGInfo_mutex);
            return ANSC_STATUS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&gmEthGInfo_mutex);

    return ANSC_STATUS_FAILURE;
}

/* CosaDmlEthLineGetCopyOfGlobalInfoForGivenIndex() */
ANSC_STATUS CosaDmlEthPortGetCopyOfGlobalInfoForGivenIfName(char *ifname, PCOSA_DML_ETH_PORT_GLOBAL_CONFIG pGlobalInfo)
{
    ANSC_STATUS   retStatus;
    INT           LineIndex = -1;

    if ((NULL == pGlobalInfo) || (ifname == NULL))
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    retStatus = CosaDmlEthPortGetIndexFromIfName(ifname, &LineIndex);

    if ( (ANSC_STATUS_FAILURE == retStatus ) || ( -1 == LineIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }

    //Copy of the data
    pthread_mutex_lock(&gmEthGInfo_mutex);
    memcpy(pGlobalInfo, &gpstEthGInfo[LineIndex], sizeof(COSA_DML_ETH_PORT_GLOBAL_CONFIG));
    pthread_mutex_unlock(&gmEthGInfo_mutex);

    return (ANSC_STATUS_SUCCESS);
}

/* *CosaDmlEthLinePrepareGlobalInfo() */
static ANSC_STATUS CosDmlEthPortPrepareGlobalInfo()
{
    INT iLoopCount = 0;
    INT Totalinterfaces = 0;

    Totalinterfaces = CosaDmlEthGetTotalNoOfInterfaces();

    //Allocate memory for Eth Global Status Information
    gpstEthGInfo = (PCOSA_DML_ETH_PORT_GLOBAL_CONFIG)AnscAllocateMemory(sizeof(COSA_DML_ETH_PORT_GLOBAL_CONFIG) * Totalinterfaces);

    //Return failure if allocation failiure
    if (NULL == gpstEthGInfo)
    {
        return ANSC_STATUS_FAILURE;
    }

    memset(gpstEthGInfo, 0, sizeof(COSA_DML_ETH_PORT_GLOBAL_CONFIG) * Totalinterfaces);
    //Assign default value
    for (iLoopCount = 0; iLoopCount < Totalinterfaces; ++iLoopCount)
    {
        gpstEthGInfo[iLoopCount].Upstream = FALSE;
        gpstEthGInfo[iLoopCount].WanStatus = ETH_WAN_DOWN;
        gpstEthGInfo[iLoopCount].LinkStatus = ETH_LINK_STATUS_DOWN;
        gpstEthGInfo[iLoopCount].WanValidated = TRUE; //Make default as True.
        snprintf(gpstEthGInfo[iLoopCount].Name, sizeof(gpstEthGInfo[iLoopCount].Name), "eth%d", iLoopCount);
        snprintf(gpstEthGInfo[iLoopCount].Path, sizeof(gpstEthGInfo[iLoopCount].Path), "%s%d", ETHERNET_IF_PATH, iLoopCount + 1);
#if defined(FEATURE_RDKB_WAN_MANAGER)
        gpstEthGInfo[iLoopCount].Enable = FALSE; //Make default as False.
        snprintf(gpstEthGInfo[iLoopCount].LowerLayers, sizeof(gpstEthGInfo[iLoopCount].LowerLayers), "%s%d", ETHERNET_IF_LOWERLAYERS, iLoopCount + 1);
#endif //FEATURE_RDKB_WAN_MANAGER
    }

    return ANSC_STATUS_SUCCESS;
}

/* Send link information to the message queue. */
static ANSC_STATUS CosaDmlEthPortSendLinkStatusToEventQueue(CosaETHMSGQWanData *MSGQWanData)
{
    CosaEthEventQData EventMsg = {0};
    mqd_t mq;
    char buffer[MAX_QUEUE_MSG_SIZE];

    //Validate buffer
    if (NULL == MSGQWanData)
    {
        CcspTraceError(("%s %d Invalid Buffer\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //message queue send
    mq = mq_open(COSA_ETH_EVENT_QUEUE_NAME, O_WRONLY);
    if (!((mqd_t)-1 != mq)) {
        CcspTraceError(("%s:%d: ", __FUNCTION__, __LINE__));
        perror("((mqd_t)-1 != mq)");
        return ANSC_STATUS_FAILURE;
    }
    memset(buffer, 0, MAX_QUEUE_MSG_SIZE);
    EventMsg.MsgType = MSG_TYPE_WAN;

    memcpy(EventMsg.Msg, MSGQWanData, sizeof(CosaETHMSGQWanData));
    memcpy(buffer, &EventMsg, sizeof(EventMsg));
    if (!(0 <= mq_send(mq, buffer, MAX_QUEUE_MSG_SIZE, 0))) {
        CcspTraceError(("%s:%d: ", __FUNCTION__, __LINE__));
        perror("(0 <= mq_send(mq, buffer, MAX_QUEUE_MSG_SIZE, 0))");
        return ANSC_STATUS_FAILURE;
    }
    if (!((mqd_t)-1 != mq_close(mq))) {
        CcspTraceError(("%s:%d: ", __FUNCTION__, __LINE__));
        perror("((mqd_t)-1 != mq_close(mq))");
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d - Successfully sent message to event queue\n", __FUNCTION__, __LINE__));

    return ANSC_STATUS_SUCCESS;
}

/* Start event handler thread. */
static void CosaDmlEthTriggerEventHandlerThread(void)
{
    pthread_t EvtThreadId;
    int iErrorCode = 0;

    //Eth event handler thread
    iErrorCode = pthread_create(&EvtThreadId, NULL, &CosaDmlEthEventHandlerThread, NULL);

    if (0 != iErrorCode)
    {
        CcspTraceInfo(("%s %d - Failed to start Event Handler Thread EC:%d\n", __FUNCTION__, __LINE__, iErrorCode));
    }
    else
    {
        CcspTraceInfo(("%s %d - Event Handler Thread Started Successfully\n", __FUNCTION__, __LINE__));
    }
}

/* Event handler thread which is monitoring for the
 * interface link status and dispatch to the other agent.
 */
static void *CosaDmlEthEventHandlerThread(void *arg)
{
    UNREFERENCED_PARAMETER(arg);
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_QUEUE_MSG_SIZE + 1];

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_QUEUE_LENGTH;
    attr.mq_msgsize = MAX_QUEUE_MSG_SIZE;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    mq = mq_open(COSA_ETH_EVENT_QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);

    if (!((mqd_t)-1 != mq)) {
        CcspTraceError(("%s:%d: ", __FUNCTION__, __LINE__));
        perror("((mqd_t)-1 != mq)");
        return NULL;
    }

    do
    {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MAX_QUEUE_MSG_SIZE, NULL);
        if (!(bytes_read >= 0)) {
            CcspTraceError(("%s:%d: ", __FUNCTION__, __LINE__));
            perror("(bytes_read >= 0)");
            return NULL;
        }
        CosaEthEventQData EventMsg = {0};
        buffer[bytes_read] = '\0';
        memcpy(&EventMsg, buffer, sizeof(EventMsg));

        //WAN Event
        if (MSG_TYPE_WAN == EventMsg.MsgType)
        {
            CosaETHMSGQWanData MSGQWanData = {0};
            char acTmpPhyStatus[32] = {0};
            BOOL IsValidStatus = TRUE;
            memcpy(&MSGQWanData, EventMsg.Msg, sizeof(CosaETHMSGQWanData));
            CcspTraceInfo(("%s %d - Event Msg Received\n", __FUNCTION__, __LINE__));
            CcspTraceInfo(("****** [%s,%d]\n", MSGQWanData.Name, MSGQWanData.LinkStatus));

            switch (MSGQWanData.LinkStatus)
            {
            case ETH_LINK_STATUS_UP:
                snprintf(acTmpPhyStatus, sizeof(acTmpPhyStatus), "%s", "Up");
                break;
            case ETH_LINK_STATUS_DOWN:
                snprintf(acTmpPhyStatus, sizeof(acTmpPhyStatus), "%s", "Down");
                break;
            default:
                IsValidStatus = FALSE;
                break;
            }

            if (TRUE == IsValidStatus)
            {
#ifdef FEATURE_RDKB_WAN_AGENT
                CosaDmlEthSetPhyStatusForWanAgent(MSGQWanData.Name, acTmpPhyStatus);
#else
               	CosaDmlEthSetPhyStatusForWanManager(MSGQWanData.Name, acTmpPhyStatus);
#endif //FEATURE_RDKB_WAN_AGENT
            }
        }
    } while (1);

    //exit from thread
    pthread_exit(NULL);
}

/* Get data from the other component. */
static ANSC_STATUS CosaDmlEthGetParamNames(char *pComponent, char *pBus, char *pParamName, char a2cReturnVal[][256], int *pReturnSize)
{
    parameterInfoStruct_t **retInfo;
    int ret = 0,
        nval;

    ret = CcspBaseIf_getParameterNames(
        bus_handle,
        pComponent,
        pBus,
        pParamName,
        1,
        &nval,
        &retInfo);

    if (CCSP_SUCCESS == ret)
    {
        int iLoopCount;

        *pReturnSize = nval;

        for (iLoopCount = 0; iLoopCount < nval; iLoopCount++)
        {
            if (NULL != retInfo[iLoopCount]->parameterName)
            {
                CcspTraceWarning(("%s parameterName[%d,%s]\n", __FUNCTION__, iLoopCount, retInfo[iLoopCount]->parameterName));

                snprintf(a2cReturnVal[iLoopCount], strlen(retInfo[iLoopCount]->parameterName) + 1, "%s", retInfo[iLoopCount]->parameterName);
            }
        }

        if (retInfo)
        {
            free_parameterInfoStruct_t(bus_handle, nval, retInfo);
        }

        return ANSC_STATUS_SUCCESS;
    }

    if (retInfo)
    {
        free_parameterInfoStruct_t(bus_handle, nval, retInfo);
    }

    return ANSC_STATUS_FAILURE;
}

/* *CosaDmlEthGetParamValues() */
static ANSC_STATUS CosaDmlEthGetParamValues(char *pComponent, char *pBus, char *pParamName, char *pReturnVal)
{
    parameterValStruct_t **retVal;
    char *ParamName[1];
    int ret = 0,
        nval;

    //Assign address for get parameter name
    ParamName[0] = pParamName;

    ret = CcspBaseIf_getParameterValues(
        bus_handle,
        pComponent,
        pBus,
        ParamName,
        1,
        &nval,
        &retVal);

    //Copy the value
    if (CCSP_SUCCESS == ret)
    {
        CcspTraceWarning(("%s parameterValue[%s]\n", __FUNCTION__, retVal[0]->parameterValue));

        if (NULL != retVal[0]->parameterValue)
        {
            memcpy(pReturnVal, retVal[0]->parameterValue, strlen(retVal[0]->parameterValue) + 1);
        }

        if (retVal)
        {
            free_parameterValStruct_t(bus_handle, nval, retVal);
        }

        return ANSC_STATUS_SUCCESS;
    }

    if (retVal)
    {
        free_parameterValStruct_t(bus_handle, nval, retVal);
    }

    return ANSC_STATUS_FAILURE;
}
#ifdef FEATURE_RDKB_WAN_AGENT
/* Notification to the Other component. */
static ANSC_STATUS CosaDmlEthSetParamValues(char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, unsigned int bCommitFlag)
{
    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t param_val[1] = {0};
    char *faultParam = NULL;
    char acParameterName[256] = {0},
         acParameterValue[128] = {0};
    int ret = 0;

    sprintf(acParameterName, "%s", pParamName);
    param_val[0].parameterName = acParameterName;

    sprintf(acParameterValue, "%s", pParamVal);
    param_val[0].parameterValue = acParameterValue;

    param_val[0].type = type;

    ret = CcspBaseIf_setParameterValues(
        bus_handle,
        pComponent,
        pBus,
        0,
        0,
        param_val,
        1,
        bCommitFlag,
        &faultParam);

    CcspTraceInfo(("Value being set [%d] \n", ret));

    if ((ret != CCSP_SUCCESS) && (faultParam != NULL))
    {
        CcspTraceError(("%s-%d Failed to set %s\n", __FUNCTION__, __LINE__, pParamName));
        bus_info->freefunc(faultParam);
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}
#else // FEATURE_RDKB_WAN_MANAGER
/* Notification to the Other component. */
static ANSC_STATUS CosaDmlEthSetParamValues(const char *pComponent, const char *pBus, const char *pParamName, const char *pParamVal, enum dataType_e type, unsigned int bCommitFlag)
{
    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t param_val[1] = {0};
    char *faultParam = NULL;
    int ret = 0;
    param_val[0].parameterName = (char *)pParamName;
    param_val[0].parameterValue = (char *)pParamVal;
    param_val[0].type = type;
    ret = CcspBaseIf_setParameterValues(
        bus_handle,
        pComponent,
        (char *)pBus,
        0,
        0,
        param_val,
        1,
        bCommitFlag,
        &faultParam);
    CcspTraceInfo(("Value being set [%d] \n", ret));
    if ((ret != CCSP_SUCCESS) && (faultParam != NULL))
    {
        CcspTraceError(("%s-%d Failed to set %s\n", __FUNCTION__, __LINE__, pParamName));
        bus_info->freefunc(faultParam);
        return ANSC_STATUS_FAILURE;
    }
    return ANSC_STATUS_SUCCESS;
}
#endif //FEATURE_RDKB_WAN_AGENT

#ifdef FEATURE_RDKB_WAN_AGENT
/* Set wan status event to Wanagent. */
ANSC_STATUS CosaDmlEthSetWanStatusForWanAgent(char *ifname, char *WanStatus)
{
    COSA_DML_ETH_PORT_GLOBAL_CONFIG stGlobalInfo = {0};
    char acSetParamName[256];
    INT iWANInstance = -1;

    //Validate buffer
    if ((NULL == ifname) || (NULL == WanStatus))
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get global copy of the data from interface name
    CosaDmlEthPortGetCopyOfGlobalInfoForGivenIfName(ifname, &stGlobalInfo);

    //Get Instance for corresponding name
    CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_WAN_AGENT, stGlobalInfo.Name, &iWANInstance);

    //Index is not present. so no need to do anything any WAN instance
    if (-1 == iWANInstance)
    {
        CcspTraceError(("%s %d WAN instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d WAN Instance:%d\n", __FUNCTION__, __LINE__, iWANInstance));

    //Set WAN Status
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_STATUS_PARAM_NAME, iWANInstance);
    CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, WanStatus, ccsp_string, TRUE);

    CcspTraceInfo(("%s %d Successfully notified %s event to WAN Agent for %s interface\n", __FUNCTION__, __LINE__, WanStatus, ifname));

    return ANSC_STATUS_SUCCESS;
}
#else //FEATURE_RDKB_WAN_MANAGER
/* Set wan link status event to WanManager. */
ANSC_STATUS CosaDmlEthSetWanLinkStatusForWanManager(char *ifname, char *WanStatus)
{
    COSA_DML_ETH_PORT_GLOBAL_CONFIG stGlobalInfo = {0};
    char *acSetParamName = NULL;
    char *acSetParamValue = NULL;
    INT iWANInstance = -1;
    //Validate buffer
    if ((NULL == ifname) || (NULL == WanStatus))
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    //Get global copy of the data from interface name
    CosaDmlEthPortGetCopyOfGlobalInfoForGivenIfName(ifname, &stGlobalInfo);
    //Get Instance for corresponding name
    CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_WAN_AGENT, stGlobalInfo.Name, &iWANInstance);
    //Index is not present. so no need to do anything any WAN instance
    if (-1 == iWANInstance)
    {
        CcspTraceError(("%s %d WAN instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }
    acSetParamName = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);
    acSetParamValue = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);

    if(acSetParamName == NULL || acSetParamValue == NULL)
    {
        CcspTraceError(("%s Memory allocation failure \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
 
    memset(acSetParamName, 0, DATAMODEL_PARAM_LENGTH);
    memset(acSetParamValue, 0, DATAMODEL_PARAM_LENGTH);
    CcspTraceInfo(("%s %d WAN Instance:%d\n", __FUNCTION__, __LINE__, iWANInstance));
    //Set WAN Status
    snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, WAN_LINK_STATUS_PARAM_NAME, iWANInstance);
    snprintf(acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", WanStatus);
    CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, TRUE);
    if(acSetParamName != NULL)
    {
       free(acSetParamName);
    }
    if(acSetParamValue != NULL)
    {
       free(acSetParamValue);
    }
    CcspTraceInfo(("%s %d Successfully notified %s event to WAN Agent for %s interface\n", __FUNCTION__, __LINE__, WanStatus, ifname));
    return ANSC_STATUS_SUCCESS;
}
#endif //FEATURE_RDKB_WAN_AGENT

static ANSC_STATUS CosaDmlGetWanOEInterfaceName(char *pInterface, unsigned int length)
{
    char acTmpReturnValue[256] = {0};
    INT iLoopCount;
    INT iTotalNoofEntries;

    if(ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, WAN_NOE_PARAM_NAME,acTmpReturnValue))
    {
        CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Total count
    iTotalNoofEntries = atoi(acTmpReturnValue);
    CcspTraceInfo(("%s %d - TotalNoofEntries:%d\n", __FUNCTION__, __LINE__, iTotalNoofEntries));

    if (0 >= iTotalNoofEntries) {
        return ANSC_STATUS_FAILURE;
    }
	
    //Traverse from loop
    for (iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++) {
        char acTmpQueryParam[256] = {0};
        //Query
        snprintf(acTmpQueryParam, sizeof(acTmpQueryParam), WAN_IF_NAME_PARAM_NAME, iLoopCount + 1);

        memset(acTmpReturnValue, 0, sizeof(acTmpReturnValue));
        if(ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(WAN_COMPONENT_NAME,WAN_DBUS_PATH,acTmpQueryParam,acTmpReturnValue))
        {
            CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
            continue;
        }

        if (NULL != strstr(acTmpReturnValue, "eth")) {
            strncpy(pInterface, acTmpReturnValue, length);
            break;
        }
    }
	
    return ANSC_STATUS_SUCCESS;
}

/* * CosaDmlEthGetLowerLayersInstanceInOtherAgent() */
static ANSC_STATUS CosaDmlEthGetLowerLayersInstanceInOtherAgent(COSA_ETH_NOTIFY_ENUM enNotifyAgent, char *pLowerLayers, INT *piInstanceNumber)
{
    //Validate buffer
    if ((NULL == pLowerLayers) || (NULL == piInstanceNumber))
    {
        CcspTraceError(("%s Invalid Buffer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Initialise default value
    *piInstanceNumber = -1;

    switch (enNotifyAgent)
    {
    case NOTIFY_TO_WAN_AGENT:
    {
        char acTmpReturnValue[256] = {0};
        INT iLoopCount;
        INT iTotalNoofEntries;

        if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, WAN_NOE_PARAM_NAME, acTmpReturnValue))
        {
            CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
            return ANSC_STATUS_FAILURE;
        }

        //Total count
        iTotalNoofEntries = atoi(acTmpReturnValue);
        CcspTraceInfo(("%s %d - TotalNoofEntries:%d\n", __FUNCTION__, __LINE__, iTotalNoofEntries));

        if (0 >= iTotalNoofEntries)
        {
            return ANSC_STATUS_SUCCESS;
        }

        //Traverse from loop
        for (iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++)
        {
            char acTmpQueryParam[256] = {0};

            //Query
            snprintf(acTmpQueryParam, sizeof(acTmpQueryParam), WAN_IF_NAME_PARAM_NAME, iLoopCount + 1);

            memset(acTmpReturnValue, 0, sizeof(acTmpReturnValue));
            if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acTmpQueryParam, acTmpReturnValue))
            {
                CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                continue;
            }

            //Compare name
            if (0 == strcmp(acTmpReturnValue, pLowerLayers))
            {
                *piInstanceNumber = iLoopCount + 1;
                break;
            }
        }
    }
    break; /* * NOTIFY_TO_WAN_AGENT */

    case NOTIFY_TO_VLAN_AGENT:
    {
        char acTmpReturnValue[256] = {0},
             a2cTmpTableParams[10][256] = {0};
        INT iLoopCount,
            iTotalNoofEntries;

        if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, VLAN_ETH_NOE_PARAM_NAME, acTmpReturnValue))
        {
            CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
            return ANSC_STATUS_FAILURE;
        }
        //Total count
        iTotalNoofEntries = atoi(acTmpReturnValue);
        CcspTraceInfo(("%s %d - TotalNoofEntries:%d\n", __FUNCTION__, __LINE__, iTotalNoofEntries));

        if (0 >= iTotalNoofEntries)
        {
            return ANSC_STATUS_SUCCESS;
        }

        //Get table names
        iTotalNoofEntries = 0;
        if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamNames(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, VLAN_ETH_LINK_TABLE_NAME, a2cTmpTableParams, &iTotalNoofEntries))
        {
            CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
            return ANSC_STATUS_FAILURE;
        }

        //Traverse from loop
        for (iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++)
        {
            char acTmpQueryParam[256] = {0};

            //Query
            snprintf(acTmpQueryParam, sizeof(acTmpQueryParam), "%sLowerLayers", a2cTmpTableParams[iLoopCount]);

            memset(acTmpReturnValue, 0, sizeof(acTmpReturnValue));
            if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acTmpQueryParam, acTmpReturnValue))
            {
                CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                continue;
            }

            //Compare lowerlayers
            if (0 == strcmp(acTmpReturnValue, pLowerLayers))
            {
                char tmpTableParam[256] = {0};
                const char *last_two;

                //Copy table param
                snprintf(tmpTableParam, sizeof(tmpTableParam), "%s", a2cTmpTableParams[iLoopCount]);

                //Get last two chareters from return value and cut the instance
                last_two = &tmpTableParam[strlen(VLAN_ETH_LINK_TABLE_NAME)];

                *piInstanceNumber = atoi(last_two);
                break;
            }
        }
    }
    break; /* * NOTIFY_TO_XTM_AGENT */

    default:
    {
        CcspTraceError(("%s Invalid Case\n", __FUNCTION__));
    }
    break; /* * default */
    }

    return ANSC_STATUS_SUCCESS;
}

/* Create and Enbale Ethernet.Link. */
ANSC_STATUS CosaDmlEthCreateEthLink(char *l2ifName, char *Path)
{
    //COSA_DML_ETH_PORT_GLOBAL_CONFIG stGlobalInfo   = { 0 };
    char acSetParamName[256];
    INT LineIndex = -1,
        iVLANInstance = -1;

    //Validate buffer
    if (NULL == l2ifName || NULL == Path)
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get line index from name
    if (ANSC_STATUS_SUCCESS != CosaDmlEthPortGetIndexFromIfName(l2ifName, &LineIndex))
    {
        CcspTraceError(("%s Failed to get index for this %s interface\n", __FUNCTION__, l2ifName));
        return ANSC_STATUS_FAILURE;
    }

    //Get Instance for corresponding lower layer
    CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_VLAN_AGENT,Path, &iVLANInstance);

    //Create VLAN Link.
    //Index is not present. so needs to create a PTM instance
    if (-1 == iVLANInstance)
    {
        char acTableName[128] = {0};
        INT iNewTableInstance = -1;

        sprintf(acTableName, "%s", VLAN_ETH_LINK_TABLE_NAME);
        if (CCSP_SUCCESS != CcspBaseIf_AddTblRow(
                                bus_handle,
                                VLAN_COMPONENT_NAME,
                                VLAN_DBUS_PATH,
                                0, /* session id */
                                acTableName,
                                &iNewTableInstance))
        {
            CcspTraceError(("%s Failed to add table %s\n", __FUNCTION__, acTableName));
            return ANSC_STATUS_FAILURE;
        }

        //Assign new instance
        iVLANInstance = iNewTableInstance;
    }

    CcspTraceInfo(("%s %d VLANAgent -> Device.Ethernet.Link Instance:%d\n", __FUNCTION__, __LINE__, iVLANInstance));

    //Set Alias
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), VLAN_ETH_LINK_PARAM_ALIAS, iVLANInstance);
    CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, l2ifName, ccsp_string,FALSE);

    //Set Name
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), VLAN_ETH_LINK_PARAM_NAME, iVLANInstance);
    CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, l2ifName, ccsp_string,FALSE);

    //Set Lowerlayers
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), VLAN_ETH_LINK_PARAM_LOWERLAYERS, iVLANInstance);
    CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, Path, ccsp_string,FALSE);

    //Set Enable
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), VLAN_ETH_LINK_PARAM_ENABLE, iVLANInstance);
    CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, "true", ccsp_boolean, TRUE);

    CcspTraceInfo(("%s %d Enabled Device.Ethernet.Link.%d.Enable for %s interface\n", __FUNCTION__, __LINE__,iVLANInstance, l2ifName));

    return ANSC_STATUS_SUCCESS;
}

/* Disable and delete Eth link. (Ethernet.Link.) */
ANSC_STATUS CosaDmlEthDeleteEthLink(char *ifName, char *Path)
{
    char acSetParamName[256],
        acTableName[128] = {0};
    INT LineIndex = -1,
        iVLANInstance = -1;

    //Validate buffer
    if ((NULL == ifName ) || ( NULL == Path ))
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get line index from name
    if (ANSC_STATUS_SUCCESS != CosaDmlEthPortGetIndexFromIfName(ifName, &LineIndex))
    {
        CcspTraceError(("%s Failed to get index for this %s interface\n", __FUNCTION__, ifName));
        return ANSC_STATUS_FAILURE;
    }

    //Get Instance for corresponding lower layer
    CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_VLAN_AGENT,Path, &iVLANInstance);

    //Index is not present. so no need to do anything any ETH Link instance
    if (-1 == iVLANInstance)
    {
        CcspTraceError(("%s %d Device.Ethernet.Link Table instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d VLANAgent -> Device.Ethernet.Link Instance:%d\n", __FUNCTION__, __LINE__, iVLANInstance));

    //Disable link.
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), VLAN_ETH_LINK_PARAM_ENABLE, iVLANInstance);
    CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, "false", ccsp_boolean, TRUE);

    //Delay - to set param.
    sleep(2);

    /*
     * Delete Device.Ethernet.Link. Instance.
     * VLANAgent will delete the vlan interface as part table deletion process.
     */
    sprintf(acTableName, "%s%d.", VLAN_ETH_LINK_TABLE_NAME, iVLANInstance);
    if (CCSP_SUCCESS != CcspBaseIf_DeleteTblRow(
                            bus_handle,
                            VLAN_COMPONENT_NAME,
                            VLAN_DBUS_PATH,
                            0, /* session id */
                            acTableName))
    {
        CcspTraceError(("%s Failed to delete table %s\n", __FUNCTION__, acTableName));
        return ANSC_STATUS_FAILURE;
    }
    CcspTraceInfo(("%s %d Deleted the Device.Ethernet.Link.%d. Table Instance for %s interface\n", __FUNCTION__, __LINE__, iVLANInstance, ifName));

    return ANSC_STATUS_SUCCESS;
}
#ifdef FEATURE_RDKB_WAN_AGENT
ANSC_STATUS CosaDmlEthSetPhyStatusForWanAgent(char *ifname, char *PhyStatus)
#else  //FEATURE_RDKB_WAN_MANAGER
ANSC_STATUS CosaDmlEthSetPhyStatusForWanManager(char *ifname, char *PhyStatus)
#endif
{
    COSA_DML_ETH_PORT_GLOBAL_CONFIG stGlobalInfo = {0};
    char acSetParamName[256];
    char acSetParamVal[256];
    INT iLinkInstance = -1;
    INT iWANInstance = -1;

    //Validate buffer
    if ((NULL == ifname) || (NULL == PhyStatus))
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get global copy of the data from interface name
    CosaDmlEthPortGetCopyOfGlobalInfoForGivenIfName(ifname, &stGlobalInfo);

    //Get Instance for corresponding name
    CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_WAN_AGENT, stGlobalInfo.Name, &iWANInstance);

    //Index is not present. so no need to do anything any WAN instance
    if (-1 == iWANInstance)
    {
        CcspTraceError(("%s %d WAN instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d WAN Instance:%d\n", __FUNCTION__, __LINE__, iWANInstance));

    //Set PHY path
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_PHY_PATH_PARAM_NAME, iWANInstance);

    memset(acSetParamVal, 0, sizeof(acSetParamVal));
    CosaDmlEthPortGetIndexFromIfName(ifname, &iLinkInstance);
    snprintf(acSetParamVal, sizeof(acSetParamVal),ETH_IF_PHY_PATH, (iLinkInstance + 1));
    CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamVal, ccsp_string, TRUE);

    //Set PHY Status
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_PHY_STATUS_PARAM_NAME, iWANInstance);
    CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, PhyStatus, ccsp_string, TRUE);

    CcspTraceInfo(("%s %d Successfully notified %s event to WAN Agent for %s interface\n", __FUNCTION__, __LINE__, PhyStatus, ifname));

    return ANSC_STATUS_SUCCESS;
}

#if FEATURE_RDKB_WAN_AGENT
ANSC_STATUS CosaDmlEthGetPhyStatusForWanAgent(char *ifname, char *PhyStatus)
#else //FEATURE_RDKB_WAN_MANAGER
ANSC_STATUS CosaDmlEthGetPhyStatusForWanManager(char *ifname, char *PhyStatus)
#endif
{
    COSA_DML_ETH_PORT_GLOBAL_CONFIG stGlobalInfo = {0};
    char acGetParamName[256];
    INT iWANInstance = -1;

    //Validate buffer
    if ((NULL == ifname) || (NULL == PhyStatus))
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get global copy of the data from interface name
    CosaDmlEthPortGetCopyOfGlobalInfoForGivenIfName(ifname, &stGlobalInfo);

    //Get Instance for corresponding name
    CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_WAN_AGENT, stGlobalInfo.Name, &iWANInstance);

    //Index is not present. so no need to do anything any WAN instance
    if (-1 == iWANInstance)
    {
        CcspTraceError(("%s %d WAN instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d WAN Instance:%d\n", __FUNCTION__, __LINE__, iWANInstance));

    //Get PHY Status
    memset(acGetParamName, 0, sizeof(acGetParamName));
    snprintf(acGetParamName, sizeof(acGetParamName), WAN_PHY_STATUS_PARAM_NAME, iWANInstance);
    CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acGetParamName, PhyStatus);

    return ANSC_STATUS_SUCCESS;
}
#endif // defined (FEATURE_RDKB_WAN_MANAGER) || defined (FEATURE_RDKB_WAN_AGENT)
