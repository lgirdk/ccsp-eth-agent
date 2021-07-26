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

/*********************************************************************** 
  
    module: plugin_main.c

        Implement COSA Data Model Library Init and Unload apis.
 
    ---------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    ---------------------------------------------------------------

    revision:

        09/28/2011    initial revision.

**********************************************************************/

#include "ansc_platform.h"
#include "ansc_load_library.h"
#include "cosa_plugin_api.h"
#include "plugin_main.h"
#include "cosa_ethernet_dml.h"
#include "safec_lib_common.h"
#include "plugin_main_apis.h"

#include "cosa_ethernet_interface_dml.h"

PCOSA_BACKEND_MANAGER_OBJECT g_pCosaBEManager;

#define THIS_PLUGIN_VERSION                         1

ANSC_HANDLE g_EthObject ;

COSAGetParamValueByPathNameProc    g_GetParamValueByPathNameProc;
COSASetParamValueByPathNameProc    g_SetParamValueByPathNameProc;
COSAGetParamValueStringProc        g_GetParamValueString;
COSAGetParamValueUlongProc         g_GetParamValueUlong;
COSAGetParamValueIntProc           g_GetParamValueInt;
COSAGetParamValueBoolProc          g_GetParamValueBool;
COSASetParamValueStringProc        g_SetParamValueString;
COSASetParamValueUlongProc         g_SetParamValueUlong;
COSASetParamValueIntProc           g_SetParamValueInt;
COSASetParamValueBoolProc          g_SetParamValueBool;
COSAGetInstanceNumbersProc         g_GetInstanceNumbers;

COSAValidateHierarchyInterfaceProc g_ValidateInterface;
COSAGetHandleProc                  g_GetRegistryRootFolder;
COSAGetInstanceNumberByIndexProc   g_GetInstanceNumberByIndex;
COSAGetInterfaceByNameProc         g_GetInterfaceByName;
COSAGetHandleProc                  g_GetMessageBusHandle;
COSAGetSubsystemPrefixProc         g_GetSubsystemPrefix;
PCCSP_CCD_INTERFACE                g_pPnmCcdIf;
ANSC_HANDLE                        g_MessageBusHandle;
char*                              g_SubsystemPrefix;
COSARegisterCallBackAfterInitDmlProc  g_RegisterCallBackAfterInitDml;
COSARepopulateTableProc            g_COSARepopulateTable;

void *                       g_pDslhDmlAgent;
extern ANSC_HANDLE     g_MessageBusHandle_Irep;
extern char            g_SubSysPrefix_Irep[32];

int ANSC_EXPORT_API
COSA_Init
    (
        ULONG                       uMaxVersionSupported, 
        void*                       hCosaPlugInfo         /* PCOSA_PLUGIN_INFO passed in by the caller */
    )
{
    PCOSA_PLUGIN_INFO               pPlugInfo  = (PCOSA_PLUGIN_INFO)hCosaPlugInfo;

    COSAGetParamValueByPathNameProc pGetParamValueByPathNameProc = (COSAGetParamValueByPathNameProc)NULL;
    COSASetParamValueByPathNameProc pSetParamValueByPathNameProc = (COSASetParamValueByPathNameProc)NULL;
    COSAGetParamValueStringProc     pGetStringProc              = (COSAGetParamValueStringProc       )NULL;
    COSAGetParamValueUlongProc      pGetParamValueUlongProc     = (COSAGetParamValueUlongProc        )NULL;
    COSAGetParamValueIntProc        pGetParamValueIntProc       = (COSAGetParamValueIntProc          )NULL;
    COSAGetParamValueBoolProc       pGetParamValueBoolProc      = (COSAGetParamValueBoolProc         )NULL;
    COSASetParamValueStringProc     pSetStringProc              = (COSASetParamValueStringProc       )NULL;
    COSASetParamValueUlongProc      pSetParamValueUlongProc     = (COSASetParamValueUlongProc        )NULL;
    COSASetParamValueIntProc        pSetParamValueIntProc       = (COSASetParamValueIntProc          )NULL;
    COSASetParamValueBoolProc       pSetParamValueBoolProc      = (COSASetParamValueBoolProc         )NULL;
    COSAGetInstanceNumbersProc      pGetInstanceNumbersProc     = (COSAGetInstanceNumbersProc        )NULL;

    COSAValidateHierarchyInterfaceProc 
                                    pValInterfaceProc           = (COSAValidateHierarchyInterfaceProc)NULL;
    COSAGetHandleProc               pGetRegistryRootFolder      = (COSAGetHandleProc                 )NULL;
    COSAGetInstanceNumberByIndexProc
                                    pGetInsNumberByIndexProc    = (COSAGetInstanceNumberByIndexProc  )NULL;
    COSAGetInterfaceByNameProc      pGetInterfaceByNameProc     = (COSAGetInterfaceByNameProc        )NULL;
    errno_t        rc = -1;

    if ( uMaxVersionSupported < THIS_PLUGIN_VERSION )
    {
      /* this version is not supported */
        return -1;
    }   
    
    pPlugInfo->uPluginVersion       = THIS_PLUGIN_VERSION;
    /* register the back-end apis for the data model */
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "AutowanFeatureSupport_GetParamBoolValue",  AutowanFeatureSupport_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Ethernet_GetParamBoolValue",  Ethernet_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Ethernet_SetParamBoolValue",  Ethernet_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthWan_GetParamBoolValue", EthWan_GetParamBoolValue);
#if !defined(AUTOWAN_ENABLE)
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthWan_SetParamBoolValue", EthWan_SetParamBoolValue);
#endif
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthWan_GetParamUlongValue", EthWan_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthLogging_GetParamBoolValue", EthLogging_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthLogging_GetParamUlongValue", EthLogging_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthLogging_SetParamBoolValue", EthLogging_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthLogging_SetParamUlongValue", EthLogging_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthLogging_Validate", EthLogging_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthLogging_Commit", EthLogging_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthLogging_Rollback", EthLogging_Rollback);
#if defined(AUTOWAN_ENABLE)
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthernetWAN_GetParamStringValue", EthernetWAN_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthernetWAN_SetParamStringValue", EthernetWAN_SetParamStringValue);
#endif
#if defined(FEATURE_RDKB_WAN_MANAGER)
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthernetWAN_GetParamBoolValue", EthernetWAN_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthernetWAN_SetParamBoolValue", EthernetWAN_SetParamBoolValue);
    
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_GetEntryCount", Interface_GetEntryCount);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_GetEntry", Interface_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_GetParamBoolValue", Interface_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_GetParamStringValue", Interface_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_GetParamUlongValue", Interface_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_GetParamIntValue", Interface_GetParamIntValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_SetParamBoolValue", Interface_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_SetParamStringValue", Interface_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_SetParamUlongValue", Interface_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_SetParamIntValue", Interface_SetParamIntValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_Validate", Interface_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_Commit", Interface_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Interface_Rollback", Interface_Rollback);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "AssociatedDevice1_GetEntryCount", AssociatedDevice1_GetEntryCount);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "AssociatedDevice1_GetEntry", AssociatedDevice1_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "AssociatedDevice1_GetParamStringValue", AssociatedDevice1_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "AssociatedDevice1_IsUpdated", AssociatedDevice1_IsUpdated);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "AssociatedDevice1_Synchronize", AssociatedDevice1_Synchronize);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Stats_GetParamBoolValue", Stats_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Stats_GetParamStringValue", Stats_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Stats_GetParamUlongValue", Stats_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Stats_GetParamIntValue", Stats_GetParamIntValue);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_GetEntryCount", EthRdkInterface_GetEntryCount);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_GetEntry", EthRdkInterface_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_AddEntry", EthRdkInterface_AddEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_DelEntry", EthRdkInterface_DelEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_GetParamBoolValue", EthRdkInterface_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_GetParamStringValue", EthRdkInterface_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_GetParamUlongValue", EthRdkInterface_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_SetParamBoolValue", EthRdkInterface_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_SetParamUlongValue", EthRdkInterface_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_SetParamStringValue", EthRdkInterface_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_Validate", EthRdkInterface_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_Commit", EthRdkInterface_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthRdkInterface_Rollback", EthRdkInterface_Rollback);
#elif defined(FEATURE_RDKB_WAN_AGENT)
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_GetEntryCount", EthInterface_GetEntryCount);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_GetEntry", EthInterface_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_GetParamBoolValue", EthInterface_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_GetParamStringValue", EthInterface_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_GetParamUlongValue", EthInterface_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_SetParamBoolValue", EthInterface_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_SetParamUlongValue", EthInterface_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_Validate", EthInterface_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_Commit", EthInterface_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EthInterface_Rollback", EthInterface_Rollback);
#endif
    g_pDslhDmlAgent                 = pPlugInfo->hDmlAgent;
    pGetParamValueByPathNameProc = (COSAGetParamValueByPathNameProc)pPlugInfo->AcquireFunction("COSAGetParamValueByPathName");
    if( pGetParamValueByPathNameProc != NULL)
    {
        g_GetParamValueByPathNameProc = pGetParamValueByPathNameProc;   
    }
    else
    {
        goto EXIT;
    }
    pSetParamValueByPathNameProc = (COSASetParamValueByPathNameProc)pPlugInfo->AcquireFunction("COSASetParamValueByPathName");

    if( pSetParamValueByPathNameProc != NULL)
    {
        g_SetParamValueByPathNameProc = pSetParamValueByPathNameProc;   
    }
    else
    {
        goto EXIT;
    }
    pGetStringProc = (COSAGetParamValueStringProc)pPlugInfo->AcquireFunction("COSAGetParamValueString");

    if( pGetStringProc != NULL)
    {
        g_GetParamValueString = pGetStringProc;   
    }
    else
    {
        goto EXIT;
    }
    pGetParamValueUlongProc = (COSAGetParamValueUlongProc)pPlugInfo->AcquireFunction("COSAGetParamValueUlong");

    if( pGetParamValueUlongProc != NULL)
    {
        g_GetParamValueUlong = pGetParamValueUlongProc;   
    }
    else
    {
        goto EXIT;
    }

    pGetParamValueIntProc = (COSAGetParamValueIntProc)pPlugInfo->AcquireFunction("COSAGetParamValueInt");

    if( pGetParamValueIntProc != NULL)
    {
        g_GetParamValueInt = pGetParamValueIntProc;   
    }
    else
    {
        goto EXIT;
    }
    pGetParamValueBoolProc = (COSAGetParamValueBoolProc)pPlugInfo->AcquireFunction("COSAGetParamValueBool");

    if( pGetParamValueBoolProc != NULL)
    {
        g_GetParamValueBool = pGetParamValueBoolProc;   
    }
    else
    {
        goto EXIT;
    }
    pSetStringProc = (COSASetParamValueStringProc)pPlugInfo->AcquireFunction("COSASetParamValueString");

    if( pSetStringProc != NULL)
    {
        g_SetParamValueString = pSetStringProc;   
    }
    else
    {
        goto EXIT;
    }
    pSetParamValueUlongProc = (COSASetParamValueUlongProc)pPlugInfo->AcquireFunction("COSASetParamValueUlong");

    if( pSetParamValueUlongProc != NULL)
    {
        g_SetParamValueUlong = pSetParamValueUlongProc;   
    }
    else
    {
        goto EXIT;
    }

    pSetParamValueIntProc = (COSASetParamValueIntProc)pPlugInfo->AcquireFunction("COSASetParamValueInt");

    if( pSetParamValueIntProc != NULL)
    {
        g_SetParamValueInt = pSetParamValueIntProc;   
    }
    else
    {
        goto EXIT;
    }
    pSetParamValueBoolProc = (COSASetParamValueBoolProc)pPlugInfo->AcquireFunction("COSASetParamValueBool");

    if( pSetParamValueBoolProc != NULL)
    {
        g_SetParamValueBool = pSetParamValueBoolProc;   
    }
    else
    {
        goto EXIT;
    }
    pGetInstanceNumbersProc = (COSAGetInstanceNumbersProc)pPlugInfo->AcquireFunction("COSAGetInstanceNumbers");

    if( pGetInstanceNumbersProc != NULL)
    {
        g_GetInstanceNumbers = pGetInstanceNumbersProc;   
    }
    else
    {
        goto EXIT;
    }
    pValInterfaceProc = (COSAValidateHierarchyInterfaceProc)pPlugInfo->AcquireFunction("COSAValidateHierarchyInterface");

    if ( pValInterfaceProc )
    {
        g_ValidateInterface = pValInterfaceProc;
    }
    else
    {
        goto EXIT;
    }
    pGetRegistryRootFolder = (COSAGetHandleProc)pPlugInfo->AcquireFunction("COSAGetRegistryRootFolder");

    if ( pGetRegistryRootFolder != NULL )
    {
        g_GetRegistryRootFolder = pGetRegistryRootFolder;
    }
    else
    {
        CcspTraceInfo(("!!! haha, catcha !!!\n"));
        goto EXIT;
    }
    pGetInsNumberByIndexProc = (COSAGetInstanceNumberByIndexProc)pPlugInfo->AcquireFunction("COSAGetInstanceNumberByIndex");

    if ( pGetInsNumberByIndexProc != NULL )
    {
        g_GetInstanceNumberByIndex = pGetInsNumberByIndexProc;
    }
    else
    {
        goto EXIT;
    }
    pGetInterfaceByNameProc = (COSAGetInterfaceByNameProc)pPlugInfo->AcquireFunction("COSAGetInterfaceByName");

    if ( pGetInterfaceByNameProc != NULL )
    {
        g_GetInterfaceByName = pGetInterfaceByNameProc;
    }
    else
    {
        goto EXIT;
    }
    g_pPnmCcdIf = g_GetInterfaceByName(g_pDslhDmlAgent, CCSP_CCD_INTERFACE_NAME);

    if ( !g_pPnmCcdIf )
    {
        CcspTraceError(("g_pPnmCcdIf is NULL !\n"));

        goto EXIT;
    }
    g_RegisterCallBackAfterInitDml = (COSARegisterCallBackAfterInitDmlProc)pPlugInfo->AcquireFunction("COSARegisterCallBackAfterInitDml");

    if ( !g_RegisterCallBackAfterInitDml )
    {
        goto EXIT;
    }
    g_COSARepopulateTable = (COSARepopulateTableProc)pPlugInfo->AcquireFunction("COSARepopulateTable");

    if ( !g_COSARepopulateTable )
    {
        goto EXIT;
    }
    /* Get Message Bus Handle */
    g_GetMessageBusHandle = (COSAGetHandleProc)pPlugInfo->AcquireFunction("COSAGetMessageBusHandle");
    if ( g_GetMessageBusHandle == NULL )
    {
        goto EXIT;
    }
    g_MessageBusHandle = (ANSC_HANDLE)g_GetMessageBusHandle(g_pDslhDmlAgent);
    if ( g_MessageBusHandle == NULL )
    {
        goto EXIT;
    }
    g_MessageBusHandle_Irep = g_MessageBusHandle;
    
    /* Get Subsystem prefix */
    g_GetSubsystemPrefix = (COSAGetSubsystemPrefixProc)pPlugInfo->AcquireFunction("COSAGetSubsystemPrefix");
    if ( g_GetSubsystemPrefix != NULL )
    {
        char*   tmpSubsystemPrefix;
        
        if (( tmpSubsystemPrefix = g_GetSubsystemPrefix(g_pDslhDmlAgent) ))
        {
            rc =  strcpy_s(g_SubSysPrefix_Irep,sizeof(g_SubSysPrefix_Irep) ,tmpSubsystemPrefix);
            if(rc != EOK)
          {
            ERR_CHK(rc);
            return -1;
           }
        }

        /* retrieve the subsystem prefix */
        g_SubsystemPrefix = g_GetSubsystemPrefix(g_pDslhDmlAgent);
    }

g_EthObject          = (ANSC_HANDLE)CosaEthernetCreate();

#if defined(FEATURE_RDKB_WAN_MANAGER)

    /* Create backend framework */
    g_pCosaBEManager = (PCOSA_BACKEND_MANAGER_OBJECT)CosaBackEndManagerCreate();

    if ( g_pCosaBEManager && g_pCosaBEManager->Initialize )
    {
        g_pCosaBEManager->hCosaPluginInfo = pPlugInfo;

        g_pCosaBEManager->Initialize   ((ANSC_HANDLE)g_pCosaBEManager);
    }
#endif

    return  0;
EXIT:

    return -1;

}

BOOL ANSC_EXPORT_API
COSA_IsObjectSupported
    (
        char*                        pObjName
    )
{
    UNREFERENCED_PARAMETER(pObjName);
    return TRUE;
}

void ANSC_EXPORT_API
COSA_Unload
    (
        void
    )
{
    /* unload the memory here */
}
