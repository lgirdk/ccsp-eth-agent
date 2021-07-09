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

    module: cosa_ethernet_apis_mips.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  CosaDmlEthInterfaceInit
        *  CosaDmlEthPortGetNumberOfEntries
        *  CosaDmlEthPortGetEntry
        *  CosaDmlEthPortSetCfg
        *  CosaDmlEthPortGetCfg
        *  CosaDmlEthPortGetDinfo
        *  CosaDmlEthPortGetStats
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

#include "cosa_ethernet_apis.h"

#ifdef _COSA_SIM_
/*Removed code for simulator, because this is usg platform*/
#elif  (_COSA_BCM_MIPS_ || _COSA_DRG_TPG_)

#include "cosa_ethernet_apis_ext_mips.h"
#include "ansc_string_util.h"

#include "ccsp_psm_helper.h"

#include "utctx/utctx_api.h"
#include "linux/if.h"
#include "linux/sockios.h"
#include <sys/ioctl.h>
#include "secure_wrapper.h"

static int saveID(char* ifName, char* pAlias, ULONG ulInstanceNumber);
static int loadID(char* ifName, char* pAlias, ULONG* ulInstanceNumber);
COSA_DML_IF_STATUS getIfStatus(const PUCHAR name, struct ifreq *pIfr);
static int setIfStatus(struct ifreq *pIfr);
int _getMac(char* ifName, char* mac);

/**************************************************************************
                        DATA STRUCTURE DEFINITIONS
**************************************************************************/

/**************************************************************************
                        GLOBAL VARIABLES
**************************************************************************/

#ifdef _COSA_BCM_MIPS_

#include "syscfg/syscfg.h"
#include "ccsp_hal_ethsw.h"

int puma6_getSwitchCfg(PCosaEthInterfaceInfo eth, PCOSA_DML_ETH_PORT_CFG pcfg);
int puma6_setSwitchCfg(PCosaEthInterfaceInfo eth, PCOSA_DML_ETH_PORT_CFG pcfg);
int puma6_getSwitchDInfo(PCosaEthInterfaceInfo eth, PCOSA_DML_ETH_PORT_DINFO pDinfo);
int puma6_getSwitchStats(PCosaEthInterfaceInfo eth, PCOSA_DML_ETH_STATS pStats);

COSA_DML_ETH_PORT_SINFO      g_EthIntSInfo[] =
    {
        /* Downstream (LAN) ports */
        {SWITCH_PORT_0_NAME,                FALSE,  {0,0,0,0,0,0}},
        {SWITCH_PORT_1_NAME,                FALSE,  {0,0,0,0,0,0}},
        {SWITCH_PORT_2_NAME,                FALSE,  {0,0,0,0,0,0}},
        {SWITCH_PORT_3_NAME,                FALSE,  {0,0,0,0,0,0}},

        /* Upstream (WAN) ports */
        {DMSB_ETH_IF_NAME_DFT_WanRouting,   TRUE,   {0,0,0,0,0,0}},
        {DMSB_ETH_IF_NAME_DFT_WanBridging,  TRUE,   {0,0,0,0,0,0}}
    };

static const ULONG g_EthernetIntNum = sizeof(g_EthIntSInfo)/sizeof(g_EthIntSInfo[0]);

EthIntControlFuncs ifFuncs = {
    getIfCfg,
    setIfCfg,
    getIfStats,
    getIfDInfo
};

EthIntControlFuncs swFuncs = {
    puma6_getSwitchCfg,
    puma6_setSwitchCfg,
    puma6_getSwitchStats,
    puma6_getSwitchDInfo
};

int g_PortIDs[]={
    CCSP_HAL_ETHSW_EthPort1,
    CCSP_HAL_ETHSW_EthPort2,
    CCSP_HAL_ETHSW_EthPort3,
    CCSP_HAL_ETHSW_EthPort4
};

CosaEthInterfaceInfo g_EthEntries[] =
    {
        {g_EthIntSInfo + 0, {'\0'}, 0, 0, &swFuncs, g_PortIDs + 0, {0}},
        {g_EthIntSInfo + 1, {'\0'}, 0, 0, &swFuncs, g_PortIDs + 1, {0}},
        {g_EthIntSInfo + 2, {'\0'}, 0, 0, &swFuncs, g_PortIDs + 2, {0}},
        {g_EthIntSInfo + 3, {'\0'}, 0, 0, &swFuncs, g_PortIDs + 3, {0}},

        {g_EthIntSInfo + 4, {'\0'}, 0, 0, &ifFuncs, NULL,          {0}},
        {g_EthIntSInfo + 5, {'\0'}, 0, 0, &ifFuncs, NULL,          {0}}
    };

#endif

static int ethGetClientsCount
    (
        ULONG PortId,
        LONG num_eth_device,
        eth_device_t *eth_device
        )
{
    int idx;
    int count_client = 0;

    if (!eth_device)
    {
        CcspTraceWarning(("ethGetClientsCount Invalid input Param\n"));
        return 0;
    }

    for (idx = 0; idx < num_eth_device; idx++)
    {
        if (PortId == (ULONG)eth_device[idx].eth_port)
        {
            count_client++;
        }
    }

    return count_client;
}

static void ethGetClientMacDetails
    (
        LONG PortId,
        LONG client_num,
        LONG num_eth_device,
        eth_device_t *eth_device,
        char *mac
    )
{
    int idx;
    char mac_addr[20];
    int isClient = 0;

    if (!eth_device || !mac)
    {
        CcspTraceWarning(("ethGetClientMacDetails Invalid input Param\n"));
        return;
    }

    for (idx = 0; idx < num_eth_device; idx++)
    {
        if (PortId == eth_device[idx].eth_port)
        {
            isClient++;
            if (isClient == client_num) {
                _ansc_memset(mac_addr, 0, 20);
                sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
                        eth_device[idx].eth_devMacAddress[0],
                        eth_device[idx].eth_devMacAddress[1],
                        eth_device[idx].eth_devMacAddress[2],
                        eth_device[idx].eth_devMacAddress[3],
                        eth_device[idx].eth_devMacAddress[4],
                        eth_device[idx].eth_devMacAddress[5]);
                strcpy(mac, mac_addr);
                return;
            }
        }
    }
}

ANSC_STATUS
CosaDmlEthPortGetClientMac
    (
        PCOSA_DML_ETH_PORT_FULL pEthernetPortFull,
        ULONG                   ulInstanceNumber
    )
{
    int ret = ANSC_STATUS_FAILURE;

    ULONG total_eth_device = 0;
    eth_device_t *output_struct = NULL;

    ret = CcspHalExtSw_getAssociatedDevice(&total_eth_device, &output_struct);
    if (ANSC_STATUS_SUCCESS != ret)
    {
        CcspTraceError(("%s CcspHalExtSw_getAssociatedDevice failed\n", __func__));
        return ret;
    }

        if ( total_eth_device )
        {
           ULONG i = 1;
           ULONG ulNumClients = 0;

           //Get the no of clients associated with port
           ulNumClients = ethGetClientsCount(ulInstanceNumber, total_eth_device, output_struct);
           pEthernetPortFull->DynamicInfo.AssocDevicesCount = 0;

           if( ulNumClients  > 0 )
           {
               pEthernetPortFull->DynamicInfo.AssocDevicesCount = ulNumClients;

               //Get Mac for associated clients
               for ( i = 1; i <= ulNumClients; i++ )
               {
                    ethGetClientMacDetails(
                                   ulInstanceNumber,
                                   i,
                                   total_eth_device,
                                   output_struct,
                                   (char*)pEthernetPortFull->AssocClient[i - 1].MacAddress);
               }
           }

           //Release the allocated memory by HAL
           if( NULL != output_struct )
           {
              free(output_struct);
              output_struct = NULL;
           }
        }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************
                            Routine Trunks
**********************************************************************/

ANSC_STATUS
CosaDmlEthInterfaceInit
    (
        ANSC_HANDLE                 hDml,
        PANSC_HANDLE                phContext
    )
{

    UNREFERENCED_PARAMETER(hDml);
    UNREFERENCED_PARAMETER(phContext);
#ifdef _COSA_BCM_MIPS_
    CHAR strMac[128]       = {0};
    ULONG i                 = 0;

    syscfg_init();

    /*
     *  Manufacturer programmed MAC address for LAN interface should be read out here. -- DH
     *
     *  It doesn't make sense to even have a MAC address in Ethernet Interface DM object,
     *  so we are not going to fill the MAC address for Upstream interfaces.
     */
    if (( -1 != _getMac("brlan0", strMac)))
    {
        AnscCopyMemory(g_EthIntSInfo[0].MacAddress, strMac, 6);
        AnscCopyMemory(g_EthIntSInfo[1].MacAddress, strMac, 6);
        AnscCopyMemory(g_EthIntSInfo[2].MacAddress, strMac, 6);
        AnscCopyMemory(g_EthIntSInfo[3].MacAddress, strMac, 6);
    }

    if (( -1 != _getMac("erouter0", strMac)))
    {
        AnscCopyMemory(g_EthIntSInfo[4].MacAddress, strMac, 6);
    }

    if (( -1 != _getMac("lbr0", strMac)))
    {
        AnscCopyMemory(g_EthIntSInfo[5].MacAddress, strMac, 6);
    }

#endif

    for (i=0; i < g_EthernetIntNum; ++i) {
        loadID(g_EthEntries[i].sInfo->Name, g_EthEntries[i].Alias, &g_EthEntries[i].instanceNumber);
        g_EthEntries[i].LastChange = AnscGetTickInSeconds();
    }

    return ANSC_STATUS_SUCCESS;
}

ULONG
CosaDmlEthPortGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    )
{
    UNREFERENCED_PARAMETER(hContext);
    return g_EthernetIntNum;
}

ANSC_STATUS
CosaDmlEthPortGetEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        PCOSA_DML_ETH_PORT_FULL     pEntry
    )
{
    UNREFERENCED_PARAMETER(hContext);
    if (pEntry)
    {
        _ansc_memset(pEntry, 0, sizeof(COSA_DML_ETH_PORT_FULL));
    }
    else
    {
        return ANSC_STATUS_FAILURE;
    }

#ifdef _COSA_BCM_MIPS_
    /* CID: 53756 Unsigned compared against 0 ulIndex >= 0 always true*/
    if (ulIndex < g_EthernetIntNum)
    {
        g_EthEntries[ulIndex].control->getCfg(g_EthEntries + ulIndex, &pEntry->Cfg);
        AnscCopyMemory(&pEntry->StaticInfo, &g_EthIntSInfo[ulIndex], sizeof(COSA_DML_ETH_PORT_SINFO));
        g_EthEntries[ulIndex].control->getDInfo(g_EthEntries + ulIndex, &pEntry->DynamicInfo);
    }
    else
    {
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
#endif
}

ANSC_STATUS
CosaDmlEthPortSetValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    )
{
    UNREFERENCED_PARAMETER(hContext);
    g_EthEntries[ulIndex].instanceNumber=ulInstanceNumber;
    AnscCopyString(g_EthEntries[ulIndex].Alias, pAlias);
    saveID(g_EthIntSInfo[ulIndex].Name, pAlias, ulInstanceNumber);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlEthPortSetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_ETH_PORT_CFG      pCfg
    )
{
    UNREFERENCED_PARAMETER(hContext);
    COSA_DML_ETH_PORT_CFG origCfg;
    PCosaEthInterfaceInfo pEthIf = (PCosaEthInterfaceInfo  )NULL;

    /*RDKB-6838, CID-32984, null check before use*/
    if ( !pCfg )
    {
        return ANSC_STATUS_FAILURE;
    }

    pEthIf = getIF(pCfg->InstanceNumber);

    if ( !pEthIf )
    {
        return ANSC_STATUS_FAILURE;
    }


    pEthIf->control->getCfg(pEthIf, &origCfg);

    pEthIf->control->setCfg(pEthIf, pCfg);

    if ( origCfg.bEnabled != pCfg->bEnabled )
    {
        pEthIf->control->getStats(pEthIf, &pEthIf->LastStats);

        pEthIf->LastChange = AnscGetTickInSeconds();
    }

    if ( !AnscEqualString(pCfg->Alias, pEthIf->Alias, TRUE) )
    {
        AnscCopyString(pEthIf->Alias, pCfg->Alias);
        saveID(pEthIf->sInfo->Name, pCfg->Alias, pCfg->InstanceNumber);
    }

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlEthPortGetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_ETH_PORT_CFG      pCfg
    )
{
    UNREFERENCED_PARAMETER(hContext);
    PCosaEthInterfaceInfo pEthIf = (PCosaEthInterfaceInfo  )NULL;

    /*RDKB-6838, CID-33167, null check before use*/
    if ( !pCfg )
    {
        return ANSC_STATUS_FAILURE;
    }

    pEthIf = getIF(pCfg->InstanceNumber);

    if ( !pEthIf )
    {
        return ANSC_STATUS_FAILURE;
    }


    pEthIf->control->getCfg(pEthIf, pCfg);

    AnscCopyString(pCfg->Alias, pEthIf->Alias);

    pCfg->InstanceNumber = pEthIf->instanceNumber;

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlEthPortGetDinfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_ETH_PORT_DINFO    pInfo
    )
{
    UNREFERENCED_PARAMETER(hContext);
    if (!pInfo)
    {
        return ANSC_STATUS_FAILURE;
    }

    PCosaEthInterfaceInfo pEthIf = getIF(ulInstanceNumber);

    if ( !pEthIf )
    {
        return ANSC_STATUS_FAILURE;
    }

    pEthIf->control->getDInfo(pEthIf, pInfo);

    pInfo->LastChange = pEthIf->LastChange;

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlEthPortGetStats
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_ETH_STATS         pStats
    )
{
    UNREFERENCED_PARAMETER(hContext);
    if (!pStats)
    {
        return ANSC_STATUS_FAILURE;
    }

    _ansc_memset(pStats, 0, sizeof(COSA_DML_ETH_STATS));

    PCosaEthInterfaceInfo pEthIf = getIF(ulInstanceNumber);

    if ( !pEthIf )
    {
        return ANSC_STATUS_FAILURE;
    }

    pEthIf->control->getStats(pEthIf, pStats);

    pStats->BroadcastPacketsReceived    -= pEthIf->LastStats.BroadcastPacketsReceived;
    pStats->BroadcastPacketsSent        -= pEthIf->LastStats.BroadcastPacketsSent;
    pStats->BytesReceived               -= pEthIf->LastStats.BytesReceived;
    pStats->BytesSent                   -= pEthIf->LastStats.BytesSent;
    pStats->DiscardPacketsReceived      -= pEthIf->LastStats.DiscardPacketsReceived;
    pStats->DiscardPacketsSent          -= pEthIf->LastStats.DiscardPacketsSent;
    pStats->ErrorsReceived              -= pEthIf->LastStats.ErrorsReceived;
    pStats->ErrorsSent                  -= pEthIf->LastStats.ErrorsSent;
    pStats->MulticastPacketsReceived    -= pEthIf->LastStats.MulticastPacketsReceived;
    pStats->MulticastPacketsSent        -= pEthIf->LastStats.MulticastPacketsSent;
    pStats->PacketsReceived             -= pEthIf->LastStats.PacketsReceived;
    pStats->PacketsSent                 -= pEthIf->LastStats.PacketsSent;
    pStats->UnicastPacketsReceived      -= pEthIf->LastStats.UnicastPacketsReceived;
    pStats->UnicastPacketsSent          -= pEthIf->LastStats.UnicastPacketsSent;
    pStats->UnknownProtoPacketsReceived -= pEthIf->LastStats.UnknownProtoPacketsReceived;

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************
                        HELPER ROUTINES
**********************************************************************/


#ifdef _COSA_BCM_MIPS_
int puma6_getSwitchCfg(PCosaEthInterfaceInfo eth, PCOSA_DML_ETH_PORT_CFG pcfg)
{
    CCSP_HAL_ETHSW_PORT         port        = *((PCCSP_HAL_ETHSW_PORT)eth->hwid);
    INT                         status;
    CCSP_HAL_ETHSW_ADMIN_STATUS AdminStatus;
    CCSP_HAL_ETHSW_LINK_RATE    LinkRate;
    CCSP_HAL_ETHSW_DUPLEX_MODE  DuplexMode;

    /* By default, port is enabled */
    pcfg->bEnabled = TRUE;

    status = CcspHalEthSwGetPortAdminStatus(port, &AdminStatus);

    if ( status == RETURN_OK )
    {
        switch ( AdminStatus )
        {
            case CCSP_HAL_ETHSW_AdminUp:
            {
                pcfg->bEnabled = TRUE;
                break;
            }
            case CCSP_HAL_ETHSW_AdminDown:
            {
                pcfg->bEnabled = FALSE;
                break;
            }
            default:
            {
                pcfg->bEnabled = TRUE;
                break;
            }
        }
    }

    status = CcspHalEthSwGetPortCfg(port, &LinkRate, &DuplexMode);

    if ( status == RETURN_OK )
    {
        switch ( LinkRate )
        {
            case CCSP_HAL_ETHSW_LINK_10Mbps:
            {
                pcfg->MaxBitRate = 10;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_100Mbps:
            {
                pcfg->MaxBitRate = 100;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_1Gbps:
            {
                pcfg->MaxBitRate = 1000;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_10Gbps:
            {
                pcfg->MaxBitRate = 10000;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_Auto:
            {
                pcfg->MaxBitRate = 0;
                break;
            }
            default:
            {
                pcfg->MaxBitRate = 0;
                break;
            }
        }

        switch ( DuplexMode )
        {
            case CCSP_HAL_ETHSW_DUPLEX_Auto:
            {
                pcfg->DuplexMode = COSA_DML_ETH_DUPLEX_Auto;
                break;
            }
            case CCSP_HAL_ETHSW_DUPLEX_Half:
            {
                pcfg->DuplexMode = COSA_DML_ETH_DUPLEX_Half;
                break;
            }
            case CCSP_HAL_ETHSW_DUPLEX_Full:
            {
                pcfg->DuplexMode = COSA_DML_ETH_DUPLEX_Full;
                break;
            }
            default:
            {
                pcfg->DuplexMode = COSA_DML_ETH_DUPLEX_Auto;
                break;
            }
        }
    }
    else
    {
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;

}
int puma6_setSwitchCfg(PCosaEthInterfaceInfo eth, PCOSA_DML_ETH_PORT_CFG pcfg) {
    CCSP_HAL_ETHSW_PORT         port        = *((PCCSP_HAL_ETHSW_PORT)eth->hwid);
    CCSP_HAL_ETHSW_ADMIN_STATUS AdminStatus;
    if(pcfg->bEnabled == TRUE)
    {
        AdminStatus = CCSP_HAL_ETHSW_AdminUp;
    }
    else
    {
        AdminStatus = CCSP_HAL_ETHSW_AdminDown;
    }
    CcspHalEthSwSetPortAdminStatus(port,AdminStatus);
    return ANSC_STATUS_SUCCESS;
}
int puma6_getSwitchDInfo(PCosaEthInterfaceInfo eth, PCOSA_DML_ETH_PORT_DINFO pDinfo){
    CCSP_HAL_ETHSW_PORT         port        = *((PCCSP_HAL_ETHSW_PORT)eth->hwid);
    INT                         status;
    CCSP_HAL_ETHSW_LINK_RATE    LinkRate;
    CCSP_HAL_ETHSW_DUPLEX_MODE  DuplexMode;
    CCSP_HAL_ETHSW_LINK_STATUS  LinkStatus;

    pDinfo->Status         = COSA_DML_IF_STATUS_Down;
    pDinfo->CurrentBitRate = 0;

    status = CcspHalEthSwGetPortStatus(port, &LinkRate, &DuplexMode, &LinkStatus);

    if ( status == RETURN_OK )
    {
        switch ( LinkStatus )
        {
            case CCSP_HAL_ETHSW_LINK_Up:
            {
                pDinfo->Status = COSA_DML_IF_STATUS_Up;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_Down:
            {
                pDinfo->Status = COSA_DML_IF_STATUS_Down;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_Disconnected:
            {
                pDinfo->Status = COSA_DML_IF_STATUS_Down;
                break;
            }
            default:
            {
                pDinfo->Status = COSA_DML_IF_STATUS_Down;
                break;
            }
        }

        switch ( LinkRate )
        {
            case CCSP_HAL_ETHSW_LINK_10Mbps:
            {
                pDinfo->CurrentBitRate = 10;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_100Mbps:
            {
                pDinfo->CurrentBitRate = 100;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_1Gbps:
            {
                pDinfo->CurrentBitRate = 1000;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_10Gbps:
            {
                pDinfo->CurrentBitRate = 10000;
                break;
            }
            case CCSP_HAL_ETHSW_LINK_Auto:
            {
                pDinfo->CurrentBitRate = 0;
                break;
            }
            default:
            {
                pDinfo->CurrentBitRate = 0;
                break;
            }
        }
    }
    else
    {
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}

int puma6_getSwitchStats(PCosaEthInterfaceInfo eth, PCOSA_DML_ETH_STATS pStats){
    UNREFERENCED_PARAMETER(eth);
    UNREFERENCED_PARAMETER(pStats);
    return ANSC_STATUS_SUCCESS;
}

static int getIfCfg(PCosaEthInterfaceInfo pEthIf, PCOSA_DML_ETH_PORT_CFG pCfg)
{
    if ( getIfStatus((PUCHAR)pEthIf->sInfo->Name, NULL ) == COSA_DML_IF_STATUS_Up )
    {
        pCfg->bEnabled = TRUE;
    }
    else
    {
        pCfg->bEnabled = FALSE;
    }

    pCfg->DuplexMode = COSA_DML_ETH_DUPLEX_Auto;
    pCfg->MaxBitRate = 1000;

    return 0;
}

static int setIfCfg(PCosaEthInterfaceInfo pEthIf, PCOSA_DML_ETH_PORT_CFG pCfg)
{
    struct ifreq ifr;
    COSA_DML_IF_STATUS enifStatus = COSA_DML_IF_STATUS_Unknown;

    enifStatus = getIfStatus((PUCHAR)pEthIf->sInfo->Name, &ifr);

    if ( ( enifStatus == COSA_DML_IF_STATUS_Unknown ) || \
         ( enifStatus == COSA_DML_IF_STATUS_NotPresent )
        )
    {
        return ANSC_STATUS_FAILURE;
    }

    if ( pCfg->bEnabled && !(ifr.ifr_flags & IFF_UP) )
    {
        ifr.ifr_flags |= IFF_UP;

        if ( setIfStatus(&ifr) )
        {
            return -1;
        }

        /*
         *  Do not trigger a respective wan-restart or multinet events for now
         *      pEthIf->sInfo->bUpstream == TRUE -> sysevent set wan-restart
         */
    }
    else if ( !(pCfg->bEnabled) && (ifr.ifr_flags & IFF_UP) )
    {
        ifr.ifr_flags &= ~IFF_UP;

        if ( setIfStatus(&ifr) )
        {
            return -1;
        }
    }

    return 0;
}

static int getIfStats(PCosaEthInterfaceInfo pEthIf, PCOSA_DML_ETH_STATS pStats)
{
    return getIfStats2((PUCHAR)pEthIf->sInfo->Name, pStats);
}

static int getIfDInfo(PCosaEthInterfaceInfo pEthIf, PCOSA_DML_ETH_PORT_DINFO pInfo)
{
    pInfo->Status = getIfStatus((PUCHAR)pEthIf->sInfo->Name, NULL);

    return 0;
}
#endif

static int getIfStats2(const PUCHAR pName, PCOSA_DML_ETH_STATS pStats)
{
#if 1
    FILE *fp;
    char buf[512];
    char *tok, *delim = ": \t\r\n", *sp, *ptr;
    int idx;

    if ((fp = fopen("/proc/net/dev", "rb")) == NULL)
        return -1;

    /* skip head line */
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return -1;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, (char *)pName) == NULL)
            continue;

        for (idx = 1, ptr = buf;
                (tok = strtok_r(ptr, delim, &sp)) != NULL; idx++) {
            ptr = NULL;
            switch (idx) {
            case 2:
                pStats->BytesReceived = (ULONG)atol(tok);
                break;
            case 3:
                pStats->PacketsReceived = (ULONG)atol(tok);
                break;
            case 4:
                pStats->ErrorsReceived = (ULONG)atol(tok);
                break;
            case 5:
                pStats->DiscardPacketsReceived = (ULONG)atol(tok);
                break;
            case 10:
                pStats->BytesSent = (ULONG)atol(tok);
                break;
            case 11:
                pStats->PacketsSent = (ULONG)atol(tok);
                break;
            case 12:
                pStats->ErrorsSent = (ULONG)atol(tok);
                break;
            case 13:
                pStats->DiscardPacketsSent = (ULONG)atol(tok);
                break;
            default:
                break;
            }
        }
    }

    fclose(fp);
    return 0;

#else
    UCHAR strCmd[512] = {0};
    UCHAR strBuf[128] = {0};
    FILE  *pFile      = NULL;

    _ansc_sprintf
        (
            strCmd,
            "cat /proc/net/dev | grep %s | tr : \" \" | awk '{print $2}'",
            pName
        );
    pFile = popen(strCmd, "r");
    fread(strBuf, sizeof(char), sizeof(strBuf), pFile);
    pStats->BytesReceived = _ansc_atoi(strBuf);
    pclose(pFile);

    _ansc_sprintf
        (
            strCmd,
            "cat /proc/net/dev | grep %s | tr : \" \" | awk '{print $3}'",
            pName
        );
    pFile = popen(strCmd, "r");
    fread(strBuf, sizeof(char), sizeof(strBuf), pFile);
    pStats->PacketsReceived= _ansc_atoi(strBuf);
    pclose(pFile);

    _ansc_sprintf
        (
            strCmd,
            "cat /proc/net/dev | grep %s | tr : \" \" | awk '{print $4}'",
            pName
        );
    pFile = popen(strCmd, "r");
    fread(strBuf, sizeof(char), sizeof(strBuf), pFile);
    pStats->ErrorsReceived= _ansc_atoi(strBuf);
    pclose(pFile);

    _ansc_sprintf
        (
            strCmd,
            "cat /proc/net/dev | grep %s | tr : \" \" | awk '{print $5}'",
            pName
        );
    pFile = popen(strCmd, "r");
    fread(strBuf, sizeof(char), sizeof(strBuf), pFile);
    pStats->DiscardPacketsReceived = _ansc_atoi(strBuf);
    pclose(pFile);

    _ansc_sprintf
        (
            strCmd,
            "cat /proc/net/dev | grep %s | tr : \" \" | awk '{print $10}'",
            pName
        );
    pFile = popen(strCmd, "r");
    fread(strBuf, sizeof(char), sizeof(strBuf), pFile);
    pStats->BytesSent= _ansc_atoi(strBuf);
    pclose(pFile);

    _ansc_sprintf
        (
            strCmd,
            "cat /proc/net/dev | grep %s | tr : \" \" | awk '{print $11}'",
            pName
        );
    pFile = popen(strCmd, "r");
    fread(strBuf, sizeof(char), sizeof(strBuf), pFile);
    pStats->PacketsSent = _ansc_atoi(strBuf);
    pclose(pFile);

    _ansc_sprintf
        (
            strCmd,
            "cat /proc/net/dev | grep %s | tr : \" \" | awk '{print $12}'",
            pName
        );
    pFile = popen(strCmd, "r");
    fread(strBuf, sizeof(char), sizeof(strBuf), pFile);
    pStats->ErrorsSent= _ansc_atoi(strBuf);
    pclose(pFile);

    _ansc_sprintf
        (
            strCmd,
            "cat /proc/net/dev | grep %s | tr : \" \" | awk '{print $13}'",
            pName
        );
    pFile = popen(strCmd, "r");
    fread(strBuf, sizeof(char), sizeof(strBuf), pFile);
    pStats->DiscardPacketsSent= _ansc_atoi(strBuf);
    pclose(pFile);

    return 0;
#endif
}

static int setIfStatus(struct ifreq *pIfr)
{
    int skfd;

    AnscTraceFlow(("%s...\n", __FUNCTION__));

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    /* CID: 73861 Argument cannot be negative*/
    if(skfd == -1)
       return -1;

    if (ioctl(skfd, SIOCSIFFLAGS, pIfr) < 0) {
        CcspTraceWarning(("cosa_ethernet_apis.c - setIfStatus: Set interface %s error...\n", pIfr->ifr_name));
        close(skfd);
        return -1;
    }
    close(skfd);

    return 0;
}

PCosaEthInterfaceInfo getIF(const ULONG instanceNumber) {
    ULONG i;
    for (i = 0; i < g_EthernetIntNum; ++i) {
        if (g_EthEntries[i].instanceNumber == instanceNumber) {
            break;
        }
    }

    if (i == g_EthernetIntNum) {
        return NULL;
    }
    return g_EthEntries + i;
}

static int saveID(char* ifName, char* pAlias, ULONG ulInstanceNumber) {
    UtopiaContext utctx;
    char idStr[COSA_DML_IF_NAME_LENGTH+10] = {0};
    Utopia_Init(&utctx);

    sprintf(idStr,"%s,%lu", pAlias,ulInstanceNumber);
    Utopia_RawSet(&utctx,COSA_ETH_INT_ID_SYSCFG_NAMESPACE,ifName,idStr);

    Utopia_Free(&utctx,TRUE);

    return 0;
}

static int loadID(char* ifName, char* pAlias, ULONG* ulInstanceNumber) {
    UtopiaContext utctx;
    char idStr[COSA_DML_IF_NAME_LENGTH+10] = {0};
    char* instNumString;
    int rv;
    Utopia_Init(&utctx);

    rv =Utopia_RawGet(&utctx, COSA_ETH_INT_ID_SYSCFG_NAMESPACE, ifName, idStr, sizeof(idStr));
    if (rv == -1 || idStr[0] == '\0') {
        Utopia_Free(&utctx, 0);
        return -1;
    }
    instNumString=idStr + AnscSizeOfToken(idStr, ",", sizeof(idStr))+1;
    *(instNumString-1)='\0';

    AnscCopyString(pAlias, idStr);
    *ulInstanceNumber = AnscGetStringUlong(instNumString);
    Utopia_Free(&utctx, 0);

    return 0;
}

#endif

