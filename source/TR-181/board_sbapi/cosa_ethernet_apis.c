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
#include <ctype.h>

#include "utctx/utctx_api.h"
#include <sys/socket.h>
#include <sys/types.h>
#include "linux/sockios.h"
#include <sys/ioctl.h>

#ifdef ARRIS_XB3_PLATFORM_CHANGES
  #include "rdk_cm_api_arris.h"
#else
  #include "linux/if.h"
#endif

#include "cosa_ethernet_apis.h"
#include "safec_lib_common.h"
#include "cosa_ethernet_internal.h"
#include <stdbool.h>
#include "ccsp_hal_ethsw.h"
#include "secure_wrapper.h"
#include <platform_hal.h>

#if defined (FEATURE_RDKB_WAN_MANAGER)
#include "cosa_ethernet_manager.h"
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
#include "ccsp_psm_helper.h"
extern  char g_Subsystem[BUFLEN_32];
#endif  //FEATURE_RDKB_AUTO_PORT_SWITCH
#endif //FEATURE_RDKB_WAN_MANAGER

#include "syscfg.h"
#ifdef FEATURE_SUPPORT_ONBOARD_LOGGING


#define LOGGING_MODULE           "ETHAGENT"
#define OnboardLog(...)         rdk_log_onboard(LOGGING_MODULE, __VA_ARGS__)
#else
#define OnboardLog(...)
#endif

#if defined (WAN_FAILOVER_SUPPORTED)
#include "cosa_rbus_handler_apis.h"
#endif

#if defined (FEATURE_RDKB_WAN_AGENT) || defined(FEATURE_RDKB_WAN_MANAGER)
#include "cosa_ethernet_manager.h"
#if defined (FEATURE_RDKB_WAN_MANAGER)
#define TOTAL_NUMBER_OF_INTERNAL_INTERFACES 4
#define DATAMODEL_PARAM_LENGTH 256
#define WANOE_IFACENAME_LENGTH 32
#endif //FEATURE_RDKB_WAN_MANAGER
#define WANOE_IFACE_UP "Up"
#define WANOE_IFACE_DOWN "Down"
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
#define STATUS_BUFF_SIZE 32
#endif //FEATURE_RDKB_WAN_AGENT || defined(FEATURE_RDKB_WAN_MANAGER)

#if defined (_MACSEC_SUPPORT_)
#define MACSEC_TIMEOUT_SEC    10
#endif

#ifdef WAN_FAILOVER_SUPPORTED
#include "sysevent/sysevent.h"
#include "cosa_apis.h"
#include "plugin_main_apis.h"
#include <sys/stat.h>
int sysevent_fd;
token_t sysevent_token;
#endif

int _getMac(char* ifName, char* mac){

    int skfd = -1;
    struct ifreq ifr;
    
    AnscTraceFlow(("%s...\n", __FUNCTION__));

    AnscCopyString(ifr.ifr_name, ifName);
    
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    /* CID: 54085 Argument cannot be negative*/
    if(skfd == -1)
       return -1;

    AnscTraceFlow(("%s...\n", __FUNCTION__));
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
        if (errno == ENODEV) {
            close(skfd);
            return -1;
        }
    }
    if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
        CcspTraceWarning(("cosa_ethernet_apis.c - getMac: Get interface %s error %d...\n", ifName, errno));
        close(skfd);
        return -1;
    }
    close(skfd);

    AnscCopyMemory(mac, ifr.ifr_hwaddr.sa_data, 6);
    return 0; 

}
static bool isValid(char *str)
{
    int i;
    int len = strlen(str);
    for (i = 0; i <= len; i++) {
        if (!isspace(str[i])) {
            break;
        }
    }
    if (str[i] == '\0') {
        return false;
    }
    return true;
}

BOOLEAN getIfAvailability( const PUCHAR name )
{
    struct ifreq ifr;
    int skfd = -1;
    AnscTraceFlow(("%s... name %s\n", __FUNCTION__,name));

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (!isValid((char*)name)) {
        return -1;
    }
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    AnscTraceFlow(("%s... name %s\n", __FUNCTION__,name));
    AnscCopyString(ifr.ifr_name, (char *)name);
    
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
        if (errno == ENODEV) {
            close(skfd);
            return -1;
        }
    }
		
    if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0) {
        CcspTraceWarning(("%s : Get interface %s error (%s)...\n", 
									__FUNCTION__, 
									name, 
									strerror( errno )));
        close( skfd );

		return FALSE;
    }
	
    close(skfd);

	return TRUE;
}

COSA_DML_IF_STATUS getIfStatus(const PUCHAR name, struct ifreq *pIfr)
{
    struct ifreq ifr;
    int skfd = -1;
    
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    /* CID: 56442 Argument cannot be negative*/
    if(skfd == -1)
       return COSA_DML_IF_STATUS_Error;

    AnscCopyString(ifr.ifr_name, (char*)name);

    if (!isValid((char*)name)) {
        return COSA_DML_IF_STATUS_Error;
    }
    AnscTraceFlow(("%s...\n", __FUNCTION__));
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
        if (errno == ENODEV) {
            close(skfd);
            return COSA_DML_IF_STATUS_Error;
        }
		
        CcspTraceWarning(("cosa_ethernet_apis.c - getIfStatus: Get interface %s error...\n", name));
        close(skfd);

		if ( FALSE == getIfAvailability( name ) )
		{
			return COSA_DML_IF_STATUS_NotPresent;
		}

        return COSA_DML_IF_STATUS_Unknown;
    }
    close(skfd);

    if ( pIfr )
    {
        AnscCopyMemory(pIfr, &ifr, sizeof(struct ifreq));
    }
    
    if ( ifr.ifr_flags & IFF_UP )
    {
        return COSA_DML_IF_STATUS_Up;
    }
    else
    {
        return COSA_DML_IF_STATUS_Down;
    }
}

/**************************************************************************
                        DATA STRUCTURE DEFINITIONS
**************************************************************************/
typedef struct
{
  uint8_t  hw[6];
} macaddr_t;

typedef enum WanMode
{
    WAN_MODE_AUTO = 0,
    WAN_MODE_ETH,
    WAN_MODE_DOCSIS,
    WAN_MODE_UNKNOWN
}WanMode_t;
/**************************************************************************
                        GLOBAL VARIABLES
**************************************************************************/

/* ETH WAN Fallback Interface Name - Should eventually move away from Compile Time */
#if defined (_XB7_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_)
#define ETHWAN_DEF_INTF_NAME "eth3"
#elif defined (_CBR2_PRODUCT_REQ_)
#define ETHWAN_DEF_INTF_NAME "eth5"
#elif defined (INTEL_PUMA7)
#define ETHWAN_DEF_INTF_NAME "nsgmii0"
#elif defined (_PLATFORM_TURRIS_)
#define ETHWAN_DEF_INTF_NAME "eth2"
#else
#define ETHWAN_DEF_INTF_NAME "eth0"
#endif

#define ETH_HOST_PARAMVALUE_TRUE "true"
#define ETH_HOST_PARAMVALUE_FALSE "false"
#define ETH_HOST_MAC_LENGTH 17
/* For LED behavior */
#define WHITE 0
#define YELLOW 1
#define SOLID   0
#define BLINK   1
#define RED 3

ANSC_STATUS is_usg_in_bridge_mode(BOOL *pBridgeMode);
void CcspHalExtSw_SendNotificationForAllHosts( void );
extern  ANSC_HANDLE bus_handle;
extern  ANSC_HANDLE g_EthObject;
extern void* g_pDslhDmlAgent;

#if defined (FEATURE_RDKB_WAN_MANAGER)
#define INTERFACE_MAPPING_TABLE_FNAME "/var/tmp/map/mapping_table.txt"
ANSC_STATUS GetEthPhyInterfaceName(INT index, CHAR *ifname, INT ifnameLength);
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

static pthread_t bootInformThreadId;
static PCOSA_DML_ETH_PORT_GLOBAL_CONFIG gpstEthGInfo = NULL;
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
#if defined (FEATURE_RDKB_WAN_MANAGER)
void getInterfaceMacAddress(macaddr_t* macAddr,char *pIfname);
int EthWanSetLED (int color, int state, int interval);
ANSC_STATUS UpdateInformMsgToWanMgr();
#endif
#if defined (FEATURE_RDKB_WAN_AGENT)
static ANSC_STATUS CosaDmlEthSetParamValues(char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, unsigned int bCommitFlag);
#elif defined (FEATURE_RDKB_WAN_MANAGER)
static ANSC_STATUS CosaDmlEthSetParamValues(const char *pComponent, const char *pBus, const char *pParamName, const char *pParamVal, enum dataType_e type, unsigned int bCommitFlag);
static ANSC_STATUS DmlEthCheckIfaceConfiguredAsPPPoE( char *ifname, BOOL *isPppoeIface);
static ANSC_STATUS  GetWan_InterfaceName (char* wanoe_ifacename, int length);
static INT gTotal = TOTAL_NUMBER_OF_INTERNAL_INTERFACES;
#endif //FEATURE_RDKB_WAN_MANAGER

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
			OnboardLog("Device reboot due to reason WAN_Mode_Change\n");
                        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_LastRebootReason", "WAN_Mode_Change") != 0)
                        {
                                AnscTraceWarning(("RDKB_REBOOT : RebootDevice syscfg_set failed GUI\n"));
                        }

                        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_LastRebootCounter", "1") != 0)
                        {
                                AnscTraceWarning(("syscfg_set failed\n"));
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

int ethGetPHYRate
    (
        CCSP_HAL_ETHSW_PORT PortId
    )
{
    INT status                              = RETURN_ERR;
    CCSP_HAL_ETHSW_LINK_RATE LinkRate       = CCSP_HAL_ETHSW_LINK_NULL;
    CCSP_HAL_ETHSW_DUPLEX_MODE DuplexMode   = CCSP_HAL_ETHSW_DUPLEX_Auto;
    INT PHYRate                             = 0;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
    CCSP_HAL_ETHSW_LINK_STATUS  LinkStatus  = CCSP_HAL_ETHSW_LINK_Down;
#endif
    /* For Broadcom platform device, CcspHalEthSwGetPortStatus returns the Linkrate based
     * on the CurrentBitRate and CcspHalEthSwGetPortCfg returns the Linkrate based on the
     * MaximumBitRate. Hence CcspHalEthSwGetPortStatus called for Broadcom platform devices.
     */
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
    status = CcspHalEthSwGetPortStatus(PortId, &LinkRate, &DuplexMode, &LinkStatus);
    CcspTraceWarning(("CcspHalEthSwGetPortStatus link rate %d\n", LinkRate));
#else
    status = CcspHalEthSwGetPortCfg(PortId, &LinkRate, &DuplexMode);
     CcspTraceWarning(("CcspHalEthSwGetPortCfg link rate %d\n", LinkRate));
#endif

    if (RETURN_OK == status)
    {
     	CcspTraceInfo((" Entry %s \n", __FUNCTION__));
        switch (LinkRate)
        {
            case CCSP_HAL_ETHSW_LINK_10Mbps:
            {
                PHYRate = 10;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_100Mbps:
            {
                PHYRate = 100;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_1Gbps:
            {
                PHYRate = 1000;
                break;
            }
#ifdef _2_5G_ETHERNET_SUPPORT_
            case CCSP_HAL_ETHSW_LINK_2_5Gbps:
            {
                PHYRate = 2500;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_5Gbps:
            {
                PHYRate = 5000;
                break;
            }
#endif // _2_5G_ETHERNET_SUPPORT_
            case CCSP_HAL_ETHSW_LINK_10Gbps:
            {
                PHYRate = 10000;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_Auto:
            {
	            CcspTraceInfo((" Entry %s LINK_Auto \n", __FUNCTION__));
                PHYRate = 1000;
                break;
            }
            default:
            {
                PHYRate = 0;
                break;
            }
        }
    }
    return PHYRate;
}



#if defined (FEATURE_RDKB_WAN_MANAGER)

BOOL isEthWanEnabled()
{
    char buf[64];
    memset(buf,0,sizeof(buf));
    if (syscfg_get(NULL, "eth_wan_enabled", buf, sizeof(buf)) == 0)
    {
        if ( 0 == strcmp(buf,"true"))
        {

            if ( 0 == access( "/nvram/ETHWAN_ENABLE" , F_OK ) )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

#ifdef WAN_FAILOVER_SUPPORTED
static BOOLEAN isMTAblocked(char *ifname, int length,char *default_wan_ifname )
{
    if (ifname && isEthWanEnabled() )
    {
              if(strncmp(ifname,default_wan_ifname,length) != 0)
             {
			   CcspTraceWarning(("current_wan_ifname not equal to default_wan_ifname\n"));
               return TRUE; 
             }
    }
         return FALSE;
}



/*Checking the Current_wan_ifname Value*/
void *SysEventHandlerThrd(void *data)
{
    UNREFERENCED_PARAMETER(data);
    pthread_detach(pthread_self());
    async_id_t interface_asyncid;
    char default_wan_ifname[64];
    int err;
    char name[64] = {0}, ifname[64] = {0};
    CcspTraceWarning(("%s started\n",__FUNCTION__));
    sysevent_fd = sysevent_open("127.0.0.1", SE_SERVER_WELL_KNOWN_PORT, SE_VERSION, "MTAHandler", &sysevent_token);
    sysevent_set_options(sysevent_fd, sysevent_token, "current_wan_ifname", TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_fd, sysevent_token, "current_wan_ifname",  &interface_asyncid);
    memset(default_wan_ifname, 0, sizeof(default_wan_ifname));
    sysevent_get(sysevent_fd, sysevent_token, "wan_ifname", default_wan_ifname, sizeof(default_wan_ifname));
    while(1)
    {
        async_id_t getnotification_asyncid;
        memset(name,0,sizeof(name));
        memset(ifname,0,sizeof(ifname));
        int namelen = sizeof(name);
        int vallen  = sizeof(ifname);
        err = sysevent_getnotification(sysevent_fd, sysevent_token, name, &namelen, ifname , &vallen, &getnotification_asyncid);
        if (err)
        {
            CcspTraceWarning(("sysevent_getnotification failed with error: %d %s\n", err,__FUNCTION__));
            CcspTraceWarning(("sysevent_getnotification failed name: %s val : %s\n", name,ifname));
            if ( 0 != v_secure_system("pidof syseventd")) {
                CcspTraceWarning(("%s syseventd not running ,breaking the receive notification loop \n",__FUNCTION__));
                break;
            }
        }
        else
        {
           CcspTraceWarning(("%s Recieved notification event  %s for interface %s\n",__FUNCTION__,name,ifname));
           if(isMTAblocked(ifname,vallen,default_wan_ifname))
           {
               CcspTraceWarning(("%s Making %s down\n",__FUNCTION__,ETHWAN_DOCSIS_INF_NAME));
               v_secure_system("ifconfig %s down",ETHWAN_DOCSIS_INF_NAME); 
           }
           else
           {
               CcspTraceWarning(("%s Making %s up\n",__FUNCTION__,ETHWAN_DOCSIS_INF_NAME));
               v_secure_system("ifconfig %s up",ETHWAN_DOCSIS_INF_NAME);
           }
        }
    }
    return NULL;
}

/* Start event handler thread. */
static void TriggerSysEventMonitorThread(void)
{
    pthread_t EvtThreadId;
    int iErrorCode = 0;

    //Eth event handler thread
    iErrorCode = pthread_create(&EvtThreadId, NULL,&SysEventHandlerThrd ,NULL);

    if (0 != iErrorCode)
    {
        CcspTraceInfo(("%s %d - Failed to start Event Handler Thread EC:%d\n", __FUNCTION__, __LINE__, iErrorCode));
    }
    else
    {
        CcspTraceInfo(("%s %d - Event Handler Thread Started Successfully\n", __FUNCTION__, __LINE__));
    }
}
#endif        

#if defined(INTEL_PUMA7)
typedef enum
{
    NF_ARPTABLE,
    NF_IPTABLE,
    NF_IP6TABLE
} bridge_nf_table_t;

INT BridgeNfDisable( const char* bridgeName, bridge_nf_table_t table, BOOL disable )
{
    INT ret = 0;
    char disableFile[100] = {0};
    char* pTableName = NULL;

    if (bridgeName == NULL)
    {
        CcspTraceError(("bridgeName is NULLn"));
        return -1;
    }

    switch (table)
    {
        case NF_ARPTABLE:
            pTableName = "nf_disable_arptables";
            break;
        case NF_IPTABLE:
            pTableName = "nf_disable_iptables";
            break;
        case NF_IP6TABLE:
            pTableName = "nf_disable_ip6tables";
            break;
        default:
            // invalid table
            CcspTraceError(("Invalid table: %d\n", table));
            return -1;
    }

    snprintf(disableFile, sizeof(disableFile), "/sys/devices/virtual/net/%s/bridge/%s",bridgeName, pTableName);
    int fd = open(disableFile, O_WRONLY);
    if (fd != -1)
    {
        ret = 0;
        char val = disable ? '1' : '0';
        int num = write(fd, &val, 1);
        if (num != 1)
        {
            CcspTraceError(("Failed to write: %d\n", num));
            ret = -1;
        }

        close(fd);
    }
    else
    {
        CcspTraceError(("Failed to open %s\n", disableFile));
        ret = -1;
    }

    return ret;
}

INT WanBridgeConfigurationIntelPuma7(WAN_MODE_BRIDGECFG *pCfg)
{
    if (!pCfg)
        return -1;

    if (pCfg->ethWanEnabled == TRUE)
    {   
        if (pCfg->ovsEnabled == TRUE)
        {
            v_secure_system("/usr/bin/bridgeUtils del-port brlan0 %s",pCfg->ethwan_ifname);
        }
        else
        {
            v_secure_system("brctl delif brlan0 %s",pCfg->ethwan_ifname);  

        }


        if (TRUE == pCfg->configureBridge)
        {
            macaddr_t macAddr;
            char wan_mac[18];
    
            if (TRUE == pCfg->meshEbEnabled)
            {
                v_secure_system("/bin/sh /etc/utopia/service.d/vlan_util_xb7.sh meshethbhaul-removeEthwan 0");
            }
            v_secure_system("ifconfig %s down",pCfg->ethwan_ifname);
            v_secure_system("ip addr flush dev %s",pCfg->ethwan_ifname);
            v_secure_system("ip -6 addr flush dev %s",pCfg->ethwan_ifname);
            v_secure_system("sysctl -w net.ipv6.conf.%s.accept_ra=0",pCfg->ethwan_ifname);
            v_secure_system("ifconfig %s down; ip link set %s name dummy-rf", pCfg->wanPhyName,pCfg->wanPhyName);

            v_secure_system("brctl addbr %s", pCfg->wanPhyName);
            v_secure_system("brctl addif %s %s", pCfg->wanPhyName,pCfg->ethwan_ifname);
            v_secure_system("sysctl -w net.ipv6.conf.%s.autoconf=0", pCfg->ethwan_ifname); 
            v_secure_system("sysctl -w net.ipv6.conf.%s.disable_ipv6=1", pCfg->ethwan_ifname);
            v_secure_system("ip6tables -I OUTPUT -o %s -p icmpv6 -j DROP", pCfg->ethwan_ifname);
            if (0 != pCfg->bridgemode)
            {
                v_secure_system("/bin/sh /etc/utopia/service.d/service_bridge_puma7.sh bridge-restart");
            }

            BridgeNfDisable(pCfg->wanPhyName, NF_ARPTABLE, TRUE);
            BridgeNfDisable(pCfg->wanPhyName, NF_IPTABLE, TRUE);
            BridgeNfDisable(pCfg->wanPhyName, NF_IP6TABLE, TRUE);

            v_secure_system("ip link set %s up",ETHWAN_DOCSIS_INF_NAME);
            v_secure_system("brctl addif %s %s", pCfg->wanPhyName,ETHWAN_DOCSIS_INF_NAME);
            v_secure_system("sysctl -w net.ipv6.conf.%s.disable_ipv6=1",ETHWAN_DOCSIS_INF_NAME);

            memset(&macAddr,0,sizeof(macaddr_t));
            getInterfaceMacAddress(&macAddr,"dummy-rf"); //dummy-rf is renamed from erouter0
            memset(wan_mac,0,sizeof(wan_mac));
            snprintf(wan_mac, sizeof(wan_mac), "%02x:%02x:%02x:%02x:%02x:%02x", macAddr.hw[0], macAddr.hw[1], macAddr.hw[2],
                    macAddr.hw[3], macAddr.hw[4], macAddr.hw[5]);
            v_secure_system("ifconfig %s down", pCfg->wanPhyName);
            v_secure_system("ifconfig %s hw ether %s",  pCfg->wanPhyName,wan_mac);
            v_secure_system("echo %s > /sys/bus/platform/devices/toe/in_if", pCfg->wanPhyName);
            v_secure_system("ifconfig %s up", pCfg->wanPhyName);
        }
        else
        {
            v_secure_system("brctl addif %s %s", pCfg->wanPhyName,pCfg->ethwan_ifname);
        }
        
        v_secure_system("ifconfig %s up",pCfg->ethwan_ifname);
        v_secure_system("cmctl down");

    }
    else
    {
        v_secure_system("brctl delif %s %s", pCfg->wanPhyName,pCfg->ethwan_ifname);

        if (pCfg->ovsEnabled)
        {
            v_secure_system("/usr/bin/bridgeUtils add-port brlan0 %s",pCfg->ethwan_ifname);
        }
        else
        {
            v_secure_system("brctl addif brlan0 %s",pCfg->ethwan_ifname);
        }
        
        if (TRUE == pCfg->configureBridge)        
        {
            if (TRUE == pCfg->meshEbEnabled)
            {
                v_secure_system("/bin/sh /etc/utopia/service.d/vlan_util_xb7.sh meshethbhaul-up 0");
            }
            if (0 != pCfg->bridgemode)
            {
                v_secure_system("/bin/sh /etc/utopia/service.d/service_bridge_puma7.sh bridge-restart");
            }

            v_secure_system("brctl delif %s %s", pCfg->wanPhyName,ETHWAN_DOCSIS_INF_NAME);
            v_secure_system("ifconfig %s down", pCfg->wanPhyName);
            v_secure_system("brctl delbr %s", pCfg->wanPhyName);
            v_secure_system("ip link set dummy-rf name %s", pCfg->wanPhyName);
            v_secure_system("echo %s > /sys/bus/platform/devices/toe/in_if", pCfg->wanPhyName);
            v_secure_system("ifconfig %s up", pCfg->wanPhyName);
       }
    }

    return 0;

}
#endif

#if defined (_COSA_BCM_ARM_)
INT WanBridgeConfigurationBcm(WAN_MODE_BRIDGECFG *pCfg)
{
    if (!pCfg)
        return -1;

    if (pCfg->ethWanEnabled == TRUE)
    { 
        if (pCfg->ovsEnabled == TRUE)
        {
            v_secure_system("/usr/bin/bridgeUtils del-port brlan0 %s",pCfg->ethwan_ifname);
        }
        else
        {
            v_secure_system("brctl delif brlan0 %s",pCfg->ethwan_ifname);  

        }       

        if (TRUE == pCfg->configureBridge)
        {
            v_secure_system("ifconfig %s down",pCfg->ethwan_ifname);
            v_secure_system("ip addr flush dev %s",pCfg->ethwan_ifname);
            v_secure_system("ip -6 addr flush dev %s",pCfg->ethwan_ifname);
            v_secure_system("sysctl -w net.ipv6.conf.%s.accept_ra=0",pCfg->ethwan_ifname);

            if (0 == pCfg->bridgemode)
            {
                CcspTraceInfo(("set CM0 %s wanif %s\n",ETHWAN_DOCSIS_INF_NAME,pCfg->wanPhyName));
                v_secure_system("ifconfig %s down", pCfg->wanPhyName);
                v_secure_system("ip link set %s name %s", pCfg->wanPhyName,ETHWAN_DOCSIS_INF_NAME);
                v_secure_system("brctl addbr %s", pCfg->wanPhyName);
                v_secure_system("brctl addif %s %s", pCfg->wanPhyName,pCfg->ethwan_ifname);

                v_secure_system("sysctl -w net.ipv6.conf.%s.autoconf=0", pCfg->ethwan_ifname); 
                v_secure_system("sysctl -w net.ipv6.conf.%s.disable_ipv6=1", pCfg->ethwan_ifname);
                v_secure_system("ip6tables -I OUTPUT -o %s -p icmpv6 -j DROP", pCfg->ethwan_ifname);
                v_secure_system("ip link set %s up",ETHWAN_DOCSIS_INF_NAME);
                v_secure_system("brctl addif %s %s", pCfg->wanPhyName,ETHWAN_DOCSIS_INF_NAME);
                v_secure_system("sysctl -w net.ipv6.conf.%s.disable_ipv6=1",ETHWAN_DOCSIS_INF_NAME);            
            }
            else
            {
#if defined (ENABLE_WANMODECHANGE_NOREBOOT)
                v_secure_system("touch /tmp/wanmodechange");
                if (pCfg->ovsEnabled)
                {
                    v_secure_system("/usr/bin/bridgeUtils multinet-syncMembers 1");
                    v_secure_system("/usr/bin/bridgeUtils del-port brlan0 %s",pCfg->ethwan_ifname);
                }
                else
                {
                    v_secure_system("/bin/sh /etc/utopia/service.d/vlan_util_tchxb6.sh multinet-syncMembers 1");
                    v_secure_system("brctl delif brlan0 %s",pCfg->ethwan_ifname);
                }
                v_secure_system("rm /tmp/wanmodechange");
#endif

                v_secure_system("brctl addif %s %s", pCfg->wanPhyName,pCfg->ethwan_ifname);
            }
        }
        else
        {
            v_secure_system("brctl addif %s %s", pCfg->wanPhyName,pCfg->ethwan_ifname);
        }
        
        v_secure_system("ifconfig %s up",pCfg->ethwan_ifname);

    }
    else
    {
        v_secure_system("brctl delif %s %s", pCfg->wanPhyName,pCfg->ethwan_ifname);

        if (pCfg->ovsEnabled)
        {
            v_secure_system("/usr/bin/bridgeUtils add-port brlan0 %s",pCfg->ethwan_ifname);
        }
        else
        {
            v_secure_system("brctl addif brlan0 %s",pCfg->ethwan_ifname);
        }
        if (TRUE == pCfg->configureBridge)        
        {
            if (0 == pCfg->bridgemode)
            {

                v_secure_system("brctl delif %s %s", pCfg->wanPhyName,ETHWAN_DOCSIS_INF_NAME);
                v_secure_system("ifconfig %s down", pCfg->wanPhyName);
                v_secure_system("brctl delbr %s", pCfg->wanPhyName);
                v_secure_system("ifconfig %s down", ETHWAN_DOCSIS_INF_NAME);
                v_secure_system("ip link set %s name %s", ETHWAN_DOCSIS_INF_NAME,pCfg->wanPhyName);
                v_secure_system("ifconfig %s up", pCfg->wanPhyName);
                v_secure_system("echo addif %s > /proc/driver/flowmgr/cmd", pCfg->wanPhyName);
                v_secure_system("echo delif %s > /proc/driver/flowmgr/cmd", ETHWAN_DOCSIS_INF_NAME);
            }
            else
            {
#if defined (ENABLE_WANMODECHANGE_NOREBOOT)
                v_secure_system("touch /tmp/wanmodechange");
                if (pCfg->ovsEnabled)
                {
                    v_secure_system("/usr/bin/bridgeUtils multinet-syncMembers 1");
                }
                else
                {
                    v_secure_system("/bin/sh /etc/utopia/service.d/vlan_util_tchxb6.sh multinet-syncMembers 1");
                }
                v_secure_system("rm /tmp/wanmodechange");
#endif
            }

        }
    }

    return 0;
}
#endif

ANSC_STATUS CosaDmlIfaceFinalize(char *pValue, BOOL isAutoWanMode)
{
    char wanPhyName[64] = {0};
    char buf[64] = {0};
    char ethwan_ifname[64] = {0};
    BOOL ovsEnabled = FALSE;
    BOOL ethwanEnabled = FALSE;
    BOOL meshEbEnabled = FALSE;
    INT lastKnownWanMode = -1;
    BOOL configureBridge = FALSE;
    INT bridgemode = 0;
    WanBridgeCfgHandler pCfgHandler = NULL;
    WAN_MODE_BRIDGECFG wanModeCfg = {0};

    if (!pValue)
        return CCSP_FAILURE;
    CcspTraceInfo(("Func %s Entered\n",__FUNCTION__));
#if defined(INTEL_PUMA7)
    pCfgHandler = WanBridgeConfigurationIntelPuma7;
#elif defined (_COSA_BCM_ARM_)
    pCfgHandler = WanBridgeConfigurationBcm;
#endif

    ethwanEnabled = isEthWanEnabled();

#if defined (_BRIDGE_UTILS_BIN_)
    if( 0 == syscfg_get( NULL, "mesh_ovs_enable", buf, sizeof( buf ) ) )
    {
          if ( strcmp (buf,"true") == 0 )
            ovsEnabled = TRUE;
          else
            ovsEnabled = FALSE;

    }
    else
    {
          CcspTraceError(("syscfg_get failed to retrieve ovs_enable\n"));

    }
#endif

    memset(buf,0,sizeof(buf));
    if (syscfg_get(NULL, "bridge_mode", buf, sizeof(buf)) == 0)
    {
        bridgemode = atoi(buf);
    }

    memset(buf,0,sizeof(buf));
    if (syscfg_get(NULL, "eb_enable", buf, sizeof(buf)) == 0)
    {
        if (strcmp(buf,"true") == 0)
        {
            meshEbEnabled = TRUE;
        }
        else
        {
            meshEbEnabled = FALSE;
        }
    }


    memset(buf,0,sizeof(buf));
    if (!syscfg_get(NULL, "wan_physical_ifname", buf, sizeof(buf)))
    {
        strcpy(wanPhyName, buf);
        printf("wanPhyName = %s\n", wanPhyName);
    }
    else
    {
        strcpy(wanPhyName, "erouter0");

    }

    // Do wan interface bridge creation/deletion only if last detected wan and current detected wan interface are different.
    // Hence deciding here whether wan bridge creation/deletion (configureBridge) is
    // needed or not based on "lastKnownWanMode and ethwanEnabled" value.
    memset(buf,0,sizeof(buf));
    if (syscfg_get(NULL, "last_wan_mode", buf, sizeof(buf)) == 0)
    {
        lastKnownWanMode = atoi(buf);
    }

    // last  known mode will be updated by wan manager after 
    // this finalize api operation done. Till that lastknownmode will be previous detected wan.
    switch (lastKnownWanMode)
    {
        case WAN_MODE_DOCSIS:
            {
                if (ethwanEnabled == TRUE)
                {
                    configureBridge = TRUE;
                }
            }
            break;
        case WAN_MODE_ETH:
            {
                if (ethwanEnabled == FALSE)
                {
                    configureBridge = TRUE;
                }
            }
            break;
        default:
            {
                // if last known mode is unknown, then default last wan mode is primary wan.
                // Hence if ethwan is enabled when last known mode is primary, then configure bridge.
                // Note: Configure bridge will need to do when prev wan mode and new wan mode is different.
                if (ethwanEnabled == TRUE)
                {
                    configureBridge = TRUE;
                }
            }
            break;
    }

    CcspTraceInfo((" %s isAutoWanMode: %d lastknownmode: %d ethwanEnabled: %d ConfigureBridge: %d bridgemode: %d\n",
                __FUNCTION__,
                isAutoWanMode,
                lastKnownWanMode,
                ethwanEnabled,
                configureBridge,
                bridgemode));

        //Get the ethwan interface name from HAL
    memset( ethwan_ifname , 0, sizeof( ethwan_ifname ) );
    if ((0 != GWP_GetEthWanInterfaceName((unsigned char*) ethwan_ifname, sizeof(ethwan_ifname)))
            || (0 == strnlen(ethwan_ifname,sizeof(ethwan_ifname)))
            || (0 == strncmp(ethwan_ifname,"disable",sizeof(ethwan_ifname))))

    {
        //Fallback case needs to set it default
        memset( ethwan_ifname , 0, sizeof( ethwan_ifname ) );
        sprintf( ethwan_ifname , "%s", ETHWAN_DEF_INTF_NAME );
    }
    wanModeCfg.bridgemode = bridgemode;
    wanModeCfg.ovsEnabled = ovsEnabled;
    wanModeCfg.ethWanEnabled = ethwanEnabled;
    wanModeCfg.meshEbEnabled = meshEbEnabled;
    wanModeCfg.configureBridge = configureBridge;
    snprintf(wanModeCfg.wanPhyName,sizeof(wanModeCfg.wanPhyName),"%s",wanPhyName);
    snprintf(wanModeCfg.ethwan_ifname,sizeof(wanModeCfg.ethwan_ifname),"%s",ethwan_ifname);

    if (pCfgHandler)
    {
        pCfgHandler(&wanModeCfg);
    }

    if (TRUE == isAutoWanMode)
    {
        v_secure_system("sysevent set phylink_wan_state up");
        if ( 0 != access( "/tmp/autowan_iface_finalized" , F_OK ) )
        {
            v_secure_system("touch /tmp/autowan_iface_finalized");
        }
        UpdateInformMsgToWanMgr();
    
    }
    return ANSC_STATUS_SUCCESS;
}


ANSC_STATUS EthwanEnableWithoutReboot(BOOL bEnable,INT bridge_mode)
{
    CcspTraceInfo(("Func %s Entered arg %d bridge mode %d\n",__FUNCTION__,bEnable,bridge_mode));
    
    if(bEnable == FALSE && (bridge_mode == 0))
    {
        v_secure_system("ip link set %s down",WAN_IF_NAME_PRIMARY);
        v_secure_system("ip link set %s name %s",WAN_IF_NAME_PRIMARY,ETHWAN_DEF_INTF_NAME);
        v_secure_system("ip link set dummy-rf name %s",WAN_IF_NAME_PRIMARY);
        v_secure_system("ip link set %s up;ip link set %s up",ETHWAN_DEF_INTF_NAME,WAN_IF_NAME_PRIMARY);
    } 

    CcspHalExtSw_setEthWanPort ( ETHWAN_DEF_INTF_NUM );

    if ( RETURN_OK == CcspHalExtSw_setEthWanEnable( bEnable ) ) 
    {
        //pthread_t tid;        
        char buf[ 8 ];
        memset( buf, 0, sizeof( buf ) );
        snprintf( buf, sizeof( buf ), "%s", bEnable ? "true" : "false" );
        if(bEnable)
        {
            v_secure_system("touch /nvram/ETHWAN_ENABLE");
        }
        else
        {
            v_secure_system("rm /nvram/ETHWAN_ENABLE");
        }

        if ( syscfg_set_commit( NULL, "eth_wan_enabled", buf ) != 0 )
        {
            CcspTraceError(( "syscfg_set failed for eth_wan_enabled\n" ));
            return ANSC_STATUS_FAILURE;
        }
    }
    else
    {
        CcspTraceError(("Func %s CcspHalExtSw_setEthWanEnable failed  arg: %d\n",__FUNCTION__,bEnable));
        return ANSC_STATUS_FAILURE;
    }
    CcspTraceInfo(("Func %s Exited arg %d\n",__FUNCTION__,bEnable));
 
    return ANSC_STATUS_SUCCESS;
}

BOOL CosaDmlEthWanLinkStatus()
{
    CCSP_HAL_ETHSW_PORT         port;
    INT                         status;
    CCSP_HAL_ETHSW_LINK_RATE    LinkRate;
    CCSP_HAL_ETHSW_DUPLEX_MODE  DuplexMode;
    CCSP_HAL_ETHSW_LINK_STATUS  LinkStatus;

    port = ETHWAN_DEF_INTF_NUM;

    port += CCSP_HAL_ETHSW_EthPort1; /* ETH WAN HALs start from 0 but Ethernet Switch HALs start with 1*/

    status = CcspHalEthSwGetPortStatus(port, &LinkRate, &DuplexMode, &LinkStatus);

    if ( status == RETURN_OK )
    {
        if (LinkStatus == CCSP_HAL_ETHSW_LINK_Up)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static void checkComponentHealthStatus(char * compName, char * dbusPath, char *status, int *retStatus)
{
    int ret = 0, val_size = 0;
    parameterValStruct_t **parameterval = NULL;
    char *parameterNames[1] = {};
    char tmp[256];
    char str[256];
    char l_Subsystem[128] = { 0 };

    sprintf(tmp,"%s.%s",compName, "Health");
    parameterNames[0] = tmp;

    strncpy(l_Subsystem, "eRT.",sizeof(l_Subsystem));
    snprintf(str, sizeof(str), "%s%s", l_Subsystem, compName);
    CcspTraceDebug(("str is:%s\n", str));

    ret = CcspBaseIf_getParameterValues(bus_handle, str, dbusPath,  parameterNames, 1, &val_size, &parameterval);
    CcspTraceDebug(("ret = %d val_size = %d\n",ret,val_size));
    if(ret == CCSP_SUCCESS)
    {
        CcspTraceDebug(("parameterval[0]->parameterName : %s parameterval[0]->parameterValue : %s\n",parameterval[0]->parameterName,parameterval[0]->parameterValue));
        strncpy(status, parameterval[0]->parameterValue, STATUS_BUFF_SIZE - 1);
        CcspTraceDebug(("status of component:%s\n", status));
    }
    free_parameterValStruct_t (bus_handle, val_size, parameterval);

    *retStatus = ret;
}


static void waitForWanMgrComponentReady()
{
    char status[STATUS_BUFF_SIZE] = {'\0'};
    int count = 0;
    int ret = -1;
    while(1)
    {
        checkComponentHealthStatus(WAN_COMP_NAME_WITHOUT_SUBSYSTEM, WAN_DBUS_PATH, status,&ret);
        if(ret == CCSP_SUCCESS && (strcmp(status, "Green") == 0))
        {
            CcspTraceInfo(("%s component health is %s, continue\n", WAN_COMPONENT_NAME, status));
            break;
        }
        else
        {
            count++;
            if(count%5 == 0)
            {
                CcspTraceError(("%s component Health, ret:%d, waiting\n", WAN_COMPONENT_NAME, ret));
            }
            sleep(5);
        }
    }
}

INT InitBootInformInfo(WAN_BOOTINFORM_MSG *pMsg)
{
    BOOL bEthWanEnable = FALSE;
    CHAR wanName[64];
    CHAR ethWanName[64];
    CHAR out_value[64];
    CHAR acSetParamName[256];
    INT iWANInstance = 0;
    char buf[64];

    if (!pMsg)
        return -1;
    memset(buf,0,sizeof(buf));
    if (syscfg_get(NULL, "eth_wan_enabled", buf, sizeof(buf)) == 0)
    {
        if ( 0 == strcmp(buf,"true"))
        {

            if ( 0 == access( "/nvram/ETHWAN_ENABLE" , F_OK ) )
            {
                bEthWanEnable = TRUE;
            }
        }
    }

    memset(out_value,0,sizeof(out_value));
    memset(wanName,0,sizeof(wanName));
    memset(ethWanName,0,sizeof(ethWanName));
    syscfg_get(NULL, "wan_physical_ifname", out_value, sizeof(out_value));

    if(0 != strnlen(out_value,sizeof(out_value)))
    {
        snprintf(wanName, sizeof(wanName), "%s", out_value);
    }
    else
    {
        snprintf(wanName, sizeof(wanName), "%s", WAN_IF_NAME_PRIMARY);
    }

    pMsg->iWanInstanceNumber = WAN_ETH_INTERFACE_INSTANCE_NUM;
    pMsg->iNumOfParam = MSG_TOTAL_NUM;

    if ( (0 != GWP_GetEthWanInterfaceName((unsigned char*)ethWanName, sizeof(ethWanName)))
            || (0 == strnlen(ethWanName,sizeof(ethWanName)))
            || (0 == strncmp(ethWanName,"disable",sizeof(ethWanName)))
       )
    {
        /* Fallback case needs to set it default */
        snprintf(ethWanName ,sizeof(ethWanName), "%s", ETHWAN_DEF_INTF_NAME);
    }

    if (bEthWanEnable == TRUE)
    {
        strncpy(pMsg->param[MSG_WAN_NAME].paramValue,wanName,sizeof(pMsg->param[MSG_WAN_NAME].paramValue) - 1);
    }
    else
    {
        strncpy(pMsg->param[MSG_WAN_NAME].paramValue,ethWanName,sizeof(pMsg->param[MSG_WAN_NAME].paramValue) - 1);
    }
 
    CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_WAN_AGENT, ethWanName, &iWANInstance);

    if (-1 != iWANInstance)
    {
        pMsg->iWanInstanceNumber = iWANInstance;
    }
    else
    {
        pMsg->iWanInstanceNumber = WAN_ETH_INTERFACE_INSTANCE_NUM;
    }

    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_BOOTINFORM_INTERFACE_PARAM_NAME,  pMsg->iWanInstanceNumber);
    strncpy(pMsg->param[MSG_WAN_NAME].paramName,acSetParamName,sizeof(pMsg->param[MSG_WAN_NAME].paramName) - 1);
    pMsg->param[MSG_WAN_NAME].paramType = ccsp_string;

    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_BOOTINFORM_PHYPATH_PARAM_NAME,  pMsg->iWanInstanceNumber);
    strncpy(pMsg->param[MSG_PHY_PATH].paramName,acSetParamName,sizeof(pMsg->param[MSG_PHY_PATH].paramName) - 1);
    strncpy(pMsg->param[MSG_PHY_PATH].paramValue,WAN_PHYPATH_VALUE,sizeof(pMsg->param[MSG_PHY_PATH].paramValue) - 1);
    pMsg->param[MSG_PHY_PATH].paramType = ccsp_string;

    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_BOOTINFORM_OPERSTATUSENABLE_PARAM_NAME,  pMsg->iWanInstanceNumber);
    strncpy(pMsg->param[MSG_OPER_STATUS].paramName,acSetParamName,sizeof(pMsg->param[MSG_OPER_STATUS].paramName) - 1);
    strncpy(pMsg->param[MSG_OPER_STATUS].paramValue,"false",sizeof(pMsg->param[MSG_OPER_STATUS].paramValue) - 1);
    pMsg->param[MSG_OPER_STATUS].paramType = ccsp_boolean;

    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_BOOTINFORM_CONFIGWANENABLE_PARAM_NAME,  pMsg->iWanInstanceNumber);
    strncpy(pMsg->param[MSG_CONFIGURE_WAN].paramName,acSetParamName,sizeof(pMsg->param[MSG_CONFIGURE_WAN].paramName) - 1);
    strncpy(pMsg->param[MSG_CONFIGURE_WAN].paramValue,"true",sizeof(pMsg->param[MSG_CONFIGURE_WAN].paramValue) - 1);
    pMsg->param[MSG_CONFIGURE_WAN].paramType = ccsp_boolean;

    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_BOOTINFORM_CUSTOMCONFIG_PARAM_NAME,  pMsg->iWanInstanceNumber);
    strncpy(pMsg->param[MSG_CUSTOMCONFIG_ENABLE].paramName,acSetParamName,sizeof(pMsg->param[MSG_CUSTOMCONFIG_ENABLE].paramName) - 1);
    strncpy(pMsg->param[MSG_CUSTOMCONFIG_ENABLE].paramValue,"true",sizeof(pMsg->param[MSG_CUSTOMCONFIG_ENABLE].paramValue) - 1);
    pMsg->param[MSG_CUSTOMCONFIG_ENABLE].paramType = ccsp_boolean;

    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_BOOTINFORM_CUSTOMCONFIGPATH_PARAM_NAME,  pMsg->iWanInstanceNumber);
    strncpy(pMsg->param[MSG_CUSTOMCONFIG_PATH].paramName,acSetParamName,sizeof(pMsg->param[MSG_CUSTOMCONFIG_PATH].paramName) - 1);
    strncpy(pMsg->param[MSG_CUSTOMCONFIG_PATH].paramValue,WAN_BOOTINFORM_CUSTOMCONFIGPATH_PARAM_VALUE,sizeof(pMsg->param[MSG_CUSTOMCONFIG_PATH].paramValue) - 1);
    pMsg->param[MSG_CUSTOMCONFIG_PATH].paramType = ccsp_string;

   
    return 0;
}

void* ThreadBootInformMsg(void *arg)
{
    WAN_BOOTINFORM_MSG msg = {0};
    INT retryMax = 60;
    INT retryCount = 0;
    BOOL retryBootInform = FALSE;
    pthread_detach(pthread_self());
    waitForWanMgrComponentReady();
    InitBootInformInfo(&msg);
    while (1)
    {
        INT index = 0;
        for (index = 0; index < msg.iNumOfParam; ++index)
        {
            ANSC_STATUS ret = CosaDmlEthSetParamValues(WAN_COMPONENT_NAME,
                    WAN_DBUS_PATH,
                    msg.param[index].paramName,
                    msg.param[index].paramValue,
                    msg.param[index].paramType,
                    TRUE);
            if (ret == ANSC_STATUS_FAILURE)
            {
                retryBootInform = TRUE;
                break;
            }
        }
        if (FALSE == retryBootInform)
        {
            break;
        }
        if (retryCount > retryMax)
            break;
        ++retryCount;
        sleep(1);
    }
    return arg;
}

void* ThreadMonitorPhyAndNotify(void *arg)
{
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)arg;
    INT EthWanInstanceNumber = WAN_ETH_INTERFACE_INSTANCE_NUM;
    pthread_detach(pthread_self());
    if (pMyObject)
    {
        PCOSA_DATAMODEL_ETH_WAN_AGENT pEthWanCfg = (PCOSA_DATAMODEL_ETH_WAN_AGENT)&pMyObject->EthWanCfg;
        if (pEthWanCfg)
        {
            INT counter = 0;
            BOOL phyStatus = FALSE;
            char acSetParamName[256];
            char acTmpPhyStatus[32] = {0};

            if (pEthWanCfg->wanInstanceNumber)
            {
                EthWanInstanceNumber = atoi(pEthWanCfg->wanInstanceNumber);
            }

            while(1)
            {
                // ethoff file only for debugging to simulate link down case.
                if ( 0 == access( "/tmp/ethoff" , F_OK ) )
                 {
                    phyStatus = FALSE;
                    sleep(1);
                    continue;                    
                 }
                if (CosaDmlEthWanLinkStatus() == TRUE)
                {
                    phyStatus = TRUE;
                    break;
                }
                if (counter >= PHY_STATUS_MONITOR_MAX_TIMEOUT)
                {
                    break;
                }
                sleep(PHY_STATUS_QUERY_INTERVAL);
                counter+=PHY_STATUS_QUERY_INTERVAL;
            }
            memset(acSetParamName, 0, sizeof(acSetParamName));
            snprintf(acSetParamName, sizeof(acSetParamName), WAN_PHY_STATUS_PARAM_NAME, EthWanInstanceNumber);

            if (phyStatus == TRUE)
            {
                 snprintf(acTmpPhyStatus, sizeof(acTmpPhyStatus), "%s", "Up");
            }
            else
            {
                 snprintf(acTmpPhyStatus, sizeof(acTmpPhyStatus), "%s", "Down");
            }
            CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acTmpPhyStatus, ccsp_string, TRUE);
            pEthWanCfg->MonitorPhyStatusAndNotify = FALSE;
        }
    }
    return arg;
}

ANSC_STATUS
CosaDmlEthWanPhyStatusMonitor(void *args)
{
    PCOSA_DATAMODEL_ETHERNET pObject = (PCOSA_DATAMODEL_ETHERNET) args;
 	pthread_t tidMonitor;
    pthread_create ( &tidMonitor, NULL, &ThreadMonitorPhyAndNotify, pObject);
    return ANSC_STATUS_SUCCESS;
}

void* ThreadUpdateInformMsg(void *arg)
{
    WAN_BOOTINFORM_MSG msg = {0};
    pthread_detach(pthread_self());
    InitBootInformInfo(&msg);
    ANSC_STATUS ret = CosaDmlEthSetParamValues(WAN_COMPONENT_NAME,
            WAN_DBUS_PATH,
            msg.param[MSG_WAN_NAME].paramName,
            msg.param[MSG_WAN_NAME].paramValue,
            msg.param[MSG_WAN_NAME].paramType,
            TRUE);
    if (ret != ANSC_STATUS_SUCCESS)
    {
         CcspTraceError(("UpdateInformMsg to wanmanager is failed for param %s\n",msg.param[MSG_WAN_NAME].paramName));
    }

    return arg;
}

ANSC_STATUS UpdateInformMsgToWanMgr()
{
    pthread_t tidInformMsg;
    pthread_create ( &tidInformMsg, NULL, &ThreadUpdateInformMsg, NULL);
    return ANSC_STATUS_SUCCESS;
}

void* ThreadConfigEthWan(void *arg)
{
    BOOL *pValue = NULL;
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    PCOSA_DATAMODEL_ETH_WAN_AGENT pEthWanCfgObj = NULL;
    if (pMyObject)
    {
        pEthWanCfgObj = (PCOSA_DATAMODEL_ETH_WAN_AGENT)&pMyObject->EthWanCfg;
    }

    pthread_detach(pthread_self());

    pValue = (BOOL*)arg;

    if(pValue == NULL)
        return NULL;

    if (pEthWanCfgObj)
    {
        pEthWanCfgObj->wanOperState = WAN_OPER_STATE_RESET_INPROGRESS;
    }
    // Request wan manager to disable wan.
    ret = CosaDmlEthSetParamValues(WAN_COMPONENT_NAME,
            WAN_DBUS_PATH,
            WAN_ENABLE_PARAM,
            "false",
            ccsp_boolean,
            TRUE);
    if (ret != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("UpdateInformMsg to wanmanager is failed for param %s false\n",WAN_ENABLE_PARAM));
    }
    sleep(2); // wait randomly some seconds till wan manager tear down the states.
    if ( ANSC_STATUS_SUCCESS != CosaDmlConfigureEthWan(*pValue))
    {
        CcspTraceError(("CosaDmlConfigureEthWan failed %d revert %d \n",*pValue,!*pValue));

        // rollback configure to previous mode in case of failure.
        if (syscfg_set_u_commit(NULL, "selected_wan_mode", pEthWanCfgObj->PrevSelMode) != 0)
        {
            CcspTraceError(("syscfg_set failed\n"));
        }

        CosaDmlConfigureEthWan(!*pValue);
        CosaDmlIfaceFinalize("2",FALSE);
    }
    else
    {
        CosaDmlIfaceFinalize("2",FALSE); // passing wan instance number for ethwan interface
        v_secure_system("sysevent set phylink_wan_state up");
        if (*pValue == TRUE)
        {
            v_secure_system("sysevent set ethwan-initialized 1");
            v_secure_system("syscfg set ntp_enabled 1"); // Enable NTP in case of ETHWAN
            v_secure_system("syscfg commit"); 
        }
        else
        {
            v_secure_system("sysevent set ethwan-initialized 0");
        }
    }
    // Request wan manager to enable wan.
    ret = CosaDmlEthSetParamValues(WAN_COMPONENT_NAME,
            WAN_DBUS_PATH,
            WAN_ENABLE_PARAM,
            "true",
            ccsp_boolean,
            TRUE);
    if (ret != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("UpdateInformMsg to wanmanager is failed for param %s true\n",WAN_ENABLE_PARAM));
    }
   
    sleep(2);// wait randomly some seconds till wan manager resume its states.
    if (pEthWanCfgObj)
    {

        pEthWanCfgObj->wanOperState = WAN_OPER_STATE_RESET_COMPLETED;
    }
 
    free(pValue);
    return arg;
}

ANSC_STATUS ConfigEthWan(BOOL bValue)
{
    pthread_t tidEthWanCfg;
    BOOL *pValue = malloc(sizeof(BOOL));
    if (pValue != NULL)
    {
        *pValue = bValue;
        pthread_create ( &tidEthWanCfg, NULL, &ThreadConfigEthWan, pValue);
    }
    return ANSC_STATUS_SUCCESS;
}


ANSC_STATUS CosaDmlConfigureEthWan(BOOL bEnable)
{
    char wanPhyName[64] = {0};
    char out_value[64] = {0};
    char buf[64] = {0};
    char ethwanMac[32] = {0};
    char ethwan_ifname[64] = {0};
    int lastKnownWanMode = -1;
    PCOSA_DATAMODEL_ETHERNET    pEthernet = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    COSA_DATAMODEL_ETH_WAN_AGENT ethWanCfg = {0};
    int bridgemode = 0;
    int retvalue = -1;
#if defined (_BRIDGE_UTILS_BIN_)
    int ovsEnable = 0;
    if( 0 == syscfg_get( NULL, "mesh_ovs_enable", buf, sizeof( buf ) ) )
    {
          if ( strcmp (buf,"true") == 0 )
            ovsEnable = 1;
          else
            ovsEnable = 0;

    }
    else
    {
          CcspTraceError(("syscfg_get failed to retrieve ovs_enable\n"));

    }
#endif

    CcspTraceInfo(("Func %s Entered arg %d\n",__FUNCTION__,bEnable));

    memset(buf,0,sizeof(buf));
    if (syscfg_get(NULL, "bridge_mode", buf, sizeof(buf)) == 0)
    {
        bridgemode = atoi(buf);
    }

    memset(buf,0,sizeof(buf));
    if (syscfg_get(NULL, "last_wan_mode", buf, sizeof(buf)) == 0)
    {
        lastKnownWanMode = atoi(buf);
    }

    syscfg_get(NULL, "wan_physical_ifname", out_value, sizeof(out_value));


    if(0 != strnlen(out_value,sizeof(out_value)))
    {
        snprintf(wanPhyName, sizeof(wanPhyName), "%s", out_value);
    }
    else
    {
        snprintf(wanPhyName, sizeof(wanPhyName), "%s", WAN_IF_NAME_PRIMARY);
    }
    CcspTraceError(("%s - In Arg %d Last KnownMode %d \n",__FUNCTION__,bEnable,lastKnownWanMode));

    if (bEnable == TRUE)
    {

        if ( (0 != GWP_GetEthWanInterfaceName((unsigned char*)ethwan_ifname, sizeof(ethwan_ifname)))
                || (0 == strnlen(ethwan_ifname,sizeof(ethwan_ifname)))
                || (0 == strncmp(ethwan_ifname,"disable",sizeof(ethwan_ifname)))
           )
        {
            /* Fallback case needs to set it default */
            snprintf(ethwan_ifname ,sizeof(ethwan_ifname), "%s", ETHWAN_DEF_INTF_NAME);
        }


#if defined (_BRIDGE_UTILS_BIN_)

        if ( syscfg_set_commit( NULL, "eth_wan_iface_name", ethwan_ifname) != 0 )
        {
            CcspTraceError(( "syscfg_set failed for eth_wan_iface_name\n" ));
        }

        if (ovsEnable)
        {
            v_secure_system("/usr/bin/bridgeUtils del-port brlan0 %s",ethwan_ifname);
        }
        else
#endif
        {
            v_secure_system("brctl delif brlan0 %s",ethwan_ifname);
        }

        macaddr_t macAddr;
        memset(&macAddr,0,sizeof(macaddr_t));
        getInterfaceMacAddress(&macAddr,ethwan_ifname);
        memset(ethwanMac,0,sizeof(ethwanMac));
        snprintf(ethwanMac, sizeof(ethwanMac), "%02x:%02x:%02x:%02x:%02x:%02x", macAddr.hw[0], macAddr.hw[1], macAddr.hw[2],
                macAddr.hw[3], macAddr.hw[4], macAddr.hw[5]);
        if (pEthernet)
        {
            CcspTraceInfo(("Ethwan interface macaddress %s cosaEthwanIfmac length %d\n",ethwanMac,strnlen(pEthernet->EthWanCfg.ethWanIfMac,sizeof(pEthernet->EthWanCfg.ethWanIfMac))));
            if (0 == strnlen(pEthernet->EthWanCfg.ethWanIfMac,sizeof(pEthernet->EthWanCfg.ethWanIfMac)))
            {
                strncpy(pEthernet->EthWanCfg.ethWanIfMac, ethwanMac, sizeof(pEthernet->EthWanCfg.ethWanIfMac)-1);
            }
        }
        memset(&macAddr,0,sizeof(macaddr_t));
        getInterfaceMacAddress(&macAddr,wanPhyName);
        char wan_mac[18];// = {0};
        memset(wan_mac,0,sizeof(wan_mac));
        snprintf(wan_mac, sizeof(wan_mac), "%02x:%02x:%02x:%02x:%02x:%02x", macAddr.hw[0], macAddr.hw[1], macAddr.hw[2],
                macAddr.hw[3], macAddr.hw[4], macAddr.hw[5]);

       v_secure_system("ip link set %s down", ethwan_ifname);
        // EWAN interface needs correct MAC before starting MACsec
        // This could probably be done once since MAC shouldn't change.
        v_secure_system("ip link set %s address %s", ethwan_ifname, wan_mac);
        v_secure_system("ifconfig %s up", ethwan_ifname);

#if defined (_MACSEC_SUPPORT_)
        CcspTraceInfo(("%s - Starting MACsec on %d with %d second timeout\n",__FUNCTION__,ETHWAN_DEF_INTF_NUM,MACSEC_TIMEOUT_SEC));
        if ( RETURN_ERR == platform_hal_StartMACsec(ETHWAN_DEF_INTF_NUM, MACSEC_TIMEOUT_SEC)) {
            CcspTraceError(("%s - MACsec start returning error\n",__FUNCTION__));
        }
#endif

        retvalue = EthwanEnableWithoutReboot(TRUE,bridgemode);
        if (retvalue != ANSC_STATUS_SUCCESS)
        {
            return retvalue;
        }

        /* ETH WAN Interface must be retrieved a second time in case MACsec
           modified the interfaces. */
        memset(ethwan_ifname,0,sizeof(ethwan_ifname));
        if ( (0 != GWP_GetEthWanInterfaceName((unsigned char*)ethwan_ifname, sizeof(ethwan_ifname)))
                || (0 == strnlen(ethwan_ifname,sizeof(ethwan_ifname)))
                || (0 == strncmp(ethwan_ifname,"disable",sizeof(ethwan_ifname)))
           )
        {
            /* Fallback case needs to set it default */
            snprintf(ethwan_ifname , sizeof(ethwan_ifname), "%s", ETHWAN_DEF_INTF_NAME);
        }

#if defined (_BRIDGE_UTILS_BIN_)
        if ( syscfg_set_commit( NULL, "eth_wan_iface_name", ethwan_ifname) != 0 )
        {
            CcspTraceError(( "syscfg_set failed for eth_wan_iface_name\n" ));
        }
#endif
        v_secure_system("ifconfig %s up", ethwan_ifname);

        // Get Macaddress from platform hal and update into sysevent.
        // eth_wan_mac event is used by paradous.
        memset(wan_mac,0,sizeof(wan_mac));
        platform_hal_GetBaseMacAddress(wan_mac);
        v_secure_system("sysevent set eth_wan_mac %s", wan_mac);


        if (bridgemode > 0)
        {
#if defined (ENABLE_WANMODECHANGE_NOREBOOT)
#if defined (_COSA_BCM_ARM_)
            v_secure_system("latticecli -n \"set AltWan.EthWanBridgeModeEnable %s\"", ethwan_ifname);
            v_secure_system("echo e 0 > /proc/driver/ethsw/vlan");
#endif
#endif
        }
    }
    else
    {
        retvalue = EthwanEnableWithoutReboot(FALSE,bridgemode);
        if (retvalue != ANSC_STATUS_SUCCESS)
        {
            return retvalue;
        }
#if defined(INTEL_PUMA7)
       v_secure_system("cmctl up");
#endif

#if defined (_MACSEC_SUPPORT_)
        CcspTraceInfo(("%s - Stopping MACsec on %d\n",__FUNCTION__,ETHWAN_DEF_INTF_NUM));
        /* Stopping MACsec on Port since DOCSIS Succeeded */
        if ( RETURN_ERR == platform_hal_StopMACsec(ETHWAN_DEF_INTF_NUM)) {
            CcspTraceError(("%s - MACsec stop error\n",__FUNCTION__));
        }
#endif

        if ( (0 != GWP_GetEthWanInterfaceName((unsigned char*)ethwan_ifname, sizeof(ethwan_ifname)))
                || (0 == strnlen(ethwan_ifname,sizeof(ethwan_ifname)))
                || (0 == strncmp(ethwan_ifname,"disable",sizeof(ethwan_ifname)))
           )
        {
            /* Fallback case needs to set it default */
            snprintf(ethwan_ifname ,sizeof(ethwan_ifname), "%s", ETHWAN_DEF_INTF_NAME);
        }

        v_secure_system("ifconfig %s down",ethwan_ifname);
        v_secure_system("ip addr flush dev %s",ethwan_ifname);
        v_secure_system("ip -6 addr flush dev %s",ethwan_ifname);
        v_secure_system("sysctl -w net.ipv6.conf.%s.accept_ra=0",ethwan_ifname);
        if (pEthernet)
        {
            CcspTraceInfo(("cosaethwanMac %s cosaEthwanIfmac length %d\n",pEthernet->EthWanCfg.ethWanIfMac,strnlen(pEthernet->EthWanCfg.ethWanIfMac,sizeof(pEthernet->EthWanCfg.ethWanIfMac))));
            v_secure_system("ip link set %s address %s", ethwan_ifname, pEthernet->EthWanCfg.ethWanIfMac);
        }
        v_secure_system("ifconfig %s up",ethwan_ifname);
        if (bridgemode > 0)
        {
#if defined (ENABLE_WANMODECHANGE_NOREBOOT)
#if defined (_COSA_BCM_ARM_)
            v_secure_system("latticecli -n \"set AltWan.EthWanBridgeModeEnable disable\"");
#endif
#endif
        }

    }
    // update latest status to ethwan cfg cosa structure.
    if (pEthernet)
    {
        CosaDmlEthWanGetCfg(&ethWanCfg);
        pEthernet->EthWanCfg.Enable = ethWanCfg.Enable;
        pEthernet->EthWanCfg.Port = ethWanCfg.Port;
    }

    CcspTraceError(("Func %s Exited arg %d\n",__FUNCTION__,bEnable));
    return ANSC_STATUS_SUCCESS;
}
#endif

ANSC_STATUS
CosaDmlEthWanSetEnable
    (
        BOOL                       bEnable
    )
{

#if defined (ENABLE_WANMODECHANGE_NOREBOOT)
    ConfigEthWan(bEnable);
    return ANSC_STATUS_SUCCESS;
#elif defined (ENABLE_ETH_WAN)
    BOOL bGetStatus = FALSE;
    CcspHalExtSw_getEthWanEnable(&bGetStatus);
    char command[50] = {0};
    errno_t rc = -1;
    if (bEnable != bGetStatus)
    {
        if(bEnable == FALSE)
        {
            v_secure_system("ifconfig erouter0 down");
            // NOTE: Eventually ETHWAN_DEF_INTF_NAME should be replaced with GWP_GetEthWanInterfaceName()
            v_secure_system("ip link set erouter0 name " ETHWAN_DEF_INTF_NAME);
            CcspTraceWarning(("****************value of command = ip link set erouter0 name %s**********************\n", ETHWAN_DEF_INTF_NAME));
            v_secure_system("ip link set dummy-rf name erouter0");
            rc =  memset_s(command,sizeof(command),0,sizeof(command));
            ERR_CHK(rc);
            /*Coverity Fix  CID: 132495 DC.STRING_BUFFER*/
            v_secure_system("ifconfig " ETHWAN_DEF_INTF_NAME" up;ifconfig erouter0 up");
            CcspTraceWarning(("****************value of command = ifconfig %s**********************\n", ETHWAN_DEF_INTF_NAME));
        } 
    }

    CcspHalExtSw_setEthWanPort ( ETHWAN_DEF_INTF_NUM );

    if ( ANSC_STATUS_SUCCESS == CcspHalExtSw_setEthWanEnable( bEnable ) ) 
    {
        pthread_t tid;		

        if(bEnable)
        {
            v_secure_system("touch /nvram/ETHWAN_ENABLE");
        }
        else
        {
            v_secure_system("rm /nvram/ETHWAN_ENABLE");
        }

        if ( syscfg_set_commit( NULL, "eth_wan_enabled", bEnable ? "true" : "false" ) != 0 )
        {
            AnscTraceWarning(( "syscfg_set failed for eth_wan_enabled\n" ));
            return ANSC_STATUS_FAILURE;
        }
        else
        {
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

#if defined (FEATURE_RDKB_WAN_MANAGER)
int
EthWanSetLED
    (
        int color,
        int state,
        int interval
    )
{
    LEDMGMT_PARAMS ledMgmt;
    memset(&ledMgmt, 0, sizeof(LEDMGMT_PARAMS));

        ledMgmt.LedColor = color;
        ledMgmt.State    = state;
        ledMgmt.Interval = interval;
#if defined(_XB6_PRODUCT_REQ_)
        if(RETURN_ERR == platform_hal_setLed(&ledMgmt)) {
                CcspTraceError(("platform_hal_setLed failed\n"));
                return 1;
        }
#endif
    return 0;
}

void getInterfaceMacAddress(macaddr_t* macAddr,char *pIfname)
{
    FILE *f = NULL;
    char line[256], *lineptr = line, fname[128];
    size_t size;

    if (pIfname)
    {
        snprintf(fname,sizeof(fname), "/sys/class/net/%s/address", pIfname);
        size = sizeof(line);
        if ((f = fopen(fname, "r")))
        {
            if ((getline(&lineptr, &size, f) >= 0))
            {
                sscanf(lineptr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &macAddr->hw[0], &macAddr->hw[1], &macAddr->hw[2], &macAddr->hw[3], &macAddr->hw[4], &macAddr->hw[5]);
            }
            fclose(f);
        }
    }

    return;
}

ANSC_STATUS EthWanBridgeInit(PCOSA_DATAMODEL_ETHERNET pEthernet)
{
    char wanPhyName[64] = {0};
    char out_value[64] = {0};
    char ethwan_ifname[64] = {0};
    char ethwanMac[32] = {0};
    macaddr_t macAddr;
    char wan_mac[18];// = {0};
    
#if defined (_BRIDGE_UTILS_BIN_)
    int ovsEnable = 0;
    char buf[ 8 ] = { 0 };
    if( 0 == syscfg_get( NULL, "mesh_ovs_enable", buf, sizeof( buf ) ) )
    {
          if ( strcmp (buf,"true") == 0 )
            ovsEnable = 1;
          else
            ovsEnable = 0;

	  CcspTraceWarning(("ovsEnable=%d\n",ovsEnable));
    }
    else
    {
          CcspTraceError(("syscfg_get failed to retrieve ovs_enable\n"));

    }
#endif

    CcspTraceError(("Func %s Entered\n",__FUNCTION__));

    if (!syscfg_get(NULL, "wan_physical_ifname", out_value, sizeof(out_value)))
    {
       strncpy(wanPhyName, out_value, sizeof(wanPhyName) - 1);
       printf("wanPhyName = %s\n", wanPhyName);
    }
    else
    {
        strncpy(wanPhyName, "erouter0", sizeof(wanPhyName) - 1);

    }

	//Get the ethwan interface name from HAL
	memset( ethwan_ifname , 0, sizeof( ethwan_ifname ) );
	if ((0 != GWP_GetEthWanInterfaceName((unsigned char*) ethwan_ifname, sizeof(ethwan_ifname))) 
             || (0 == strnlen(ethwan_ifname,sizeof(ethwan_ifname)))
            || (0 == strncmp(ethwan_ifname,"disable",sizeof(ethwan_ifname))))
	{
		//Fallback case needs to set it default
		memset( ethwan_ifname , 0, sizeof( ethwan_ifname ) );
		snprintf( ethwan_ifname , sizeof(ethwan_ifname) ,"%s", ETHWAN_DEF_INTF_NAME );
    }

    CcspTraceInfo(("Ethwan interface %s \n",ethwan_ifname));
    memset(&macAddr,0,sizeof(macaddr_t));
    getInterfaceMacAddress(&macAddr,ethwan_ifname);

    memset(ethwanMac,0,sizeof(ethwanMac));
    snprintf(ethwanMac, sizeof(ethwanMac), "%02x:%02x:%02x:%02x:%02x:%02x", macAddr.hw[0], macAddr.hw[1], macAddr.hw[2],
            macAddr.hw[3], macAddr.hw[4], macAddr.hw[5]);
    if (pEthernet)
    {
        CcspTraceInfo(("Ethwan interface macaddress %s cosaEthwanIfmac length %d\n",ethwanMac,strnlen(pEthernet->EthWanCfg.ethWanIfMac,sizeof(pEthernet->EthWanCfg.ethWanIfMac))));
        if (0 == strnlen(pEthernet->EthWanCfg.ethWanIfMac,sizeof(pEthernet->EthWanCfg.ethWanIfMac)))
        {
            strncpy(pEthernet->EthWanCfg.ethWanIfMac, ethwanMac, sizeof(pEthernet->EthWanCfg.ethWanIfMac)-1);
        }
    }

    memset(&macAddr,0,sizeof(macaddr_t));
    getInterfaceMacAddress(&macAddr,wanPhyName);
    memset(wan_mac,0,sizeof(wan_mac));
    sprintf(wan_mac, "%02x:%02x:%02x:%02x:%02x:%02x",macAddr.hw[0],macAddr.hw[1],macAddr.hw[2],macAddr.hw[3],macAddr.hw[4],macAddr.hw[5]);

    v_secure_system("ifconfig %s down", ethwan_ifname);

#if !defined(INTEL_PUMA7)
#if defined (_BRIDGE_UTILS_BIN_)

        if (ovsEnable)
        {
            v_secure_system("/usr/bin/bridgeUtils del-port brlan0 %s",ethwan_ifname);
        } 
        else
#endif
        {
            v_secure_system("vlan_util del_interface brlan0 %s", ethwan_ifname);
        }
#endif

#ifdef _COSA_BCM_ARM_
    v_secure_system("ifconfig %s down; ip link set %s name %s", wanPhyName,wanPhyName,ETHWAN_DOCSIS_INF_NAME);
#else
    v_secure_system("ifconfig %s down; ip link set %s name dummy-rf", wanPhyName,wanPhyName);
#endif
    v_secure_system("brctl addbr %s; brctl addif %s %s", wanPhyName,wanPhyName,ethwan_ifname);

#if defined(INTEL_PUMA7)
    v_secure_system("brctl addif %s %s",wanPhyName,ETHWAN_DOCSIS_INF_NAME);
    BridgeNfDisable(wanPhyName, NF_ARPTABLE, TRUE);
    BridgeNfDisable(wanPhyName, NF_IPTABLE, TRUE);
    BridgeNfDisable(wanPhyName, NF_IP6TABLE, TRUE);
#endif

    v_secure_system("sysctl -w net.ipv6.conf.%s.autoconf=0", ethwan_ifname); // Fix: RDKB-22835, disabling IPv6 for ethwan port
    v_secure_system("sysctl -w net.ipv6.conf.%s.disable_ipv6=1", ethwan_ifname); // Fix: RDKB-22835, disabling IPv6 for ethwan port
    v_secure_system("ifconfig %s hw ether %s", ethwan_ifname,wan_mac);
    v_secure_system("ip6tables -I OUTPUT -o %s -p icmpv6 -j DROP", ethwan_ifname);
#ifdef _COSA_BCM_ARM_
    v_secure_system("ip link set %s up",ETHWAN_DOCSIS_INF_NAME);
    v_secure_system("brctl addbr %s; brctl addif %s %s", wanPhyName,wanPhyName,ETHWAN_DOCSIS_INF_NAME);
    v_secure_system("sysctl -w net.ipv6.conf.%s.disable_ipv6=1",ETHWAN_DOCSIS_INF_NAME);
#endif

    v_secure_system("ifconfig %s down", wanPhyName);
    v_secure_system("ifconfig %s hw ether %s", wanPhyName,wan_mac);
    platform_hal_GetBaseMacAddress(wan_mac);
    v_secure_system("sysevent set eth_wan_mac %s", wan_mac);
    v_secure_system("ifconfig %s up", wanPhyName);

#ifdef INTEL_PUMA7
    v_secure_system("ifconfig %s up",ethwan_ifname);
#endif
    CcspTraceError(("Func %s Exited\n",__FUNCTION__));
    return ANSC_STATUS_SUCCESS;
}
#endif

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
    //ETH Port Init.
    CosaDmlEthPortInit((PANSC_HANDLE)pMyObject);

#ifdef FEATURE_RDKB_WAN_MANAGER
#ifndef AUTOWAN_ENABLE
    char wanoe_ifname[WANOE_IFACENAME_LENGTH] = {0};

    /* To initialize PhyStatus for Manager we set Down */
    if (ANSC_STATUS_SUCCESS == GetWan_InterfaceName (wanoe_ifname, sizeof(wanoe_ifname))) 
    {
        ANSC_STATUS ret = ANSC_STATUS_FAILURE;
        ret = CosaDmlEthSetPhyPathForWanManager(wanoe_ifname);
        CcspTraceError(("%s:%d Initialize %s PhyPath for Manager[%s]\n", __FUNCTION__, __LINE__, wanoe_ifname, (ret == ANSC_STATUS_SUCCESS)?"Success":"Failed"));
    }
    else
    {
        CcspTraceError(("%s:%d  GetWan_InterfaceName Failed\n", __FUNCTION__, __LINE__));
    }
#endif //AUTOWAN_ENABLE
#endif

    #ifdef WAN_FAILOVER_SUPPORTED
      TriggerSysEventMonitorThread();
    #endif
    /*!! Moved Message queue Event handler creation before Eth hal initailization to avoid any missing of Ethwan callback notification.
    *  This helps to cache ethwan link up/down events in message queue even if wan manager or any consumer not ready by the time.
    */

    /* Trigger Event Handler thread.
     * Monitor for the link status event and notify the other agents,
     */
    CosaDmlEthTriggerEventHandlerThread();

#ifdef AUTOWAN_ENABLE 
    {
        PCOSA_DATAMODEL_ETH_WAN_AGENT pEthWanCfg = NULL;

        if (pMyObject)
        {
            pEthWanCfg = (PCOSA_DATAMODEL_ETH_WAN_AGENT)&pMyObject->EthWanCfg;
            if (pEthWanCfg)
            {
                pEthWanCfg->MonitorPhyStatusAndNotify = FALSE;
            }
        }
        CcspTraceInfo(("pthread create boot inform \n"));
        pthread_create(&bootInformThreadId, NULL, &ThreadBootInformMsg, NULL);

        if (TRUE == isEthWanEnabled())
        {
            EthWanBridgeInit(pMyObject);
        }

        //Initialise ethsw-hal to get event notification from lower layer.
        if (CcspHalEthSwInit() != RETURN_OK)
        {
            CcspTraceError(("Hal initialization failed \n"));
            return ANSC_STATUS_FAILURE;
        }
#if defined(INTEL_PUMA7)
        if (TRUE == isEthWanEnabled())
        {
            CcspHalExtSw_setEthWanPort ( ETHWAN_DEF_INTF_NUM );
            if (ANSC_STATUS_SUCCESS != CcspHalExtSw_setEthWanEnable( TRUE))
            {
                CcspTraceError(("CcspHalExtSw_setEthWanEnable failed in bootup \n"));
            }
        }
#endif 
    }
#else
    #if defined(_PLATFORM_RASPBERRYPI_) || defined(_PLATFORM_TURRIS_)
    char wanPhyName[20] = {0},out_value[20] = {0};

    if (!syscfg_get(NULL, "wan_physical_ifname", out_value, sizeof(out_value)))
    {
       strcpy(wanPhyName, out_value);
    }
    else
    {
       return -1;
    }
    v_secure_system("ifconfig " ETHWAN_DEF_INTF_NAME" down");
    v_secure_system("ip link set "ETHWAN_DEF_INTF_NAME" name %s",wanPhyName);
    v_secure_system("ifconfig %s up",wanPhyName);
    #endif

    //Initialise ethsw-hal to get event notification from lower layer.
    if (CcspHalEthSwInit() != RETURN_OK)
    {
        CcspTraceError(("Hal initialization failed \n"));
        return ANSC_STATUS_FAILURE;
    }
#endif
#if defined (FEATURE_RDKB_WAN_AGENT)

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
    if(CosaDmlGetWanOEInterfaceName(WanOEInterface, sizeof(WanOEInterface)) == ANSC_STATUS_SUCCESS) {
#if defined (INTEL_PUMA7) && !defined (AUTOWAN_ENABLE)
        if(FALSE) 
#else
        if(GWP_GetEthWanLinkStatus() == 1) 
#endif
        {
            if(CosaDmlEthGetPhyStatusForWanManager(WanOEInterface, PhyStatus) == ANSC_STATUS_SUCCESS) {
                if(strcmp(PhyStatus, "Up") != 0) {
                    CosaDmlEthSetPhyStatusForWanManager(WanOEInterface, WANOE_IFACE_UP);
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

#else
    UNREFERENCED_PARAMETER(phContext);
#endif //#if defined (FEATURE_RDKB_WAN_AGENT) || FEATURE_RDKB_WAN_MANAGER
    return ANSC_STATUS_SUCCESS;
}

#if defined (FEATURE_RDKB_WAN_MANAGER)
ANSC_STATUS GetEthPhyInterfaceName(INT index, CHAR *ifname, INT ifnameLength)
{
    FILE *fp = NULL;
    CHAR data[256];
    CHAR logicalPort[64];
    CHAR logicalName[64];
    CHAR typeInterface[64];
    CHAR *pTmpData = NULL;
    INT parsedIndex = -1;
    BOOL ifnameFound = FALSE;


    fp = fopen(INTERFACE_MAPPING_TABLE_FNAME, "r");
    if (fp == NULL)
        return ANSC_STATUS_FAILURE;

    memset(data,0,sizeof(data));
    memset(logicalPort,0,sizeof(logicalPort));
    memset(logicalName,0,sizeof(logicalName));
    memset(typeInterface,0,sizeof(typeInterface));
    while (fgets(data,sizeof(data), fp) != NULL)
    {
        sscanf(data,"%s %s %s",logicalPort,logicalName,typeInterface);
        pTmpData=strstr(logicalPort,"=");
        if (pTmpData != NULL)
        {
            ++pTmpData;
            parsedIndex = atoi(pTmpData);
            if (index == parsedIndex)
            {
                if (strstr(typeInterface,"PHY"))
                {
                    pTmpData = strstr(data,"BaseInterface=");
                    if (pTmpData)
                    {
                        pTmpData += strlen("BaseInterface=");
                        strncpy(ifname,pTmpData,ifnameLength);
                        ifname[strlen(ifname) - 1] = '\0';
                        ifnameFound = TRUE;
                        break;
                    }
                }
                else
                {
                     strncpy(ifname,logicalName + strlen("LogicalName="),ifnameLength);
                     ifnameFound = TRUE;
                     break;
                }
            }
        }
    }
    fclose(fp);
    if (TRUE == ifnameFound)
    {
        if (TRUE == isEthWanEnabled())
        {
            errno_t rc = -1;
            int ind = -1;

            rc = strcmp_s(ETHWAN_DEF_INTF_NAME,strlen(ETHWAN_DEF_INTF_NAME),ifname,&ind);
            ERR_CHK(rc);
            if (ind == 0)
            {
                char wanoe_ifname[WANOE_IFACENAME_LENGTH] = {0};

                if (ANSC_STATUS_SUCCESS == GetWan_InterfaceName (wanoe_ifname, sizeof(wanoe_ifname)))
                {
                    memset(ifname,0,ifnameLength);
                    strncpy(ifname,wanoe_ifname,ifnameLength);
                }
            }
        }
        return ANSC_STATUS_SUCCESS;
    }
    return ANSC_STATUS_FAILURE;
}
#endif

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
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
    char infPortCapability[BUFLEN_32] = {0};
    char acGetParamName[BUFLEN_256] = {0};
#endif  //FEATURE_RDKB_AUTO_PORT_SWITCH
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
        //Fill line static information and initialize default values
        DML_ETHIF_INIT(pETHTemp);
        pETHTemp->ulInstanceNumber = iLoopCount + 1;
        pMyObject->ulPtNextInstanceNumber=  pETHTemp->ulInstanceNumber + 1;
        // Get  Name.
#if defined (INTEL_PUMA7)
        if ( ANSC_STATUS_SUCCESS != GetEthPhyInterfaceName(iLoopCount + 1,pETHTemp->Name,sizeof(pETHTemp->Name)))
        {
            snprintf(pETHTemp->Name, sizeof(pETHTemp->Name), "sw_%d", iLoopCount + 1);
        }
        CcspTraceWarning(("\n ifname copied %s index %d\n",pETHTemp->Name,iLoopCount));
#else
        snprintf(pETHTemp->Name, sizeof(pETHTemp->Name), "eth%d", iLoopCount);
#endif
        snprintf(pETHTemp->LowerLayers, sizeof(pETHTemp->LowerLayers), "%s%d", ETHERNET_IF_LOWERLAYERS, iLoopCount + 1);
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH

    snprintf(acGetParamName, sizeof(acGetParamName), PSM_ETHAGENT_IF_PORTCAPABILITY, iLoopCount + 1);

    CcspTraceInfo(("%s query[%s]\n",__FUNCTION__,acGetParamName));
    Ethagent_GetParamValuesFromPSM(acGetParamName,infPortCapability,sizeof(infPortCapability));

    CcspTraceInfo(("%s infPortCapability[%s]\n",__FUNCTION__,infPortCapability));

    if(!strcmp(infPortCapability,"WAN_LAN"))
    {
        pETHTemp->PortCapability = PORT_CAP_WAN_LAN;
    }
    else if(!strcmp(infPortCapability,"WAN"))
    {
        pETHTemp->PortCapability = PORT_CAP_WAN;
    }
    else
    {
        pETHTemp->PortCapability = PORT_CAP_LAN;
    }

#endif  //FEATURE_RDKB_AUTO_PORT_SWITCH
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
            strncpy(tmpEthGInfo.Name, gpstEthGInfo[lastIndex].Name, sizeof(gpstEthGInfo[lastIndex].Name) - 1);
            strncpy(tmpEthGInfo.Path, gpstEthGInfo[lastIndex].Path, sizeof(gpstEthGInfo[lastIndex].Path) - 1);
            strncpy(tmpEthGInfo.LowerLayers, gpstEthGInfo[lastIndex].LowerLayers, sizeof(gpstEthGInfo[lastIndex].LowerLayers) - 1);
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
            strncpy(gpstEthGInfo[IfIndex].Name, tmpEthGInfo.Name, sizeof(tmpEthGInfo.Name) - 1);
            strncpy(gpstEthGInfo[IfIndex].Path, tmpEthGInfo.Path, sizeof(tmpEthGInfo.Path) - 1);
            strncpy(gpstEthGInfo[IfIndex].LowerLayers, tmpEthGInfo.LowerLayers, sizeof(tmpEthGInfo.LowerLayers) - 1);
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
        CosaDmlEthPortLinkStatusCallback(ifname,WANOE_IFACE_UP);
        CcspTraceInfo(("Successfully updated PhyStatus to Up for [%s] interface \n",ifname));
    }
    else
    {
        CosaDmlEthPortLinkStatusCallback(ifname,WANOE_IFACE_DOWN);
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
    strncpy(gpstEthGInfo[IfIndex].LowerLayers,newLowerLayers, sizeof(gpstEthGInfo[IfIndex].LowerLayers) - 1);
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
    pthread_mutex_lock(&gmEthGInfo_mutex);
    if (IfIndex >= CosaDmlEthGetTotalNoOfInterfaces())
    {
        pthread_mutex_unlock(&gmEthGInfo_mutex);
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }
    strncpy(gpstEthGInfo[IfIndex].Name,newIfname, sizeof(gpstEthGInfo[IfIndex].Name) - 1);
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

#if defined(FEATURE_RDKB_WAN_MANAGER) || defined(FEATURE_RDKB_WAN_AGENT)

INT CosaDmlEthPortLinkStatusCallback(CHAR *ifname, CHAR *state)
{
    if (NULL == ifname || state == NULL)
    {
        CcspTraceError(("Invalid memory \n"));
        return ANSC_STATUS_FAILURE;
    }

    COSA_DML_ETH_LINK_STATUS link_status;

    if (strncasecmp(state, WANOE_IFACE_UP, 2) == 0)
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
#endif

static ANSC_STATUS CosaDmlEthPortGetIndexFromIfName(char *ifname, INT *IfIndex)
{
    INT iTotalInterfaces;
    INT iLoopCount;

    if (NULL == ifname || IfIndex == NULL || gpstEthGInfo == NULL)
    {
        CcspTraceError(("Invalid Memory \n"));
        return ANSC_STATUS_FAILURE;
    }

    *IfIndex = -1;
    pthread_mutex_lock(&gmEthGInfo_mutex);

    iTotalInterfaces = CosaDmlEthGetTotalNoOfInterfaces();

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

    //Assign default value
    for (iLoopCount = 0; iLoopCount < Totalinterfaces; ++iLoopCount)
    {
        gpstEthGInfo[iLoopCount].Upstream = FALSE;
        gpstEthGInfo[iLoopCount].WanStatus = ETH_WAN_DOWN;
        gpstEthGInfo[iLoopCount].LinkStatus = ETH_LINK_STATUS_DOWN;
        gpstEthGInfo[iLoopCount].WanValidated = TRUE; //Make default as True.
#if defined (INTEL_PUMA7)
        if ( ANSC_STATUS_SUCCESS != GetEthPhyInterfaceName(iLoopCount + 1,gpstEthGInfo[iLoopCount].Name,sizeof(gpstEthGInfo[iLoopCount].Name)))
        {
            snprintf(gpstEthGInfo[iLoopCount].Name, sizeof(gpstEthGInfo[iLoopCount].Name), "sw_%d", iLoopCount+1);
        }
        CcspTraceWarning(("\n ifname copied %s index %d\n",gpstEthGInfo[iLoopCount].Name,iLoopCount));
#else
        snprintf(gpstEthGInfo[iLoopCount].Name, sizeof(gpstEthGInfo[iLoopCount].Name), "eth%d", iLoopCount);
#endif
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
                // Update latest Hal Ethwan Interface name to wan manager interface table
                // whatever receives in ethwan link up/down callback.  This is needed always when macsec is enabled.
#ifdef FEATURE_RDKB_WAN_MANAGER
#if defined(INTEL_PUMA7)
                char acSetParamName[256];
                INT iWANInstance = -1;
                // wait till wan manager is available on bus.
                waitForWanMgrComponentReady();
                CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_WAN_AGENT,  MSGQWanData.Name, &iWANInstance);
                if (-1 == iWANInstance)
                {
                    memset(acSetParamName, 0, sizeof(acSetParamName));
                    snprintf(acSetParamName, sizeof(acSetParamName), WAN_IF_NAME_PARAM_NAME, WAN_ETH_INTERFACE_INSTANCE_NUM);
                    if (CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, MSGQWanData.Name, ccsp_string, TRUE) != ANSC_STATUS_SUCCESS)
                    {
                        CcspTraceError(("%s %d: Unable to set param name %s\n", __FUNCTION__, __LINE__, acSetParamName));
                    }
                }
#endif
#endif

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
    parameterValStruct_t **retVal = NULL;
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
        if(acSetParamName)
            free(acSetParamName);
        if(acSetParamValue)
            free(acSetParamValue);
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
    CcspTraceInfo(("%s %d Successfully notified %s event to WANMANAGER for %s interface\n", __FUNCTION__, __LINE__, WanStatus, ifname));
    return ANSC_STATUS_SUCCESS;
}

/* Set wan interface name to WanManager */
ANSC_STATUS CosaDmlEthSetWanInterfaceNameForWanManager(char *ifname, char *WanIfName)
{
    COSA_DML_ETH_PORT_GLOBAL_CONFIG stGlobalInfo = {0};
    char acSetParamName[256];
    char acSetParamValue[256];
    INT iWANInstance = -1;
    //Validate buffer
    if ((NULL == ifname) || (NULL == WanIfName))
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
    snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, WAN_INTERFACE_PARAM_NAME, iWANInstance);
    snprintf(acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", WanIfName);
    CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, TRUE);

    CcspTraceInfo(("%s %d Successfully notified WAN Interface name %s for %s base interface\n", __FUNCTION__, __LINE__, WanIfName, ifname));
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
            char acTmpQueryParam[300] = {0};

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
#if defined (FEATURE_RDKB_WAN_MANAGER)
    BOOL isPppoeIface = FALSE;
#endif //FEATURE_RDKB_WAN_MANAGER
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
#if defined (FEATURE_RDKB_WAN_MANAGER)
    if (ANSC_STATUS_SUCCESS != DmlEthCheckIfaceConfiguredAsPPPoE(l2ifName, &isPppoeIface))
    {
        CcspTraceError(("%s - DmlEthCheckIfaceConfiguredAsPPPoE() failed \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
#endif //FEATURE_RDKB_WAN_MANAGER
    //Create VLAN Link.
    //Index is not present. so needs to create a PTM instance
    if (-1 == iVLANInstance)
    {
        char acTableName[128] = {0};
        INT iNewTableInstance = -1;

        snprintf(acTableName, sizeof(acTableName), "%s", VLAN_ETH_LINK_TABLE_NAME);
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
#if defined (FEATURE_RDKB_WAN_MANAGER)
    if (isPppoeIface)
        CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, PPPoE_VLAN_INTERFACE_NAME, ccsp_string, FALSE);
    else
        CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, WAN_INTERFACE_NAME, ccsp_string, FALSE);
#else //FEATURE_RDKB_WAN_AGENT
    CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, l2ifName, ccsp_string,FALSE);
#endif //FEATURE_RDKB_WAN_MANAGER
#if defined (FEATURE_RDKB_WAN_MANAGER)
    //Set Base interface
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), VLAN_ETH_LINK_PARAM_BASEINTERFACE, iVLANInstance);
    CosaDmlEthSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, l2ifName, ccsp_string,FALSE);
#endif //FEATURE_RDKB_WAN_MANAGER
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
    snprintf(acTableName, sizeof(acTableName), "%s%d.", VLAN_ETH_LINK_TABLE_NAME, iVLANInstance);
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
#ifdef FEATURE_RDKB_WAN_MANAGER
ANSC_STATUS CosaDmlEthSetPhyPathForWanManager(char *ifname)
{
    COSA_DML_ETH_PORT_GLOBAL_CONFIG stGlobalInfo = {0};
    char acSetParamName[256] ={0};
    char acSetParamVal[256] = {0};
    INT iLinkInstance = -1;
    INT iWANInstance = -1;

    //Validate Input
    if (NULL == ifname)
    {
        CcspTraceError(("%s ifname[NULL]\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceError(("%s:%d ifname[%s]\n", __FUNCTION__,__LINE__,ifname));

    //Get global copy of the data from interface name
    CosaDmlEthPortGetCopyOfGlobalInfoForGivenIfName(ifname, &stGlobalInfo);
    CcspTraceError(("%s:%d stGlobalInfo.Name[%s]\n", __FUNCTION__,__LINE__,stGlobalInfo.Name));

    //Get Instance for corresponding name
    CosaDmlEthGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_WAN_AGENT, stGlobalInfo.Name, &iWANInstance);

    //Index is not present. so no need to do anything any WAN instance
    if (-1 == iWANInstance)
    {
        CcspTraceError(("%s %d WAN instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    snprintf(acSetParamName, sizeof(acSetParamName), WAN_PHY_PATH_PARAM_NAME, iWANInstance);
    if (CosaDmlEthPortGetIndexFromIfName(ifname, &iLinkInstance) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Unable to get linkinstance from iface name %s\n", __FUNCTION__, __LINE__, ifname));
        return ANSC_STATUS_FAILURE;
    }

    //Set PHY path
    snprintf(acSetParamVal, sizeof(acSetParamVal),ETH_IF_PHY_PATH, (iLinkInstance + 1));
    if (CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamVal, ccsp_string, TRUE) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Unable to set param name %s with value %s\n", __FUNCTION__, __LINE__, acSetParamName, acSetParamVal));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d Successfully update phy.Path to WAN Manager for %s interface\n", __FUNCTION__, __LINE__,  ifname));

    return ANSC_STATUS_SUCCESS;
}
#endif // FEATURE_RDKB_WAN_MANAGER

#ifdef FEATURE_RDKB_WAN_AGENT
ANSC_STATUS CosaDmlEthSetPhyStatusForWanAgent(char *ifname, char *PhyStatus)
#else  //FEATURE_RDKB_WAN_MANAGER
ANSC_STATUS CosaDmlEthSetPhyStatusForWanManager(char *ifname, char *PhyStatus)
#endif
{
    COSA_DML_ETH_PORT_GLOBAL_CONFIG stGlobalInfo = {0};
    char acSetParamName[256];
#ifndef AUTOWAN_ENABLE
    char acSetParamVal[256];
    INT iLinkInstance = -1;
#endif
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

// Phy path not needed to set in case of auto wan policy mode.
// it will be set in part of boot inform msg.
#ifndef AUTOWAN_ENABLE
    //Set PHY path
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_PHY_PATH_PARAM_NAME, iWANInstance);

    memset(acSetParamVal, 0, sizeof(acSetParamVal));
    if (CosaDmlEthPortGetIndexFromIfName(ifname, &iLinkInstance) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Unable to get linkinstance from iface name %s\n", __FUNCTION__, __LINE__, ifname));
        return ANSC_STATUS_FAILURE;
    }

    snprintf(acSetParamVal, sizeof(acSetParamVal),ETH_IF_PHY_PATH, (iLinkInstance + 1));
    if (CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamVal, ccsp_string, TRUE) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Unable to set param name %s with value %s\n", __FUNCTION__, __LINE__, acSetParamName, acSetParamVal));
        return ANSC_STATUS_FAILURE;
    }

#endif
    //Set PHY Status
    memset(acSetParamName, 0, sizeof(acSetParamName));
    snprintf(acSetParamName, sizeof(acSetParamName), WAN_PHY_STATUS_PARAM_NAME, iWANInstance);
    if (CosaDmlEthSetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, PhyStatus, ccsp_string, TRUE) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Unable to set param name %s\n", __FUNCTION__, __LINE__, acSetParamName));
        return ANSC_STATUS_FAILURE;
    }

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
    if (CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acGetParamName, PhyStatus) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: CosaDmlEthGetParamValues() returned FAILURE\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}
#if defined (FEATURE_RDKB_WAN_MANAGER)
/**
 * @note API to check the given interface is configured to use as a PPPoE interface
*/
static ANSC_STATUS DmlEthCheckIfaceConfiguredAsPPPoE( char *ifname, BOOL *isPppoeIface)
{
    char acTmpReturnValue[DATAMODEL_PARAM_LENGTH] = {0};
    char acTmpQueryParam[DATAMODEL_PARAM_LENGTH] = {0};
    INT iLoopCount, iTotalNoofEntries;
    char *endPtr = NULL;

    *isPppoeIface = FALSE;

    if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, WAN_NOE_PARAM_NAME, acTmpReturnValue))
    {
        CcspTraceError(("[%s][%d]Failed to get param value\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }
    //Total interface count
    iTotalNoofEntries = strtol(acTmpReturnValue, &endPtr, 10);
    if(*endPtr)
    {
        CcspTraceError(("Unable to convert '%s' to base 10", acTmpReturnValue ));
        return ANSC_STATUS_FAILURE;
    }

    if (0 >= iTotalNoofEntries)
    {
        return ANSC_STATUS_SUCCESS;
    }

    //Traverse table
    for (iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++)
    {
        //Query for WAN interface name
        snprintf(acTmpQueryParam, sizeof(acTmpQueryParam), WAN_IF_NAME_PARAM_NAME, iLoopCount + 1);
        memset(acTmpReturnValue, 0, sizeof(acTmpReturnValue));
        if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH, acTmpQueryParam, acTmpReturnValue))
        {
            CcspTraceError(("[%s][%d] Failed to get param value\n", __FUNCTION__, __LINE__));
            continue;
        }

        //Compare WAN interface name
        if (0 == strcmp(acTmpReturnValue, ifname))
        {
            //Query for PPP Enable data model
            snprintf(acTmpQueryParam, sizeof(acTmpQueryParam), WAN_IF_PPP_ENABLE_PARAM, iLoopCount + 1);
            memset(acTmpReturnValue, 0, sizeof(acTmpReturnValue));
            if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH,
                                                            acTmpQueryParam, acTmpReturnValue))
            {
                CcspTraceError(("[%s][%d] Failed to get param value\n", __FUNCTION__, __LINE__));
            }
            if (0 == strcmp(acTmpReturnValue, "true"))
            {
                //Query for PPP LinkType data model
                snprintf(acTmpQueryParam, sizeof(acTmpQueryParam), WAN_IF_PPP_LINKTYPE_PARAM, iLoopCount + 1);
                memset(acTmpReturnValue, 0, sizeof(acTmpReturnValue));
                if (ANSC_STATUS_FAILURE == CosaDmlEthGetParamValues(WAN_COMPONENT_NAME, WAN_DBUS_PATH,
                                                                acTmpQueryParam, acTmpReturnValue))
                {
                    CcspTraceError(("[%s][%d] Failed to get param value\n", __FUNCTION__, __LINE__));
                }
                if (0 == strcmp(acTmpReturnValue, "PPPoE"))
                {
                    *isPppoeIface = TRUE;
                }
            }
            break;
        }
    }
    return ANSC_STATUS_SUCCESS;
}

/**
 * @Note Utility API to get WANOE interface from HAL layer.
 */
static ANSC_STATUS  GetWan_InterfaceName (char* wanoe_ifacename, int length) {
    if (NULL == wanoe_ifacename || length == 0) {
        CcspTraceError(("[%s][%d] Invalid argument  \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    char wanoe_ifname[WANOE_IFACENAME_LENGTH] = {0};
    if (RETURN_OK != GWP_GetEthWanInterfaceName((unsigned char*)wanoe_ifname, sizeof(wanoe_ifname))) {
        CcspTraceError(("[%s][%d] Failed to get wanoe interface name \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    strncpy (wanoe_ifacename,wanoe_ifname,length);
    return ANSC_STATUS_SUCCESS;
}

/**
 * @Note Callback invoked upon wanoe interface link up from HAL.
 */
void EthWanLinkUp_callback() {
    char wanoe_ifname[WANOE_IFACENAME_LENGTH] = {0};
#ifdef AUTOWAN_ENABLE
        char redirFlag[10]={0};
        char captivePortalEnable[10]={0};
        unsigned int ethWanPort = 0xffffffff;

        if (FALSE == isEthWanEnabled())
        {
            return;
        }
        if (!syscfg_get(NULL, "redirection_flag", redirFlag, sizeof(redirFlag)) && !syscfg_get(NULL, "CaptivePortal_Enable", captivePortalEnable, sizeof(captivePortalEnable))){
          if (!strcmp(redirFlag,"true") && !strcmp(captivePortalEnable,"true"))
        EthWanSetLED(WHITE, BLINK, 1);
//Cox: Cp is disabled and need to show solid white
         if(!strcmp(captivePortalEnable,"false"))
         {
        CcspTraceInfo(("%s: CP disabled case and set led to solid white \n", __FUNCTION__));
        EthWanSetLED(WHITE, SOLID, 1);
         }
        }
    // phy link wan state should set to run dibbler client.
    v_secure_system("sysevent set phylink_wan_state up");

    v_secure_system("sysevent set ethwan-initialized 1");
    v_secure_system("syscfg set ntp_enabled 1"); // Enable NTP in case of ETHWAN
    v_secure_system("syscfg commit"); 
    if (RETURN_OK == CcspHalExtSw_getEthWanPort(&ethWanPort) )
    {
        ethWanPort += CCSP_HAL_ETHSW_EthPort1; /* ethWanPort starts from 0*/
        CcspTraceInfo(("WAN_MODE: Ethernet %d\n", ethGetPHYRate(ethWanPort)));
    }
    else
    {
        CcspTraceInfo(("WAN_MODE: Ethernet WAN Port Couldn't Be Retrieved\n"));
    }
	
#if defined (WAN_FAILOVER_SUPPORTED)
	publishEWanLinkStatus(true);
#endif

#endif

   if (ANSC_STATUS_SUCCESS == GetWan_InterfaceName (wanoe_ifname, sizeof(wanoe_ifname))) {
       // Update always Ethwan interface name into global structure if macsec is enabled.
#if defined(INTEL_PUMA7)
       CosaDmlEthPortSetName(ETHWAN_DEF_INTF_NAME,wanoe_ifname);
#endif
       CcspTraceInfo (("[%s][%d] WANOE [%s] interface link up event received \n", __FUNCTION__,__LINE__,wanoe_ifname));
        if ( TRUE == CosaDmlEthPortLinkStatusCallback (wanoe_ifname, WANOE_IFACE_UP)) {
            CcspTraceInfo (("[%s][%d] Successfully posted WANOE [%s] interface link up event to message queue\n", __FUNCTION__,__LINE__,wanoe_ifname));
        }else {
            CcspTraceError (("[%s][%d] Failed to post WANOE [%s] interface link up event to message queue\n", __FUNCTION__,__LINE__,wanoe_ifname));
        }
    }
}

/**
 * @Note Callback invoked upon wanoe interface link up from HAL.
 */
void EthWanLinkDown_callback() {
    char wanoe_ifname[WANOE_IFACENAME_LENGTH] = {0};

#ifdef AUTOWAN_ENABLE
    if (FALSE == isEthWanEnabled())
    {
        return;
    }

    v_secure_system("sysevent set phylink_wan_state down");
	
#if defined (WAN_FAILOVER_SUPPORTED)
	publishEWanLinkStatus(false);
#endif
	
#endif
    if (ANSC_STATUS_SUCCESS == GetWan_InterfaceName (wanoe_ifname, sizeof(wanoe_ifname))) {
       // Update always Ethwan interface name into global structure if macsec is enabled.
#if defined(INTEL_PUMA7)
       CosaDmlEthPortSetName(ETHWAN_DEF_INTF_NAME,wanoe_ifname);
#endif
        CcspTraceInfo (("[%s][%d] WANOE [%s] interface link down event received \n", __FUNCTION__,__LINE__,wanoe_ifname));
        if ( TRUE == CosaDmlEthPortLinkStatusCallback (wanoe_ifname, WANOE_IFACE_DOWN)) {
            CcspTraceInfo (("[%s][%d] Successfully posted WANOE [%s] interface link down event to message queue\n", __FUNCTION__,__LINE__,wanoe_ifname));
        }else {
            CcspTraceError (("[%s][%d] Failed to post WANOE [%s] interface link down event to message queue\n", __FUNCTION__,__LINE__,wanoe_ifname));
        }
    }
}
#endif // defined (FEATURE_RDKB_WAN_MANAGER)
#endif // defined (FEATURE_RDKB_WAN_MANAGER) || defined (FEATURE_RDKB_WAN_AGENT)
#ifdef FEATURE_RDKB_WAN_UPSTREAM
BOOL EthRdkInterfaceSetUpstream( PCOSA_DML_ETH_PORT_CONFIG pEthLink )
{
    int  ret = -1;
    char wanoe_ifname[WANOE_IFACENAME_LENGTH] = {0};

    if ( pEthLink == NULL )
    {
        AnscTraceError(("[%s][%d] Invalid pEthLink\n",__FUNCTION__, __LINE__));
        return FALSE;
    }

    if (ANSC_STATUS_SUCCESS != GetWan_InterfaceName (wanoe_ifname, sizeof(wanoe_ifname)))
    {
        AnscTraceError(("[%s][%d] GetWan_InterfaceName Failed\n",__FUNCTION__, __LINE__));
        return FALSE;
    }

    if (!strcmp(wanoe_ifname,pEthLink->Name))
    {
        ret =  CosaDmlSetWanOEMode(NULL, pEthLink );
    }

    AnscTraceInfo(("[%s][%d] X_RDK_Interface.UpStream set to [%s] [%s]\n",__FUNCTION__, __LINE__,
        ((pEthLink->Upstream) ? "Enable" : "Disable"),((ret == 0)?"Success":"Failed")));
    if(ret == 0)
    {
        return TRUE;
    }

     return FALSE;
}

BOOL EthInterfaceSetUpstream( PCOSA_DML_ETH_PORT_FULL pEthernetPortFull )
{
    int  ret = -1;

    if(pEthernetPortFull == NULL)
    {
        AnscTraceError(("[%s][%d] Null Pointer\n",__FUNCTION__, __LINE__));
        return FALSE;
    }

    AnscTraceInfo(("[%s][%d] EthName[%s] Upstream[%s]\n",__FUNCTION__, __LINE__, pEthernetPortFull->StaticInfo.Name, ((pEthernetPortFull->StaticInfo.bUpstream) ? "Enable" : "Disable")));

    if (strncmp(pEthernetPortFull->StaticInfo.Name, WAN_ETHERNET_IFNAME, strlen(WAN_ETHERNET_IFNAME)) == 0)
    {
        ret = CosaDmlSetWanOEMode(pEthernetPortFull,  NULL);
    }

    AnscTraceInfo(("[%s][%d] Interface.UpStream set to %s status[%s]\n",__FUNCTION__, __LINE__,
        ((pEthernetPortFull->StaticInfo.bUpstream) ? "Enable" : "Disable"),((ret == 0)?"Success":"Failed")));
    if(ret == 0)
    {
        return TRUE;
    }
    return FALSE;
}
#endif //FEATURE_RDKB_WAN_UPSTREAM

#if defined (FEATURE_RDKB_WAN_MANAGER)
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
int Ethagent_GetParamValuesFromPSM( char *pParamName, char *pReturnVal, int returnValLength )
{
    int     retPsmGet     = CCSP_SUCCESS;
    CHAR   *param_value   = NULL;

    /* Input Validation */
    if( ( NULL == pParamName) || ( NULL == pReturnVal ) || ( 0 >= returnValLength ) )
    {
        CcspTraceError(("%s Invalid Input Parameters\n",__FUNCTION__));
        return CCSP_FAILURE;
    }

    retPsmGet = PSM_Get_Record_Value2(bus_handle,g_Subsystem, pParamName, NULL, &param_value);
    if (retPsmGet != CCSP_SUCCESS)
    {
        CcspTraceError(("%s Error %d reading %s\n", __FUNCTION__, retPsmGet, pParamName));
    }
    else
    {
        /* Copy DB Value */
        snprintf(pReturnVal, returnValLength, "%s", param_value);
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
   return retPsmGet;
}

int Ethagent_SetParamValuesToPSM( char *pParamName, char *pParamVal )
{
    int     retPsmSet  = CCSP_SUCCESS;

    /* Input Validation */
    if( ( NULL == pParamName) || ( NULL == pParamVal ) )
    {
        CcspTraceError(("%s Invalid Input Parameters\n",__FUNCTION__));
        return CCSP_FAILURE;
    }

    CcspTraceInfo(("%s  pParamName[%s] pParamVal[%s]\n",__FUNCTION__,pParamName,pParamVal));

    retPsmSet = PSM_Set_Record_Value2(bus_handle,g_Subsystem, pParamName, ccsp_string, pParamVal);
    if (retPsmSet != CCSP_SUCCESS)
    {
        CcspTraceError(("%s Error %d writing %s\n", __FUNCTION__, retPsmSet, pParamName));
    }

    return retPsmSet;
}

ANSC_STATUS CosaDmlEthPortSetPortCapability( PCOSA_DML_ETH_PORT_CONFIG pEthLink )
{
    char WanOEInterface[16] = {0};
    char acGetParamName[256] = {0};
    char portCapability[16] = {0};
    int  ifIndex = -1;

    if (pEthLink->Name == NULL)
    {
        CcspTraceError(("%s Invalid data \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    if (ANSC_STATUS_SUCCESS == CosaDmlEthPortGetIndexFromIfName(pEthLink->Name, &ifIndex))
    {
        if (ANSC_STATUS_SUCCESS == CosaDmlGetWanOEInterfaceName(WanOEInterface, sizeof(WanOEInterface)))
        {
            if (0 == strcmp(pEthLink->Name, WanOEInterface))
            {
                if (pEthLink->PortCapability != PORT_CAP_LAN)
                {
                    if (0 == CosaDmlSetWanOEMode(NULL, pEthLink))
                    {
                        snprintf(acGetParamName, sizeof(acGetParamName), PSM_ETHAGENT_IF_PORTCAPABILITY, (ifIndex + 1));
                        Ethagent_SetParamValuesToPSM(acGetParamName,(pEthLink->PortCapability == PORT_CAP_WAN)?"WAN":"WAN_LAN");
                        CcspTraceInfo(("%s %d - [%s] PSM set \n", __FUNCTION__, __LINE__,acGetParamName));
                    }
                    else
                    {
                        CcspTraceError(("%s %d  CosaDmlSetWanOEMode failed \n",__FUNCTION__, __LINE__));
                        return ANSC_STATUS_FAILURE;
                    }
                }
                else
                {
                    CcspTraceWarning(("%s %d operation not allowed \n",__FUNCTION__, __LINE__));
                    return ANSC_STATUS_FAILURE;
                }
            }
            else
            {
                snprintf(acGetParamName, sizeof(acGetParamName), PSM_ETHAGENT_IF_PORTCAPABILITY, (ifIndex + 1));
                CcspTraceInfo(("%s %d - [%s] PSM set \n", __FUNCTION__, __LINE__,acGetParamName));
                if (pEthLink->PortCapability == PORT_CAP_LAN)
                {
                    strcpy(portCapability, "LAN");
                }
                else if (pEthLink->PortCapability == PORT_CAP_WAN)
                {
                    strcpy(portCapability, "WAN");
                }
                else
                {
                    strcpy(portCapability, "WAN_LAN");
                }
                Ethagent_SetParamValuesToPSM(acGetParamName,portCapability);
            }
        }
        else
        {
            CcspTraceError(("%s %d  CosaDmlGetWanOEInterfaceName failed \n",__FUNCTION__, __LINE__));
            return ANSC_STATUS_FAILURE;
        }
    }
    else
    {
        CcspTraceError(("%s %d CosaDmlEthPortGetIndexFromIfName failed \n",__FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }
    return ANSC_STATUS_SUCCESS;
}
#endif //FEATURE_RDKB_AUTO_PORT_SWITCH
#endif //FEATURE_RDKB_WAN_MANAGER
