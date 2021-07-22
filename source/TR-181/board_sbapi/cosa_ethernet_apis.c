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
#endif //FEATURE_RDKB_WAN_MANAGER

#include "syscfg.h"
#ifdef FEATURE_SUPPORT_ONBOARD_LOGGING


#define LOGGING_MODULE           "ETHAGENT"
#define OnboardLog(...)         rdk_log_onboard(LOGGING_MODULE, __VA_ARGS__)
#else
#define OnboardLog(...)
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
       return -1;

    AnscCopyString(ifr.ifr_name, (char*)name);

    if (!isValid((char*)name)) {
        return -1;
    }
    AnscTraceFlow(("%s...\n", __FUNCTION__));
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
        if (errno == ENODEV) {
            close(skfd);
            return -1;
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

#if defined(_PLATFORM_RASPBERRYPI_) || defined(_PLATFORM_TURRIS_)
#define ETHWAN_DEF_INTF_NAME "eth0"
#endif

/* Provide a fallback definition - not expected to be used, but enough for the code to compile */
#if defined (FEATURE_RDKB_WAN_MANAGER) && ! defined(ETHWAN_DEF_INTF_NAME)
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

pthread_t bootInformThreadId;
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
#if defined (FEATURE_RDKB_WAN_MANAGER)
void getWanMacAddress(macaddr_t* macAddr,char *pIfname);
int EthWanSetLED (int color, int state, int interval);
#endif
#if defined (FEATURE_RDKB_WAN_AGENT)
static ANSC_STATUS CosaDmlEthSetParamValues(char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, unsigned int bCommitFlag);
#elif defined (FEATURE_RDKB_WAN_MANAGER)
static ANSC_STATUS CosaDmlEthSetParamValues(const char *pComponent, const char *pBus, const char *pParamName, const char *pParamVal, enum dataType_e type, unsigned int bCommitFlag);
static ANSC_STATUS DmlEthCheckIfaceConfiguredAsPPPoE( char *ifname, BOOL *isPppoeIface);
static ANSC_STATUS  GetWan_InterfaceName (char* wanoe_ifacename, int length);
INT gTotal = TOTAL_NUMBER_OF_INTERNAL_INTERFACES;
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
                        else
                        {
                                if (syscfg_commit() != 0)
                                {
                                        AnscTraceWarning(("RDKB_REBOOT : RebootDevice syscfg_commit failed for ETHWAN mode\n"));
                                }
                        }


                        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_LastRebootCounter", "1") != 0)
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

#if defined (FEATURE_RDKB_WAN_MANAGER)

ANSC_STATUS CosaDmlIfaceFinalize(char *pValue)
{
    char wanPhyName[64] = {0};
    char buf[64] = {0};
    char ethwan_ifname[64] = {0};
    int ovsEnable = 0;
    BOOL ethwanEnabled = FALSE;

    if (!pValue)
        return CCSP_FAILURE;
    CcspTraceError(("Func %s Entered\n",__FUNCTION__));


    if (syscfg_get(NULL, "eth_wan_enabled", buf, sizeof(buf)) == 0)
    {
        if ( 0 == strcmp(buf,"true"))
        {
            if ( 0 == access( "/nvram/ETHWAN_ENABLE" , F_OK ) )
            {
                ethwanEnabled = TRUE;
                CcspTraceInfo(("Ethwan enabled \n"));
            }
        }
    }

#if defined (_BRIDGE_UTILS_BIN_)
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

    if (!syscfg_get(NULL, "wan_physical_ifname", buf, sizeof(buf)))
    {
        strcpy(wanPhyName, buf);
        printf("wanPhyName = %s\n", wanPhyName);
    }
    else
    {
        strcpy(wanPhyName, "erouter0");

    }

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
   
    if (ethwanEnabled == TRUE)
    {
        if (ovsEnable)
        {
            v_secure_system("/usr/bin/bridgeUtils del-port brlan0 %s",ethwan_ifname);
        }
        else
        {
            v_secure_system("brctl delif brlan0 %s",ethwan_ifname);
        }
        v_secure_system("brctl addif %s %s", wanPhyName,ethwan_ifname);
    }
    else
    {

        v_secure_system("brctl delif %s %s", wanPhyName,ethwan_ifname);

        if (ovsEnable)
        {
            v_secure_system("/usr/bin/bridgeUtils add-port brlan0 %s",ethwan_ifname);
        }
        else
        {
            v_secure_system("brctl addif brlan0 %s",ethwan_ifname);
        }

    }
    if ( 0 != access( "/tmp/autowan_iface_finalized" , F_OK ) )
    {
        v_secure_system("touch /tmp/autowan_iface_finalized");
    }
    return ANSC_STATUS_SUCCESS;
}


ANSC_STATUS EthwanEnableWithoutReboot(BOOL bEnable)
{
    CcspTraceError(("Func %s Entered arg %d\n",__FUNCTION__,bEnable));
    
    if(bEnable == FALSE)
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

        if ( syscfg_set( NULL, "eth_wan_enabled", buf ) != 0 )
        {
            CcspTraceError(( "syscfg_set failed for eth_wan_enabled\n" ));
            return RETURN_ERR;
        }
        else
        {
            if ( syscfg_commit() != 0 )
            {
                CcspTraceError(( "syscfg_commit failed for eth_wan_enabled\n" ));
                return RETURN_ERR;
            }

        }
    }
    CcspTraceError(("Func %s Exited arg %d\n",__FUNCTION__,bEnable));
 
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
       return TRUE;
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

ANSC_STATUS CosaDmlConfigureEthWan(BOOL bEnable)
{
    char wanPhyName[64] = {0};
    char out_value[64] = {0};
    char buf[64] = {0};
    char ethwan_ifname[64] = {0};
    int lastKnownWanMode = -1;
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

        if ( syscfg_set( NULL, "eth_wan_iface_name", ethwan_ifname) != 0 )
        {
            CcspTraceError(( "syscfg_set failed for eth_wan_iface_name\n" ));
        }
        else
        {
            if ( syscfg_commit() != 0 )
            {
                CcspTraceError(( "syscfg_commit failed for eth_wan_iface_name\n" ));
            }

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
        getWanMacAddress(&macAddr,wanPhyName);
        char wan_mac[18];// = {0};
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

        EthwanEnableWithoutReboot(TRUE);

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
        if ( syscfg_set( NULL, "eth_wan_iface_name", ethwan_ifname) != 0 )
        {
            CcspTraceError(( "syscfg_set failed for eth_wan_iface_name\n" ));
        }
        else
        {
            if ( syscfg_commit() != 0 )
            {
                CcspTraceError(( "syscfg_commit failed for eth_wan_iface_name\n" ));
            }

        }
#endif
        v_secure_system("ifconfig %s up", ethwan_ifname);

    }
    else
    {
        EthwanEnableWithoutReboot(FALSE);
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
        v_secure_system("ifconfig %s up",ethwan_ifname);
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
#if defined (ENABLE_ETH_WAN)
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
		char buf[ 8 ] = {0};
		snprintf( buf, sizeof( buf ), "%s", bEnable ? "true" : "false" );

		if(bEnable)
		{
			v_secure_system("touch /nvram/ETHWAN_ENABLE");
		}
		else
		{
			v_secure_system("rm /nvram/ETHWAN_ENABLE");
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

void getWanMacAddress(macaddr_t* macAddr,char *pIfname)
{
    FILE *f = NULL;
    char line[256], *lineptr = line, fname[128];
    size_t size;
    int i, macaddr[18];

    if (pIfname)
    {
        snprintf(fname,sizeof(fname), "/sys/class/net/%s/address", pIfname);
        size = sizeof(line);
        if ((f = fopen(fname, "r")))
        {
            if ((getline(&lineptr, &size, f) >= 0))
            {
                sscanf(lineptr, "%02x:%02x:%02x:%02x:%02x:%02x", &macaddr[0], &macaddr[1], &macaddr[2], &macaddr[3], &macaddr[4], &macaddr[5]);
                for (i = 0; i < 18; i++)
                    macAddr->hw[i] = (uint8_t) macaddr[i];
            }
            fclose(f);
        }
    }

    return;
}

ANSC_STATUS EthWanBridgeInit()
{
    char wanPhyName[64] = {0};
    char out_value[64] = {0};
    char ethwan_ifname[64] = {0};
   
#if defined (_BRIDGE_UTILS_BIN_)
    int ovsEnable = 0;
    char buf[ 8 ] = { 0 };
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

            CcspTraceError(("Ethwan interface %s \n",ethwan_ifname));
        macaddr_t macAddr;
       getWanMacAddress(&macAddr,wanPhyName);
            char wan_mac[18];// = {0};
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
    v_secure_system("brctl addif %s dpdmta1",wanPhyName);
    v_secure_system("echo 1 > /sys/devices/virtual/net/%s/bridge/nf_disable_iptables",wanPhyName);
    v_secure_system("echo 1 > /sys/devices/virtual/net/%s/bridge/nf_disable_ip6tables",wanPhyName);
    v_secure_system("echo 1 > /sys/devices/virtual/net/%s/bridge/nf_disable_arptables",wanPhyName);
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

#ifdef AUTOWAN_ENABLE 
    {
        PCOSA_DATAMODEL_ETH_WAN_AGENT pEthWanCfg = NULL;
        char buf[64];
        BOOL ethwanEnabled = FALSE;

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

        if (syscfg_get(NULL, "eth_wan_enabled", buf, sizeof(buf)) == 0)
        {
            if ( 0 == strcmp(buf,"true"))
            {
                if ( 0 == access( "/nvram/ETHWAN_ENABLE" , F_OK ) )
                {
                    ethwanEnabled = TRUE;
                    CcspTraceInfo(("Ethwan enabled \n"));
                }
            }
        }

        if (TRUE == ethwanEnabled)
        {
            EthWanBridgeInit();

            //Initialise ethsw-hal to get event notification from lower layer.
            if (CcspHalEthSwInit() != RETURN_OK)
            {
                CcspTraceError(("Hal initialization failed \n"));
                return ANSC_STATUS_FAILURE;
            }
        }
    }
#else
    #if defined(_PLATFORM_RASPBERRYPI_)
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
    //ETH Port Init.
    CosaDmlEthPortInit((PANSC_HANDLE)pMyObject);
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
        if(GWP_GetEthWanLinkStatus() == 1) {
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
    if (IfIndex < TOTAL_NUMBER_OF_INTERNAL_INTERFACES)
    {
        CcspTraceError(("%s Invalid or Default index[%d]\n", __FUNCTION__, IfIndex));
        return ANSC_STATUS_FAILURE;
    }
    pthread_mutex_lock(&gmEthGInfo_mutex);
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
#endif

   if (ANSC_STATUS_SUCCESS == GetWan_InterfaceName (wanoe_ifname, sizeof(wanoe_ifname))) {
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
    v_secure_system("sysevent set phylink_wan_state down");
#endif
    if (ANSC_STATUS_SUCCESS == GetWan_InterfaceName (wanoe_ifname, sizeof(wanoe_ifname))) {
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
