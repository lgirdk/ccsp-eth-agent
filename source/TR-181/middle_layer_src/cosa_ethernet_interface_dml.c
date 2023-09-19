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

#include "cosa_ethernet_interface_dml.h"
#include "cosa_ethernet_apis.h"
#include "cosa_ethernet_internal.h"

#ifdef _HUB4_PRODUCT_REQ_
#include <sys/sysinfo.h>
#define WAN_INTERFACE_LEN 8
#define BUFLEN_16 16
#endif
#if defined(MULTILAN_FEATURE) && defined (INTEL_PUMA7)
#if defined (ETH_4_PORTS) && defined (ENABLE_ETH_WAN)
#include <syscfg/syscfg.h>
#define BUFLEN_6 6
#endif
#endif
#include "safec_lib_common.h"


/***********************************************************************

 APIs for Object:

    Ethernet.Interface.{i}.

    *  Interface_GetEntryCount
    *  Interface_GetEntry
    *  Interface_GetParamBoolValue
    *  Interface_GetParamIntValue
    *  Interface_GetParamUlongValue
    *  Interface_GetParamStringValue
    *  Interface_SetParamBoolValue
    *  Interface_SetParamIntValue
    *  Interface_SetParamUlongValue
    *  Interface_SetParamStringValue
    *  Interface_Validate
    *  Interface_Commit
    *  Interface_Rollback

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        Interface_GetEntryCount
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
Interface_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    return CosaDmlEthPortGetNumberOfEntries(NULL);
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        Interface_GetEntry
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
Interface_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DATAMODEL_ETHERNET        pMyObject     = (PCOSA_DATAMODEL_ETHERNET)g_pCosaBEManager->hEthernet;

    *pInsNumber = pMyObject->EthernetPortFullTable[nIndex].Cfg.InstanceNumber;

    return &pMyObject->EthernetPortFullTable[nIndex]; /* return the handle */
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Interface_GetParamBoolValue
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
Interface_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* collect value */
         CosaDmlEthPortGetCfg(NULL, &pEthernetPortFull->Cfg);
        *pBool = pEthernetPortFull->Cfg.bEnabled;
        return TRUE;
    }

    if (strcmp(ParamName, "Upstream") == 0)
    {
        /* collect value */
        *pBool = pEthernetPortFull->StaticInfo.bUpstream;
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Interface_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Interface_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "MaxBitRate") == 0)
    {
        /* collect value */
        CosaDmlEthPortGetCfg(NULL, &pEthernetPortFull->Cfg);
        *pInt = pEthernetPortFull->Cfg.MaxBitRate;
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
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
Interface_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Status") == 0)
    {
        /* collect value */
        CosaDmlEthPortGetDinfo(NULL, pEthernetPortFull->Cfg.InstanceNumber, &pEthernetPortFull->DynamicInfo);

        *puLong = pEthernetPortFull->DynamicInfo.Status;
        return TRUE;
    }

    if (strcmp(ParamName, "LastChange") == 0)
    {
        /* collect value */
        CosaDmlEthPortGetDinfo(NULL, pEthernetPortFull->Cfg.InstanceNumber, &pEthernetPortFull->DynamicInfo);

        *puLong = AnscGetTimeIntervalInSeconds(pEthernetPortFull->DynamicInfo.LastChange, AnscGetTickInSeconds());
        return TRUE;
    }

    if (strcmp(ParamName, "DuplexMode") == 0)
    {
        /* collect value */
        *puLong = pEthernetPortFull->Cfg.DuplexMode;
        return TRUE;
    }

    if (strcmp(ParamName, "CurrentBitRate") == 0)
    {
        /* collect value */

        CosaDmlEthPortGetDinfo(NULL, pEthernetPortFull->Cfg.InstanceNumber, &pEthernetPortFull->DynamicInfo);

        *puLong = pEthernetPortFull->DynamicInfo.CurrentBitRate;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        Interface_GetParamStringValue
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
Interface_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    UNREFERENCED_PARAMETER(pUlSize);
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;
    char assocDeviceMacList[(17 * ETH_INTERFACE_MAX_ASSOC_DEVICES) + 1];
    errno_t rc = -1;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* collect value */
        AnscCopyString(pValue, pEthernetPortFull->Cfg.Alias);
        return 0;
    }

    if (strcmp(ParamName, "Name") == 0)
    {
        /* collect value */
        AnscCopyString(pValue, pEthernetPortFull->StaticInfo.Name);
        return 0;
    }

    if (strcmp(ParamName, "LowerLayers") == 0)
    {
        /* collect value */
        strcpy(pValue, "");
        return 0;
    }

    if (strcmp(ParamName, "MACAddress") == 0)
    {
        //when dut reboot, brlan0 start so late then sometimes g_EthIntSInfo still can't get brlan0's mac
#if CFG_TR181_ETH_BORROW_MAC
        if( (0x00 == pEthernetPortFull->StaticInfo.MacAddress[0]) && (0x00 == pEthernetPortFull->StaticInfo.MacAddress[1]) && (0x00 == pEthernetPortFull->StaticInfo.MacAddress[2]) && (0x00 == pEthernetPortFull->StaticInfo.MacAddress[3]) && (0x00 == pEthernetPortFull->StaticInfo.MacAddress[4]) && (0x00 == pEthernetPortFull->StaticInfo.MacAddress[5]))
        {
            CHAR strMac[128] = {0};
            if ( -1 != _getMac("brlan0", strMac) )
            {
                AnscCopyMemory(pEthernetPortFull->StaticInfo.MacAddress,strMac,6);
            }
        }
#endif
#if defined(MULTILAN_FEATURE) && defined (INTEL_PUMA7)

        CHAR strMac[128] = {0};
#ifdef FEATURE_RDKB_WAN_MANAGER
        char wanPhyName[32] = {0};
        char out_value[32] = {0};
        if (syscfg_get(NULL, "wan_physical_ifname", out_value, sizeof(out_value)) == 0)
        {
           strncpy(wanPhyName, out_value, sizeof(wanPhyName));
           CcspTraceInfo(("%s %d - WanPhyName=%s \n", __FUNCTION__,__LINE__, wanPhyName));
        }
        else
        {
           strncpy(wanPhyName, "erouter0", sizeof(wanPhyName));
           CcspTraceInfo(("%s %d - WanPhyName=%s \n", __FUNCTION__,__LINE__, wanPhyName));
        }
        if(strcmp(pEthernetPortFull->StaticInfo.Name, wanPhyName) == 0 ) {
                if ( -1 != _getMac(wanPhyName, strMac) )
#else
        if(strcmp(pEthernetPortFull->StaticInfo.Name, "erouter0") == 0 ) {
                if ( -1 != _getMac("erouter0", strMac) )
#endif
                {
                        AnscCopyMemory(pEthernetPortFull->StaticInfo.MacAddress,strMac,6);
                }
        }
#if defined (ETH_4_PORTS) && defined (ENABLE_ETH_WAN)
        if(strcmp(pEthernetPortFull->StaticInfo.Name, "sw_4") == 0) {
                char buf[BUFLEN_6]= {0};
                syscfg_get(NULL, "eth_wan_enabled", buf, sizeof(buf));
                if(strcmp(buf, "true") == 0) {
                        if ( -1 != _getMac("erouter0", strMac) )
                        {
                               AnscCopyMemory(pEthernetPortFull->StaticInfo.MacAddress,strMac,6);
                        }
                }
        }
#endif
#endif
        rc = sprintf_s
            (
                pValue,
                *pUlSize,
                "%02x:%02x:%02x:%02x:%02x:%02x",
                pEthernetPortFull->StaticInfo.MacAddress[0],
                pEthernetPortFull->StaticInfo.MacAddress[1],
                pEthernetPortFull->StaticInfo.MacAddress[2],
                pEthernetPortFull->StaticInfo.MacAddress[3],
                pEthernetPortFull->StaticInfo.MacAddress[4],
                pEthernetPortFull->StaticInfo.MacAddress[5]
            );
        if(rc < EOK)
        {
          ERR_CHK(rc);
          return -1;
        }
        return 0;
    }

    if (strcmp(ParamName, "X_CISCO_COM_AssociatedDevice") == 0)
    {
        CosaDmlEthPortGetDinfo(NULL, pEthernetPortFull->Cfg.InstanceNumber, &pEthernetPortFull->DynamicInfo);
        CosaEthPortGetAssocDevices(pEthernetPortFull->DynamicInfo.AssocDevices,assocDeviceMacList,pEthernetPortFull->DynamicInfo.AssocDevicesCount);

        if(AnscSizeOfString(assocDeviceMacList) < *pUlSize)
        {
            AnscCopyString(pValue,assocDeviceMacList);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(assocDeviceMacList);
            return 1;
        }
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Interface_SetParamBoolValue
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
Interface_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* save update to backup */
        pEthernetPortFull->Cfg.bEnabled = bValue;
        return TRUE;
    }


    if (strcmp(ParamName, "Upstream") == 0)
    {
#ifdef FEATURE_RDKB_WAN_UPSTREAM
        pEthernetPortFull->StaticInfo.bUpstream = bValue;
        EthInterfaceSetUpstream( pEthernetPortFull );
        return TRUE;
#elif defined(AUTOWAN_ENABLE) && defined(WAN_MANAGER_UNIFICATION_ENABLED)
        if (ANSC_STATUS_SUCCESS == EthwanEnableWithoutReboot(bValue))
            return TRUE;
#endif  // FEATURE_RDKB_WAN_UPSTREAM
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Interface_SetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int                         iValue
            );

    description:

        This function is called to set integer parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int                         iValue
                The updated integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Interface_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "MaxBitRate") == 0)
    {
        /* save update to backup */
        pEthernetPortFull->Cfg.MaxBitRate = iValue;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Interface_SetParamUlongValue
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
Interface_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "DuplexMode") == 0)
    {
        /* save update to backup */
        pEthernetPortFull->Cfg.DuplexMode = uValue;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Interface_SetParamStringValue
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
Interface_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* save update to backup */
	/* CID 281791 Copy into fixed size buffer fix */
	if(strlen(pString) < (sizeof(pEthernetPortFull->Cfg.Alias)-1))
	{
	    AnscCopyString(pEthernetPortFull->Cfg.Alias, pString);
	    return TRUE;
	}
    }

    if (strcmp(ParamName, "LowerLayers") == 0)
    {
        /* save update to backup */
        /* Interface is a layer 1 interface. LowerLayers will not be used. */
        return FALSE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Interface_Validate
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
Interface_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    PCOSA_DATAMODEL_ETHERNET        pMyObject               = (PCOSA_DATAMODEL_ETHERNET)g_pCosaBEManager->hEthernet;
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull       = (PCOSA_DML_ETH_PORT_FULL )hInsContext;
    ULONG                           ulEntryCount            = 0;
    ULONG                           ulIndex                 = 0;

    ulEntryCount = CosaDmlEthPortGetNumberOfEntries(NULL);

    for ( ulIndex = 0; ulIndex < ulEntryCount; ulIndex++ )
    {
        if (
                ((ULONG)pEthernetPortFull != (ULONG)&pMyObject->EthernetPortFullTable[ulIndex]) &&
                (strcmp(pEthernetPortFull->Cfg.Alias, pMyObject->EthernetPortFullTable[ulIndex].Cfg.Alias) == 0)
            )
        {
            AnscCopyString(pReturnParamName, "Alias");

            *puLength = AnscSizeOfString("Alias");

            return FALSE;
        }
    }

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
Interface_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    CosaDmlEthPortSetCfg(NULL, &pEthernetPortFull->Cfg);

    return 0;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        Interface_Rollback
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
Interface_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;

    CosaDmlEthPortGetCfg(NULL, &pEthernetPortFull->Cfg);

    return 0;
}

/***********************************************************************

 APIs for Object:

    Ethernet.Interface.{i}.X_RDKCENTRAL-COM_AssociatedDevice.{i}.

    *  AssociatedDevice1_GetEntryCount
    *  AssociatedDevice1_GetEntry
    *  AssociatedDevice1_IsUpdated
    *  AssociatedDevice1_Synchronize
    *  AssociatedDevice1_GetParamStringValue
***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        AssociatedDevice_GetEntryCount
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
AssociatedDevice1_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_ETHERNET        pMyObject     = (PCOSA_DATAMODEL_ETHERNET)g_pCosaBEManager->hEthernet;
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;
    ULONG                           InterfaceIndex  = pEthernetPortFull->Cfg.InstanceNumber;

    return pMyObject->EthernetPortFullTable[InterfaceIndex - 1].DynamicInfo.AssocDevicesCount;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        AssociatedDevice1_GetEntry
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
AssociatedDevice1_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PCOSA_DATAMODEL_ETHERNET        pMyObject     = (PCOSA_DATAMODEL_ETHERNET)g_pCosaBEManager->hEthernet;
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;
    ULONG                           InterfaceIndex  = pEthernetPortFull->Cfg.InstanceNumber;

    *pInsNumber = nIndex + 1;

    return &pMyObject->EthernetPortFullTable[InterfaceIndex - 1].AssocClient[nIndex]; /* return the handle */
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        AssociatedDevice1_IsUpdated
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is checking whether the table is updated or not.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     TRUE or FALSE.

**********************************************************************/
BOOL
AssociatedDevice1_IsUpdated
    (
     ANSC_HANDLE hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    return TRUE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
    AssociatedDevice1_Synchronize
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to synchronize the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
AssociatedDevice1_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_ETHERNET        pMyObject     = (PCOSA_DATAMODEL_ETHERNET)g_pCosaBEManager->hEthernet;
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;
    ULONG                           InterfaceIndex  = pEthernetPortFull->Cfg.InstanceNumber;
    ANSC_STATUS                     ret                  = ANSC_STATUS_SUCCESS;

    if(InterfaceIndex < 1)
    {
        CcspTraceWarning(("####Index number is than one \n"));
        return ret;
    }

    ret = CosaDmlEthPortGetClientMac(&pMyObject->EthernetPortFullTable[InterfaceIndex - 1], InterfaceIndex);
    return ret;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        AssociatedDevice1_GetParamStringValue
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
ULONG
AssociatedDevice1_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    UNREFERENCED_PARAMETER(pUlSize);
    PCOSA_DML_ASSOCDEV_INFO     pAssocClient = (PCOSA_DML_ASSOCDEV_INFO)hInsContext;

    if (strcmp(ParamName, "MACAddress") == 0)
    {
        AnscCopyMemory(pValue, pAssocClient->MacAddress, 18);
        return 0;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/***********************************************************************

 APIs for Object:

    Ethernet.Interface.{i}.Stats.

    *  Stats_GetParamBoolValue
    *  Stats_GetParamIntValue
    *  Stats_GetParamUlongValue
    *  Stats_GetParamStringValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Stats_GetParamBoolValue
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
Stats_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pBool);

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Stats_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Stats_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pInt);

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Stats_GetParamUlongValue
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
Stats_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;
    COSA_DML_ETH_STATS              stats;

    CosaDmlEthPortGetStats(NULL, pEthernetPortFull->Cfg.InstanceNumber, &stats);

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "PacketsSent") == 0)
    {
        /* collect value */
        *puLong = stats.PacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "PacketsReceived") == 0)
    {
        /* collect value */
        *puLong = stats.PacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "ErrorsSent") == 0)
    {
        /* collect value */
        *puLong = stats.ErrorsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "ErrorsReceived") == 0)
    {
        /* collect value */
        *puLong = stats.ErrorsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "UnicastPacketsSent") == 0)
    {
        /* collect value */
        *puLong = stats.UnicastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "UnicastPacketsReceived") == 0)
    {
        /* collect value */
        *puLong = stats.UnicastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "DiscardPacketsSent") == 0)
    {
        /* collect value */
        *puLong = stats.DiscardPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "DiscardPacketsReceived") == 0)
    {
        /* collect value */
        *puLong = stats.DiscardPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "MulticastPacketsSent") == 0)
    {
        /* collect value */
        *puLong = stats.MulticastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "MulticastPacketsReceived") == 0)
    {
        /* collect value */
        *puLong = stats.MulticastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "BroadcastPacketsSent") == 0)
    {
        /* collect value */
        *puLong = stats.BroadcastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "BroadcastPacketsReceived") == 0)
    {
        /* collect value */
        *puLong = stats.BroadcastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "UnknownProtoPacketsReceived") == 0)
    {
        /* collect value */
        *puLong = stats.UnknownProtoPacketsReceived;
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        Stats_GetParamStringValue
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
Stats_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    /* check the parameter name and return the corresponding value */
    PCOSA_DML_ETH_PORT_FULL         pEthernetPortFull = (PCOSA_DML_ETH_PORT_FULL)hInsContext;
    COSA_DML_ETH_STATS              stats;

    CosaDmlEthPortGetStats(NULL, pEthernetPortFull->Cfg.InstanceNumber, &stats);
        if (strcmp(ParamName, "BytesSent") == 0)
    {
        /* collect value */
        snprintf(pValue,*pUlSize,"%llu",stats.BytesSent);
        return 0;
    }

    if (strcmp(ParamName, "BytesReceived") == 0)
    {
        /* collect value */
        snprintf(pValue,*pUlSize,"%llu",stats.BytesReceived);
        return 0;
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); 
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pValue);
    UNREFERENCED_PARAMETER(pUlSize);*/

    return -1;
}

