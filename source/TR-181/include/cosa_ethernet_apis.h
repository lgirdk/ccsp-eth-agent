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

    module: cosa_ethernet_apis.h

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------


    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/11/2011    initial revision.

**************************************************************************/


#ifndef  _COSA_ETHERNET_API_H
#define  _COSA_ETHERNET_API_H
#include <stdbool.h>
#include "../middle_layer_src/cosa_apis.h"
#include "../middle_layer_src/plugin_main_apis.h"
#include "ccsp_hal_ethsw.h"

#define WAN_ETHERNET_IFNAME "erouter0"

#define PSM_ETHMANAGER_CFG_UPSTREAM      "dmsb.ethagent.if.%d.Upstream"
#define PSM_ETHMANAGER_CFG_ADDTOBRIDGE   "dmsb.ethagent.if.%d.AddToLanBridge"
#ifdef UNIT_TEST_DOCKER_SUPPORT
#define STATIC
#else
#define STATIC static
#endif

/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/
#if defined (FEATURE_RDKB_WAN_AGENT)
#define ETHERNET_IF_PATH                  "Device.Ethernet.X_RDKCENTRAL-COM_Interface."

//VLAN Agent - ETHERNET.LINK
#define VLAN_DBUS_PATH                    "/com/cisco/spvtg/ccsp/vlanagent"
#define VLAN_COMPONENT_NAME               "eRT.com.cisco.spvtg.ccsp.vlanagent"

#define VLAN_ETH_LINK_TABLE_NAME          "Device.X_RDKCENTRAL-COM_Ethernet.Link."
#define VLAN_ETH_NOE_PARAM_NAME           "Device.X_RDKCENTRAL-COM_Ethernet.LinkNumberOfEntries"
#define VLAN_ETH_LINK_PARAM_ALIAS         "Device.X_RDKCENTRAL-COM_Ethernet.Link.%d.Alias"
#define VLAN_ETH_LINK_PARAM_NAME          "Device.X_RDKCENTRAL-COM_Ethernet.Link.%d.Name"
#define VLAN_ETH_LINK_PARAM_LOWERLAYERS   "Device.X_RDKCENTRAL-COM_Ethernet.Link.%d.LowerLayers"
#define VLAN_ETH_LINK_PARAM_ENABLE        "Device.X_RDKCENTRAL-COM_Ethernet.Link.%d.Enable"

#define ETH_IF_PHY_PATH                   "Device.Ethernet.X_RDKCENTRAL-COM_Interface.%d"

#elif defined (FEATURE_RDKB_WAN_MANAGER)

#define BUFLEN_256 256
#define BUFLEN_32  32
#define WAN_INTERFACE_NAME         "erouter0"

#define ETHERNET_IF_PATH                  "Device.Ethernet.X_RDK_Interface."
#define ETHERNET_IF_LOWERLAYERS           "Device.Ethernet.X_RDK_Interface."

//VLAN Agent - ETHERNET.LINK
#define VLAN_DBUS_PATH                    "/com/cisco/spvtg/ccsp/vlanmanager"
#define VLAN_COMPONENT_NAME               "eRT.com.cisco.spvtg.ccsp.vlanmanager"

#define VLAN_ETH_LINK_TABLE_NAME          "Device.X_RDK_Ethernet.Link."
#define VLAN_ETH_NOE_PARAM_NAME           "Device.X_RDK_Ethernet.LinkNumberOfEntries"

#define VLAN_TERM_PARAM_ENABLE            "Device.X_RDK_Ethernet.VLANTermination.%d.Enable"

//WAN Agent
#define WAN_DBUS_PATH                     "/com/cisco/spvtg/ccsp/wanmanager"
#define WAN_COMPONENT_NAME                "eRT.com.cisco.spvtg.ccsp.wanmanager"
#define WAN_COMP_NAME_WITHOUT_SUBSYSTEM   "com.cisco.spvtg.ccsp.wanmanager"

#if defined(WAN_MANAGER_UNIFICATION_ENABLED)
#define WAN_NOE_PARAM_NAME                "Device.X_RDK_WanManager.InterfaceNumberOfEntries"
#define WAN_PHY_STATUS_PARAM_NAME         "Device.X_RDK_WanManager.Interface.%d.BaseInterfaceStatus"
#define WAN_PHY_PATH_PARAM_NAME           "Device.X_RDK_WanManager.Interface.%d.BaseInterface"
#define WAN_LINK_STATUS_PARAM_NAME        "Device.X_RDK_WanManager.Interface.%d.VirtualInterface.1.VlanStatus"
#define WAN_INTERFACE_PARAM_NAME          "Device.X_RDK_WanManager.Interface.%d.VirtualInterface.1.Name"
#define WAN_STATUS_PARAM_NAME             "Device.X_RDK_WanManager.Interface.%d.Selection.Status"
#define WAN_IF_NAME_PARAM_NAME            "Device.X_RDK_WanManager.Interface.%d.Name"

#define WAN_BOOTINFORM_CUSTOMCONFIG_PARAM_NAME         "Device.X_RDK_WanManager.Interface.%d.EnableCustomConfig"
#define WAN_BOOTINFORM_CUSTOMCONFIGPATH_PARAM_NAME     "Device.X_RDK_WanManager.Interface.%d.CustomConfigPath"
#define WAN_BOOTINFORM_CONFIGWANENABLE_PARAM_NAME      "Device.X_RDK_WanManager.Interface.%d.ConfigureWanEnable"
#define WAN_BOOTINFORM_OPERSTATUSENABLE_PARAM_NAME     "Device.X_RDK_WanManager.Interface.%d.EnableOperStatusMonitor"
#define WAN_BOOTINFORM_PHYPATH_PARAM_NAME              "Device.X_RDK_WanManager.Interface.%d.BaseInterface"
#define WAN_BOOTINFORM_INTERFACE_PARAM_NAME            "Device.X_RDK_WanManager.Interface.%d.VirtualInterface.1.Name"
#else
#define WAN_NOE_PARAM_NAME                "Device.X_RDK_WanManager.CPEInterfaceNumberOfEntries"
#define WAN_PHY_STATUS_PARAM_NAME         "Device.X_RDK_WanManager.CPEInterface.%d.Phy.Status"
#define WAN_PHY_PATH_PARAM_NAME           "Device.X_RDK_WanManager.CPEInterface.%d.Phy.Path"
#define WAN_LINK_STATUS_PARAM_NAME        "Device.X_RDK_WanManager.CPEInterface.%d.Wan.LinkStatus"
#define WAN_INTERFACE_PARAM_NAME          "Device.X_RDK_WanManager.CPEInterface.%d.Wan.Name"
#define WAN_STATUS_PARAM_NAME             "Device.X_RDK_WanManager.CPEInterface.%d.Wan.Status"
#define WAN_IF_NAME_PARAM_NAME            "Device.X_RDK_WanManager.CPEInterface.%d.Name"

#define WAN_BOOTINFORM_CUSTOMCONFIG_PARAM_NAME           "Device.X_RDK_WanManager.CPEInterface.%d.EnableCustomConfig"
#define WAN_BOOTINFORM_CUSTOMCONFIGPATH_PARAM_NAME       "Device.X_RDK_WanManager.CPEInterface.%d.CustomConfigPath"
#define WAN_BOOTINFORM_CONFIGWANENABLE_PARAM_NAME        "Device.X_RDK_WanManager.CPEInterface.%d.ConfigureWanEnable"
#define WAN_BOOTINFORM_OPERSTATUSENABLE_PARAM_NAME       "Device.X_RDK_WanManager.CPEInterface.%d.EnableOperStatusMonitor"
#define WAN_BOOTINFORM_PHYPATH_PARAM_NAME                "Device.X_RDK_WanManager.CPEInterface.%d.Phy.Path"
#define WAN_BOOTINFORM_INTERFACE_PARAM_NAME              "Device.X_RDK_WanManager.CPEInterface.%d.Wan.Name"
#endif /** WAN_MANAGER_UNIFICATION_ENABLED */

#define WAN_BOOTINFORM_CUSTOMCONFIGPATH_PARAM_VALUE "Device.X_CISCO_COM_CableModem."
#define WAN_ENABLE_PARAM "Device.X_RDK_WanManager.Enable"
#define WAN_IF_NAME_PRIMARY "erouter0"
#define WAN_PHYPATH_VALUE "Device.X_RDKCENTRAL-COM_EthernetWAN."
#define WAN_CM_INTERFACE_INSTANCE_NUM      1
#define WAN_ETH_INTERFACE_INSTANCE_NUM     2

#define ETH_IF_PHY_PATH                   "Device.Ethernet.X_RDK_Interface.%d"

#ifdef _COSA_BCM_ARM_
#define ETHWAN_DOCSIS_INF_NAME "cm0"
#elif defined(INTEL_PUMA7) 
#define ETHWAN_DOCSIS_INF_NAME "dpdmta1"
#else
#define ETHWAN_DOCSIS_INF_NAME "cm0"
#endif

#define PHY_STATUS_MONITOR_MAX_TIMEOUT 240
#define PHY_STATUS_QUERY_INTERVAL 2
#define CFG_TR181_ETH_BORROW_MAC  1
#define OVS_DIR_PATH "/sys/module/openvswitch"
#if defined (FEATURE_RDKB_WAN_MANAGER)
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH

#define PSM_ETHAGENT_IF_PORTCAPABILITY "dmsb.ethagent.if.%d.PortCapability"
int Ethagent_GetParamValuesFromPSM( char *pParamName, char *pReturnVal, int returnValLength );
int Ethagent_SetParamValuesToPSM( char *pParamName, char *pParamVal );

typedef enum
_COSA_DML_ETH_PORT_CAP_OPER
{
    PORT_CAP_WAN = 1,
    PORT_CAP_LAN,
    PORT_CAP_WAN_LAN
} COSA_DML_ETH_PORT_CAP_OPER;
#endif  //FEATURE_RDKB_AUTO_PORT_SWITCH
#endif

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
#if defined(INTEL_PUMA7)
typedef enum
{
    NF_ARPTABLE,
    NF_IPTABLE,
    NF_IP6TABLE
} bridge_nf_table_t;
#endif
typedef struct _WAN_PARAM_INFO
{
    CHAR paramName[256];
    CHAR paramValue[256];
    enum dataType_e paramType;
}WAN_PARAM_INFO;

enum WANBOOTINFORM_MSG
{
    MSG_WAN_NAME = 0,
    MSG_PHY_PATH,
    MSG_CONFIGURE_WAN,
    MSG_CUSTOMCONFIG_ENABLE,
    MSG_CUSTOMCONFIG_PATH,
    MSG_OPER_STATUS,
    MSG_TOTAL_NUM
};
typedef struct _WAN_BOOTINFORM_MSG
{
    WAN_PARAM_INFO param[MSG_TOTAL_NUM];
    INT iNumOfParam;
    INT iWanInstanceNumber;
}WAN_BOOTINFORM_MSG;

typedef enum WanOperStateAck
{
    WAN_OPER_STATE_NONE = 0,
    WAN_OPER_STATE_RESET_INPROGRESS,
    WAN_OPER_STATE_RESET_COMPLETED
}WanOperStateAck;

typedef struct _WAN_MODE_BRIDGECFG
{
    INT bridgemode;
    BOOL ovsEnabled;
    BOOL ethWanEnabled;
    BOOL configureBridge;
    BOOL meshEbEnabled;
    CHAR ethwan_ifname[64];
    CHAR wanPhyName[64];
}WAN_MODE_BRIDGECFG;

typedef INT (*WanBridgeCfgHandler)(WAN_MODE_BRIDGECFG *pcfg);
#endif //FEATURE_RDKB_WAN_MANAGER

typedef struct _Eth_host_t
{
    UCHAR eth_macAddr[6];
    INT eth_port;
    BOOL eth_Active;
    struct _Eth_host_t* next;
}Eth_host_t;

typedef  struct
_COSA_DATAMODEL_ETH_WAN_AGENT_CONTENT
{
    BOOLEAN                Enable;
    ULONG             	    Port;
    BOOLEAN                MonitorPhyStatusAndNotify;
    char                   wanInstanceNumber[4];
#if defined (FEATURE_RDKB_WAN_MANAGER)
    WanOperStateAck        wanOperState;
    INT                    PrevSelMode;
#endif
    char                   ethWanIfMac[32];
}
COSA_DATAMODEL_ETH_WAN_AGENT, *PCOSA_DATAMODEL_ETH_WAN_AGENT;

struct _COSA_DML_ETH_LOG_STATUS
{
    ULONG       Log_Period;
    BOOLEAN     Log_Enable;	
}_struct_pack_;

typedef struct _COSA_DML_ETH_LOG_STATUS COSA_DML_ETH_LOG_STATUS, *PCOSA_DML_ETH_LOG_STATUS;

/* Enum to store link status (up/down) of ethernet ports. */
typedef enum
_COSA_DML_ETH_LINK_STATUS
{
    ETH_LINK_STATUS_UP = 1,
    ETH_LINK_STATUS_DOWN
} COSA_DML_ETH_LINK_STATUS;

/** Enum to store wan status of port .( up/down) */
typedef enum
_COSA_DML_ETH_WAN_STATUS
{
    ETH_WAN_UP = 1,
    ETH_WAN_DOWN
} COSA_DML_ETH_WAN_STATUS;

#if defined(FEATURE_RDKB_WAN_MANAGER)
typedef enum
_COSA_DML_ETH_TABLE_OPER
{
    ETH_ADD_TABLE = 1,
    ETH_DEL_TABLE
} COSA_DML_ETH_TABLE_OPER;
#endif // FEATURE_RDKB_WAN_MANAGER

/* Structure to hold port configuration data. */
typedef struct
_COSA_DML_ETH_PORT_CONFIG
{
    ULONG ulInstanceNumber; /* Instance number. */
    CHAR Name[64]; /* Interface name. eth0, eth1 etc */
#ifdef FEATURE_RDKB_WAN_MANAGER
    BOOL Enable;
#endif
    COSA_DML_ETH_LINK_STATUS LinkStatus; /* Link status - up/down */
    COSA_DML_ETH_WAN_STATUS WanStatus; /* Wan link status - Up/down */
    BOOL Upstream; /* Indicates interface is used for WAN. */
    BOOL WanValidated; /* Is it is a valid WAN interface */
    BOOL AddToLanBridge; /* TRUE, if interface is in LAN bridge, FALSE if interface not in LAN bridge */
    CHAR Path[128]; /* contains current table path */
#ifdef FEATURE_RDKB_WAN_MANAGER
    CHAR LowerLayers[128]; /* contains LowerLayers */
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
    COSA_DML_ETH_PORT_CAP_OPER PortCapability;
#endif //FEATURE_RDKB_AUTO_PORT_SWITCH
#endif
} COSA_DML_ETH_PORT_CONFIG, *PCOSA_DML_ETH_PORT_CONFIG;

/* Structure to hold global data used for wan connection management. */
typedef struct
_COSA_DML_ETH_PORT_GLOBAL_CONFIG
{
    BOOL Upstream;
    CHAR Name[64];                       /* Name of interface. */
#ifdef FEATURE_RDKB_WAN_MANAGER
    BOOL Enable;
#endif
    COSA_DML_ETH_LINK_STATUS LinkStatus; /* Link status of interface. */
    COSA_DML_ETH_WAN_STATUS WanStatus; /* Wan link status. */
    BOOL WanValidated;
    CHAR Path[128]; /* contains current table path */
#ifdef FEATURE_RDKB_WAN_MANAGER
    CHAR LowerLayers[128]; /* contains LowerLayers */
#endif
} COSA_DML_ETH_PORT_GLOBAL_CONFIG, *PCOSA_DML_ETH_PORT_GLOBAL_CONFIG;

/* Enum wan status. */
typedef enum _COSA_ETH_NOTIFY_ENUM
{
    NOTIFY_TO_WAN_AGENT = 1,
    NOTIFY_TO_VLAN_AGENT
} COSA_ETH_NOTIFY_ENUM;

typedef enum _COSA_ETH_VLAN_OBJ_TYPE
{
    VLAN_ETH_LINK_OBJ = 1,
    VLAN_TERMINATION_OBJ
} COSA_ETH_VLAN_OBJ_TYPE;

/*
    Standard function declaration
*/
#define MAXINSTANCE 128
#define ETH_INTERFACE_MAX_ASSOC_DEVICES   256
#define MAC_SZ  6 /* Mac address in Hex format */
#define MACADDR_SZ 17 /* Expanded MAC address in 00:02:... format */

typedef  enum
_COSA_DML_ETH_DUPLEX_MODE
{
    COSA_DML_ETH_DUPLEX_Half        = 1,
    COSA_DML_ETH_DUPLEX_Full,
    COSA_DML_ETH_DUPLEX_Auto
}
COSA_DML_ETH_DUPLEX_MODE, *PCOSA_DML_ETH_DUPLEX_MODE;

typedef  struct
_COSA_DML_ETH_STATS
{
    ULLONG                           BytesSent;
    ULLONG                           BytesReceived;
    ULONG                           PacketsSent;
    ULONG                           PacketsReceived;
    ULONG                           ErrorsSent;
    ULONG                           ErrorsReceived;
    ULONG                           UnicastPacketsSent;
    ULONG                           UnicastPacketsReceived;
    ULONG                           DiscardPacketsSent;
    ULONG                           DiscardPacketsReceived;
    ULONG                           MulticastPacketsSent;
    ULONG                           MulticastPacketsReceived;
    ULONG                           BroadcastPacketsSent;
    ULONG                           BroadcastPacketsReceived;
    ULONG                           UnknownProtoPacketsReceived;
}
COSA_DML_ETH_STATS, *PCOSA_DML_ETH_STATS;

typedef  struct
_COSA_DML_ETH_PORT_CFG
{
    ULONG                           InstanceNumber;
    char                            Alias[COSA_DML_IF_NAME_LENGTH];

    BOOLEAN                         bEnabled;
    LONG                            MaxBitRate;
    COSA_DML_ETH_DUPLEX_MODE        DuplexMode;
}
COSA_DML_ETH_PORT_CFG,  *PCOSA_DML_ETH_PORT_CFG;

typedef  struct
_COSA_DML_ETH_PORT_SINFO
{
    char                            Name[COSA_DML_IF_NAME_LENGTH];
    BOOLEAN                         bUpstream;
    UCHAR                           MacAddress[6];
}
COSA_DML_ETH_PORT_SINFO,  *PCOSA_DML_ETH_PORT_SINFO;

/*
 *  Dynamic portion of Ethernet port info
 */

typedef  struct
_COSA_DML_ASSOCDEV_INFO
{
    UCHAR                           MacAddress[MACADDR_SZ+1];
}
COSA_DML_ASSOCDEV_INFO, *PCOSA_DML_ASSOCDEV_INFO;

typedef  struct
_COSA_DML_ETH_PORT_DINFO
{
    COSA_DML_IF_STATUS              Status;
    ULONG                           CurrentBitRate;
    ULONG                           LastChange;
    ULONG                           AssocDevicesCount;
    UCHAR                           AssocDevices[(MAC_SZ*ETH_INTERFACE_MAX_ASSOC_DEVICES)];
}
COSA_DML_ETH_PORT_DINFO,  *PCOSA_DML_ETH_PORT_DINFO;

typedef  struct
_COSA_DML_ETH_PORT_FULL
{
    COSA_DML_ETH_PORT_CFG           Cfg;
    COSA_DML_ETH_PORT_SINFO         StaticInfo;
    COSA_DML_ETH_PORT_DINFO         DynamicInfo;
    COSA_DML_ASSOCDEV_INFO          AssocClient[ETH_INTERFACE_MAX_ASSOC_DEVICES];
}
COSA_DML_ETH_PORT_FULL, *PCOSA_DML_ETH_PORT_FULL;

/* pCfg Identified by InstanceNumber */
ANSC_STATUS CosaDmlEthPortSetCfg ( ANSC_HANDLE hContext, PCOSA_DML_ETH_PORT_CFG pCfg );

ANSC_STATUS CosaDmlEthPortGetCfg ( ANSC_HANDLE hContext, PCOSA_DML_ETH_PORT_CFG pCfg );

ANSC_STATUS CosaDmlEthPortSetValues ( ANSC_HANDLE hContext, ULONG ulIndex, ULONG ulInstanceNumber, char* pAlias );

ANSC_STATUS CosaDmlEthPortGetDinfo ( ANSC_HANDLE hContext, ULONG ulInstanceNumber, PCOSA_DML_ETH_PORT_DINFO pInfo );

ANSC_STATUS CosaDmlEthPortGetClientMac ( PCOSA_DML_ETH_PORT_FULL pEthernetPortFull, ULONG ulInstanceNumber );

ANSC_STATUS CosaDmlEthPortGetEntry ( ANSC_HANDLE hContext, ULONG ulIndex, PCOSA_DML_ETH_PORT_FULL pEntry );

ANSC_STATUS CosaDmlEthPortGetStats ( ANSC_HANDLE hContext, ULONG ulInstanceNumber, PCOSA_DML_ETH_STATS pStats );

ANSC_STATUS CosaDmlEthInterfaceInit ( ANSC_HANDLE hDml, PANSC_HANDLE phContext );

ANSC_STATUS CosaDmlEthPortGetNumberOfEntries( ANSC_HANDLE hContext );


ANSC_STATUS
CosaDmlEthInit
    (
        ANSC_HANDLE                 hDml,
        PANSC_HANDLE                phContext
    );

ANSC_STATUS
CosaDmlEthPortInit
    (
        PANSC_HANDLE                phContext
    );
#if defined (FEATURE_RDKB_WAN_MANAGER)
ANSC_STATUS EthwanEnableWithoutReboot(BOOL bEnable);
BOOL CosaDmlEthWanLinkStatus();
ANSC_STATUS CosaDmlConfigureEthWan(BOOL bEnable);
ANSC_STATUS CosaDmlEthWanPhyStatusMonitor(void *arg);
ANSC_STATUS CosaDmlIfaceFinalize(char *pValue, BOOL isAutoWanMode);
BOOL isEthWanEnabled();
#endif
#if defined (FEATURE_RDKB_WAN_AGENT)
ANSC_STATUS CosaDmlEthGetPortCfg( INT nIndex, PCOSA_DML_ETH_PORT_CONFIG pEthLink);
ANSC_STATUS CosaDmlEthPortSetUpstream( INT IfIndex, BOOL Upstream );
ANSC_STATUS CosaDmlEthPortSetWanValidated(INT IfIndex, BOOL WanValidated);
ANSC_STATUS CosaDmlEthPortGetWanStatus( INT IfIndex, COSA_DML_ETH_WAN_STATUS *wan_status );
ANSC_STATUS CosaDmlEthPortSetWanStatus( INT IfIndex, COSA_DML_ETH_WAN_STATUS wan_status);
ANSC_STATUS CosaDmlEthPortGetLinkStatus( INT IfIndex, COSA_DML_ETH_LINK_STATUS *LinkStatus );
INT CosaDmlEthPortLinkStatusCallback( CHAR *ifname, CHAR* state );
ANSC_STATUS CosaDmlEthPortGetCopyOfGlobalInfoForGivenIfName(char* ifName, PCOSA_DML_ETH_PORT_GLOBAL_CONFIG pGlobalInfo );
ANSC_STATUS CosaDmlEthSetWanStatusForWanAgent(char *ifname, char *WanStatus);
ANSC_STATUS CosaDmlEthCreateEthLink(char *l2ifName, char *Path);
ANSC_STATUS CosaDmlEthDeleteEthLink(char *ifName, char *Path);
ANSC_STATUS CosaDmlEthGetPhyStatusForWanAgent( char *ifname, char *PhyStatus );
ANSC_STATUS CosaDmlEthSetPhyStatusForWanAgent( char *ifname, char *PhyStatus );
#elif defined(FEATURE_RDKB_WAN_MANAGER)
ANSC_STATUS CosaDmlEthGetPortCfg( INT nIndex, PCOSA_DML_ETH_PORT_CONFIG pEthLink);
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
ANSC_STATUS CosaDmlEthPortSetPortCapability( PCOSA_DML_ETH_PORT_CONFIG pEthLink );
#endif
#ifdef FEATURE_RDKB_WAN_UPSTREAM
BOOL EthInterfaceSetUpstream( PCOSA_DML_ETH_PORT_FULL pEthernetPortFull );
BOOL EthRdkInterfaceSetUpstream( PCOSA_DML_ETH_PORT_CONFIG pEthLink ); 
#endif
ANSC_STATUS CosaDmlEthPortSetUpstream( CHAR *ifname, BOOL Upstream );
ANSC_STATUS CosaDmlEthPortSetWanValidated(INT IfIndex, BOOL WanValidated);
ANSC_STATUS CosaDmlEthPortGetWanStatus( CHAR *ifname, COSA_DML_ETH_WAN_STATUS *wan_status );
ANSC_STATUS CosaDmlEthPortSetWanStatus( CHAR *ifname, COSA_DML_ETH_WAN_STATUS wan_status);
ANSC_STATUS CosaDmlEthPortSetLowerLayers(char *ifname, char *newLowerLayers);
ANSC_STATUS CosaDmlTriggerExternalEthPortLinkStatus(char *ifname, BOOL status);
ANSC_STATUS CosaDmlEthPortGetLinkStatus( CHAR *ifname, COSA_DML_ETH_LINK_STATUS *LinkStatus );
ANSC_STATUS CosaDmlEthPortSetName( CHAR *ifname, CHAR *newIfname);
INT CosaDmlEthPortLinkStatusCallback( CHAR *ifname, CHAR* state );
void EthWanLinkUp_callback();
void EthWanLinkDown_callback();
ANSC_STATUS CosaDmlEthPortGetCopyOfGlobalInfoForGivenIfName(char* ifName, PCOSA_DML_ETH_PORT_GLOBAL_CONFIG pGlobalInfo );
ANSC_STATUS CosaDmlEthSetWanLinkStatusForWanManager(char *ifname, char *WanStatus);
ANSC_STATUS CosaDmlEthSetWanInterfaceNameForWanManager(char *ifname, char *WanIfName);
ANSC_STATUS CosaDmlEthCreateEthLink(char *l2ifName, char *Path);
ANSC_STATUS CosaDmlEthDeleteEthLink(char *ifName, char *Path);
ANSC_STATUS CosaDmlEthGetPhyStatusForWanManager( char *ifname, char *PhyStatus );
ANSC_STATUS CosaDmlEthSetPhyStatusForWanManager( char *ifname, char *PhyStatus );
ANSC_STATUS CosaDmlEthSetPhyPathForWanManager(char *ifname);
ANSC_STATUS CosDmlEthPortUpdateGlobalInfo(PANSC_HANDLE phContext, CHAR *ifname, COSA_DML_ETH_TABLE_OPER Oper );
ANSC_STATUS EthMgr_AddPortToLanBridge (PCOSA_DML_ETH_PORT_CONFIG pEthLink, BOOLEAN AddToBridge);
#ifdef _SR300_PRODUCT_REQ_
void EthWanLinkUp_callback();
void EthWanLinkDown_callback();
#endif //_SR300_PRODUCT_REQ_
#endif
void Ethernet_Hosts_Sync(void);
INT CosaDmlEth_AssociatedDevice_callback(eth_device_t *eth_dev);
int ethGetPHYRate
    (
        CCSP_HAL_ETHSW_PORT PortId
    );
int get_sysevent_parameter(char *output);
void copy_command_output(FILE *fp, char * buf, int len);
bool isEthwan_initialized();
#endif
