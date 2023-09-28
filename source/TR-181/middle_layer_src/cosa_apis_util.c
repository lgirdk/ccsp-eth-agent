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


/**********************************************************************

    module:	cosa_apis_util.h

        This is base file for all parameters H files.

    ---------------------------------------------------------------

    description:

        This file contains all utility functions for COSA DML API development.

    ---------------------------------------------------------------

    environment:

        COSA independent

    ---------------------------------------------------------------

    author:

        Roger Hu
        leichen2@cisco.com

    ---------------------------------------------------------------

    revision:

        01/30/2011    initial revision.
        06/15/2012    add IPv4 address utils

**********************************************************************/



#include "cosa_apis.h"
#include "plugin_main_apis.h"

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/types.h>
#include "ansc_platform.h"

//$HL 4/30/2013
#include "ccsp_psm_helper.h"
#include "secure_wrapper.h"
#include "safec_lib_common.h"

#define MAX_VALUE 3
#define LINK_SYS_PATH_SIZE 256

ANSC_STATUS
COSAGetParamValueByPathName
    (
        void*                       bus_handle,
        parameterValStruct_t        *val,
        ULONG                       *parameterValueLength
    );

ANSC_STATUS
CosaUtilStringToHex
    (
        char          *str,
        unsigned char *hex_str,
        int           hex_sz 
    )
{
    INT val = 0;
    INT i= 0;
    INT index= 0;
    CHAR  byte[3]       = {'\0'};

    while(str[i] != '\0')
    {
        byte[0] = str[i];
        byte[1] = str[i+1];
        byte[2] = '\0';
        if(_ansc_sscanf(byte, "%x", &val) != 1)
            break;
	hex_str[index] = val;
        i += 2;
        index++;
        if (str[i] == ':' || str[i] == '-'  || str[i] == '_')
            i++;
    }
    if(index != hex_sz)
        return ANSC_STATUS_FAILURE;

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaSListPushEntryByInsNum
    (
        PSLIST_HEADER               pListHead,
        PCOSA_CONTEXT_LINK_OBJECT   pCosaContext
    )
{
    PCOSA_CONTEXT_LINK_OBJECT       pCosaContextEntry = (PCOSA_CONTEXT_LINK_OBJECT)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY       )NULL;
    ULONG                           ulIndex           = 0;

    if ( pListHead->Depth == 0 )
    {
        AnscSListPushEntryAtBack(pListHead, &pCosaContext->Linkage);
    }
    else
    {
        pSLinkEntry = AnscSListGetFirstEntry(pListHead);

        for ( ulIndex = 0; ulIndex < pListHead->Depth; ulIndex++ )
        {
            pCosaContextEntry = ACCESS_COSA_CONTEXT_LINK_OBJECT(pSLinkEntry);
            pSLinkEntry       = AnscSListGetNextEntry(pSLinkEntry);

            if ( pCosaContext->InstanceNumber < pCosaContextEntry->InstanceNumber )
            {
                AnscSListPushEntryByIndex(pListHead, &pCosaContext->Linkage, ulIndex);

                return ANSC_STATUS_SUCCESS;
            }
        }

        AnscSListPushEntryAtBack(pListHead, &pCosaContext->Linkage);
    }

    return ANSC_STATUS_SUCCESS;
}

PCOSA_CONTEXT_LINK_OBJECT
CosaSListGetEntryByInsNum
    (
        PSLIST_HEADER               pListHead,
        ULONG                       InstanceNumber
    )
{
    PCOSA_CONTEXT_LINK_OBJECT       pCosaContextEntry = (PCOSA_CONTEXT_LINK_OBJECT)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY       )NULL;
    ULONG                           ulIndex           = 0;

    if ( pListHead->Depth == 0 )
    {
        return NULL;
    }
    else
    {
        pSLinkEntry = AnscSListGetFirstEntry(pListHead);

        for ( ulIndex = 0; ulIndex < pListHead->Depth; ulIndex++ )
        {
            pCosaContextEntry = ACCESS_COSA_CONTEXT_LINK_OBJECT(pSLinkEntry);
            pSLinkEntry       = AnscSListGetNextEntry(pSLinkEntry);

            if ( pCosaContextEntry->InstanceNumber == InstanceNumber )
            {
                return pCosaContextEntry;
            }
        }
    }

    return NULL;
}

PUCHAR
CosaUtilGetLowerLayers
    (
        PUCHAR                      pTableName,
        PUCHAR                      pKeyword
    )
{

    ULONG                           ulNumOfEntries              = 0;
    ULONG                           i                           = 0;
    ULONG                           j                           = 0;
    ULONG                           ulEntryNameLen              = 256;
    CHAR                            ucEntryParamName[256+15]       = {0};
    CHAR                            ucEntryNameValue[256]       = {0};
    CHAR                            ucEntryFullPath[256]        = {0};
    CHAR                            ucLowerEntryPath[256+25]       = {0};
    CHAR                            ucLowerEntryName[256+7]       = {0};
    ULONG                           ulEntryInstanceNum          = 0;
    ULONG                           ulEntryPortNum              = 0;
    char*                           pMatchedLowerLayer          = NULL;
    PANSC_TOKEN_CHAIN               pTableListTokenChain        = (PANSC_TOKEN_CHAIN)NULL;
    PANSC_STRING_TOKEN              pTableStringToken           = (PANSC_STRING_TOKEN)NULL;
    errno_t rc       = -1;
    int     ind      = -1;

    if ( !pTableName || AnscSizeOfString((char*)pTableName) == 0 ||
         !pKeyword   || AnscSizeOfString((char*)pKeyword) == 0
       )
    {
        return NULL;
    }

    pTableListTokenChain = AnscTcAllocate((char*)pTableName, ",");

    if ( !pTableListTokenChain )
    {
        return NULL;
    }

    while ((pTableStringToken = AnscTcUnlinkToken(pTableListTokenChain)))
    {
	/* CID 56300 Array compared against 0 */
        if ( pTableStringToken->Name[0] != '\0' )
        {
            rc = strcmp_s("Device.Ethernet.Interface.",strlen("Device.Ethernet.Interface."),pTableStringToken->Name,&ind);
            ERR_CHK(rc);
            if ((!ind) && (rc == EOK))
            {
                ulNumOfEntries =       CosaGetParamValueUlong("Device.Ethernet.InterfaceNumberOfEntries");

                for ( i = 0 ; i < ulNumOfEntries; i++ )
                {
                    ulEntryInstanceNum = CosaGetInstanceNumberByIndex("Device.Ethernet.Interface.", i);

                    if ( ulEntryInstanceNum )
                    {
                        /* Coverity Fix:CID 73664 DC.STRING_BUFFER */
                        snprintf(ucEntryFullPath, sizeof(ucEntryFullPath),"%s%lu", "Device.Ethernet.Interface.", ulEntryInstanceNum);

                        snprintf(ucEntryParamName,sizeof(ucEntryParamName), "%s%s", ucEntryFullPath, ".Name");
               
                        ulEntryNameLen = sizeof(ucEntryNameValue);
                        rc = strcmp_s(ucEntryNameValue,ulEntryNameLen,(char*)pKeyword,&ind);
                        ERR_CHK(rc);
                        if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                             ((!ind) && (rc == EOK)) )
                        {
                            pMatchedLowerLayer =  AnscCloneString(ucEntryFullPath);

                            break;
                        }
                    }
                }
            }
            else
            {
                 rc = strcmp_s("Device.IP.Interface.",strlen("Device.IP.Interface."),pTableStringToken->Name,&ind);
                 ERR_CHK(rc);
                 if ((!ind) && (rc == EOK))
                {
                   ulNumOfEntries =       CosaGetParamValueUlong("Device.IP.InterfaceNumberOfEntries");
                   for ( i = 0 ; i < ulNumOfEntries; i++ )
                   {
                      ulEntryInstanceNum = CosaGetInstanceNumberByIndex("Device.IP.Interface.", i);

                    if ( ulEntryInstanceNum )
                    {
                        /*Coverity Fix:CID 73664 DC.STRING_BUFFER */
                        snprintf(ucEntryFullPath, sizeof(ucEntryFullPath),"%s%lu", "Device.IP.Interface.", ulEntryInstanceNum);

                        snprintf(ucEntryParamName,sizeof(ucEntryParamName), "%s%s", ucEntryFullPath, ".Name");

                        ulEntryNameLen = sizeof(ucEntryNameValue); 
                        rc = strcmp_s(ucEntryNameValue,ulEntryNameLen,(char*)pKeyword,&ind);
                         ERR_CHK(rc);
                        if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                             ((!ind) && (rc == EOK)))
                        {
                            pMatchedLowerLayer =  AnscCloneString(ucEntryFullPath);

                            break;
                        }
                      }
                   }
               }
               else
               {
                     rc = strcmp_s("Device.WiFi.Radio.",strlen("Device.WiFi.Radio."),pTableStringToken->Name,&ind);
                     ERR_CHK(rc);
                    if ((!ind) && (rc == EOK))

                    {
                     ulNumOfEntries =       CosaGetParamValueUlong("Device.WiFi.RadioNumberOfEntries");

                       for (i = 0; i < ulNumOfEntries; i++)
                      {
                        ulEntryInstanceNum = CosaGetInstanceNumberByIndex("Device.WiFi.Radio.", i);
                    
                        if (ulEntryInstanceNum)
                       {
                        /* Coverity Fix CID:73664 DC.STRING_BUFFER */
                        snprintf(ucEntryFullPath, sizeof(ucEntryFullPath),"%s%lu.", "Device.WiFi.Radio.", ulEntryInstanceNum);
                        
                        snprintf(ucEntryParamName, sizeof(ucEntryParamName),"%s%s", ucEntryFullPath, "Name");
                        
                            ulEntryNameLen = sizeof(ucEntryNameValue);
                             rc = strcmp_s(ucEntryNameValue,ulEntryNameLen,(char*)pKeyword,&ind);
                             ERR_CHK(rc);
                             if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                             ((!ind) && (rc == EOK)))

                            {
                               pMatchedLowerLayer = AnscCloneString(ucEntryFullPath);
                            
                               break;
                            }
                       }
                     }
                   }
                     else
                     {
                         rc = strcmp_s("Device.MoCA.Interface.",strlen("Device.MoCA.Interface."),pTableStringToken->Name,&ind);
                         ERR_CHK(rc);
                         if ((!ind) && (rc == EOK))

                          {
                            ulNumOfEntries =       CosaGetParamValueUlong("Device.MoCA.InterfaceNumberOfEntries");

                             for ( i = 0 ; i < ulNumOfEntries; i++ )
                            {
                                ulEntryInstanceNum = CosaGetInstanceNumberByIndex("Device.MoCA.Interface.", i);

                               if ( ulEntryInstanceNum )
                               {
                                   /*Coverity Fix CID:73664 DC.STRING_BUFFER */
                        snprintf(ucEntryFullPath, sizeof(ucEntryFullPath),"%s%lu", "Device.MoCA.Interface.", ulEntryInstanceNum);

                        snprintf(ucEntryParamName, sizeof(ucEntryParamName),"%s%s", ucEntryFullPath, ".Name");
               
                                       ulEntryNameLen = sizeof(ucEntryNameValue);
                                      rc = strcmp_s(ucEntryNameValue,ulEntryNameLen,(char*)pKeyword,&ind);
                                      ERR_CHK(rc);
                                 if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                             ((!ind) && (rc == EOK)) )
                            {
                                  pMatchedLowerLayer =  AnscCloneString(ucEntryFullPath);

                                   break;
                            }
                               }
                            }
                         }
               else 
               {
                 rc = strcmp_s("Device.X_CISCO_COM_GRE.Interface.",strlen("Device.X_CISCO_COM_GRE.Interface."),pTableStringToken->Name,&ind);
                 ERR_CHK(rc);
                 if ((!ind) && (rc == EOK))

                {
                 ulNumOfEntries =       CosaGetParamValueUlong("Device.X_CISCO_COM_GRE.InterfaceNumberOfEntries");

                 for ( i = 0 ; i < ulNumOfEntries; i++ )
                 {
                    ulEntryInstanceNum = CosaGetInstanceNumberByIndex("Device.X_CISCO_COM_GRE.Interface.", i);

                    if ( ulEntryInstanceNum )
                    {
                        snprintf(ucEntryFullPath, sizeof(ucEntryFullPath),"%s%lu", "Device.X_CISCO_COM_GRE.Interface.", ulEntryInstanceNum);

                        snprintf(ucEntryParamName, sizeof(ucEntryParamName),"%s%s", ucEntryFullPath, ".Name");
               
                        ulEntryNameLen = sizeof(ucEntryNameValue);
                         rc = strcmp_s(ucEntryNameValue,ulEntryNameLen,(char*)pKeyword,&ind);
                          ERR_CHK(rc);
                             if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                             ((!ind) && (rc == EOK)))

                        {
                            pMatchedLowerLayer =  AnscCloneString(ucEntryFullPath);

                            break;
                        }
                     }
                 }
               }
               else
               {
                  rc = strcmp_s("Device.Ethernet.Link.",strlen("Device.Ethernet.Link."),pTableStringToken->Name,&ind);
                 ERR_CHK(rc);
                 if ((!ind) && (rc == EOK))

            {
                ulNumOfEntries =       CosaGetParamValueUlong("Device.Ethernet.LinkNumberOfEntries");

                for ( i = 0 ; i < ulNumOfEntries; i++ )
                {
                    ulEntryInstanceNum = CosaGetInstanceNumberByIndex("Device.Ethernet.Link.", i);

                    if ( ulEntryInstanceNum )
                    {
                        /*Coverity Fix CID:73664 DC.STRING_BUFFER */
                        snprintf(ucEntryFullPath,sizeof(ucEntryFullPath), "%s%lu", "Device.Ethernet.Link.", ulEntryInstanceNum);

                        snprintf(ucEntryParamName, sizeof(ucEntryParamName), "%s%s", ucEntryFullPath, ".Name");
               
                        ulEntryNameLen = sizeof(ucEntryNameValue);
                         rc = strcmp_s(ucEntryNameValue,ulEntryNameLen,(char*)pKeyword,&ind);
                         ERR_CHK(rc);
                             if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                             ((!ind) && (rc == EOK)))

                        {
                            pMatchedLowerLayer =  AnscCloneString(ucEntryFullPath);

                            break;
                        }
                    }
                }
            }
               else 
               {
                  rc = strcmp_s("Device.WiFi.SSID.",strlen("Device.WiFi.SSID."),pTableStringToken->Name,&ind);
                 ERR_CHK(rc);
                 if ((!ind) && (rc == EOK))
                {
                  parameterValStruct_t varStruct;
                  ulNumOfEntries = 0;
                  rc = strcpy_s(ucEntryParamName,sizeof(ucEntryParamName),"Device.WiFi.SSIDNumberOfEntries");
                  if(rc != EOK)
                  {
                    ERR_CHK(rc);
                    return NULL;
                  }
                
                 
                  varStruct.parameterName = ucEntryParamName;
                  varStruct.parameterValue = ucEntryNameValue;
                  ulEntryNameLen = sizeof(ucEntryNameValue);
                if (COSAGetParamValueByPathName(g_MessageBusHandle,&varStruct,&ulEntryNameLen))
                {
                    AnscTraceFlow(("<HL>%s not found %s\n",__FUNCTION__,varStruct.parameterName ));
                    break;
                }
                AnscTraceFlow(("<HL>%s ucEntryNameValue=%s\n", __FUNCTION__,ucEntryNameValue));
                _ansc_sscanf(ucEntryNameValue,"%lu",&ulNumOfEntries);
                AnscTraceFlow(("<HL>%s Wifi # of entries=%lu\n", __FUNCTION__,ulNumOfEntries));
                i = 0;
                ulEntryInstanceNum =1;
                while (i < ulNumOfEntries)
                {
                    rc = memset_s(ucEntryParamName,sizeof(ucEntryParamName), 0, sizeof(ucEntryParamName));
                    ERR_CHK(rc);
                    rc = memset_s(ucEntryNameValue,sizeof(ucEntryNameValue), 0, sizeof(ucEntryNameValue));
                     ERR_CHK(rc);
                    _ansc_sprintf(ucEntryParamName,"Device.WiFi.SSID.%lu.Name",ulEntryInstanceNum);                    
                        /*Coverity Fix CID:73664 DC.STRING_BUFFER */
                    snprintf(ucEntryParamName,sizeof(ucEntryParamName),"Device.WiFi.SSID.%lu.Name",ulEntryInstanceNum);                    
                        
                    ulEntryNameLen = sizeof(ucEntryNameValue);
                    if (COSAGetParamValueByPathName(g_MessageBusHandle,&varStruct,&ulEntryNameLen))
                    {
                        AnscTraceFlow(("<HL>%s WiFi instance(%lu) not found\n", __FUNCTION__,
                            ulEntryInstanceNum));
                        ulEntryInstanceNum++;
                        continue;
                    }  
                    AnscTraceFlow(("<HL>%s WiFi instance(%lu) has name =%s inputName=%s\n",
                        __FUNCTION__,ulEntryInstanceNum,ucEntryNameValue,pKeyword));
                     rc = strcmp_s(ucEntryNameValue,sizeof(ucEntryNameValue),(char*)pKeyword,&ind);
                     ERR_CHK(rc);
                     if ((!ind) && (rc == EOK))
                    {
                        /*Coverity Fix CID:73664 DC.STRING_BUFFER */
                        snprintf(ucEntryFullPath,sizeof(ucEntryFullPath),"Device.WiFi.SSID.%lu",ulEntryInstanceNum);
                        pMatchedLowerLayer =  AnscCloneString(ucEntryFullPath);

                        break;
                    }
                    ulEntryInstanceNum++;
                    i++;
                }
            }
              else
              {
                rc = strcmp_s("Device.Bridging.Bridge.",strlen("Device.Bridging.Bridge."),pTableStringToken->Name,&ind);
                ERR_CHK(rc);
                 if ((!ind) && (rc == EOK))
               {
                 ulNumOfEntries =  CosaGetParamValueUlong("Device.Bridging.BridgeNumberOfEntries");
                 CcspTraceInfo(("----------CosaUtilGetLowerLayers, bridgenum:%lu\n", ulNumOfEntries));
                 for ( i = 0 ; i < ulNumOfEntries; i++ )
                 {
                    ulEntryInstanceNum = CosaGetInstanceNumberByIndex("Device.Bridging.Bridge.", i);
                    CcspTraceInfo(("----------CosaUtilGetLowerLayers, instance num:%lu\n", ulEntryInstanceNum));

                    if ( ulEntryInstanceNum )
                    {
                        /*Coverity Fix CID:73664 DC.STRING_BUFFER */
                        snprintf(ucEntryFullPath,sizeof(ucEntryFullPath), "%s%lu", "Device.Bridging.Bridge.", ulEntryInstanceNum);
                        snprintf(ucLowerEntryPath,sizeof(ucLowerEntryPath), "%s%s", ucEntryFullPath, ".PortNumberOfEntries"); 
                        
                        ulEntryPortNum = CosaGetParamValueUlong(ucLowerEntryPath);  
                        CcspTraceInfo(("----------CosaUtilGetLowerLayers, Param:%s,port num:%lu\n",ucLowerEntryPath, ulEntryPortNum));

                        for ( j = 1; j<= ulEntryPortNum; j++) {
                        /*Coverity Fix CID:73664 DC.STRING_BUFFER */
                            snprintf(ucLowerEntryName,sizeof(ucLowerEntryName), "%s%s%lu", ucEntryFullPath, ".Port.", j);
                            snprintf(ucEntryParamName,sizeof(ucEntryParamName), "%s%s%lu%s", ucEntryFullPath, ".Port.", j, ".Name");
                            CcspTraceInfo(("----------CosaUtilGetLowerLayers, Param:%s,Param2:%s\n", ucLowerEntryName, ucEntryParamName));
                        
                            ulEntryNameLen = sizeof(ucEntryNameValue);
                             rc = strcmp_s(ucEntryNameValue,ulEntryNameLen,(char*)pKeyword,&ind);
                             ERR_CHK(rc);
                             if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                             ((!ind) && (rc == EOK)))

                            {
                                pMatchedLowerLayer =  AnscCloneString(ucLowerEntryName);
                                CcspTraceInfo(("----------CosaUtilGetLowerLayers, J:%lu, LowerLayer:%s\n", j, pMatchedLowerLayer));
                                break;
                            }
                        }
                    }
                }
            }
             }
           }
         }
       }
     }
   }
 }
            if ( pMatchedLowerLayer )
            {
                AnscFreeMemory(pTableStringToken);
                break;
            }
        }

        AnscFreeMemory(pTableStringToken);
    }

    if ( pTableListTokenChain )
    {
        AnscTcFree((ANSC_HANDLE)pTableListTokenChain);
    }

    /* Too many trace. Disable this  -- Yan */
    /*
    CcspTraceWarning
        ((
            "CosaUtilGetLowerLayers: %s matched LowerLayer(%s) with keyword %s in the table %s\n",
            pMatchedLowerLayer ? "Found a":"Not find any",
            pMatchedLowerLayer ? pMatchedLowerLayer : "",
            pKeyword,
            pTableName
        ));
    */

    return (PUCHAR)pMatchedLowerLayer;
}

/*
 *
 */
/*mapping for LinkType conversion*/

/* please don't alter LinkType, it can be used as index starting with 1*/
LINKTYPE_MAP_T g_linktype_map[COSA_DML_LINK_TYPE_TOTAL] = {
    {   "Device.Ethernet.Interface.", 
        "Ethernet", 
        COSA_DML_LINK_TYPE_Eth 
    },
    {   "Device.Ethernet.Link.", 
        "EthLink",
        COSA_DML_LINK_TYPE_EthLink 
    },
    {   "Device.Ethernet.VLANTermination.", 
        "Ethernet.VLANTermination", 
        COSA_DML_LINK_TYPE_EthVlan
    },
    {   "Device.USB.Interface.", 
        "USB", 
        COSA_DML_LINK_TYPE_Usb
    },
    {   "Device.HPNA.Interface.", 
        "HPNA", 
        COSA_DML_LINK_TYPE_Hpna
    },
    {   "Device.MoCA.Interface.", 
        "MoCA", 
        COSA_DML_LINK_TYPE_Moca
    },
    {   "Device.HomePlug.Interface.", 
        "HomePlug", 
        COSA_DML_LINK_TYPE_HomePlug
    },
    {   "Device.UPA.Interface.", 
        "UPA", 
        COSA_DML_LINK_TYPE_Upa
    },
    {   "Device.WiFi.SSID.", 
        "WiFi", 
        COSA_DML_LINK_TYPE_WiFiSsid
    },
    {   "Device.Bridging.Bridge.", 
        "Bridge", 
        COSA_DML_LINK_TYPE_Bridge
    },
    {   "Device.PPP.Interface.", 
        "PPP", 
        COSA_DML_LINK_TYPE_PPP
    },
    {   "DOCSIS", 
        "DOCSIS", 
        COSA_DML_LINK_TYPE_DOCSIS
    }
};


COSA_DML_LINK_TYPE CosaUtilGetLinkTypeFromStr(char* pLinkTypeStr)
{
    int index=0;
    if(NULL == pLinkTypeStr)
        return COSA_DML_LINK_TYPE_LAST;

    for(index=0; index<COSA_DML_LINK_TYPE_TOTAL; index++)
    {
        if(NULL != g_linktype_map[index].LinkTypeStr && strstr(pLinkTypeStr, g_linktype_map[index].LinkTypeStr) != NULL)
        {
            return g_linktype_map[index].LinkType;
        }
    }

    return COSA_DML_LINK_TYPE_LAST;
}

char* CosaUtilGetStrFromLinkTypePath(char* pLinkTypePath){
    int index=0;

    if(NULL == pLinkTypePath)
        return NULL;

    AnscTraceFlow(("%s: %s\n", __FUNCTION__, pLinkTypePath));
    for(index=0; index<COSA_DML_LINK_TYPE_TOTAL; index++)
    {
        //if(!strncmp(g_linktype_map[index].LinkTypePath, pLinkTypePath, 
        //        sizeof(g_linktype_map[index].LinkTypePath)))
        if(NULL != g_linktype_map[index].LinkTypePath && strstr(pLinkTypePath, g_linktype_map[index].LinkTypePath) != NULL)
        {
            AnscTraceFlow(("%s: return index %d\n", __FUNCTION__, index));
            return g_linktype_map[index].LinkTypeStr;
        }
    }

    return NULL;
}

char* CosaUtilGetLinkTypeStr(COSA_DML_LINK_TYPE LinkType)
{
    int index=0;
    AnscTraceFlow(("%s: %d\n", __FUNCTION__, LinkType));
    for(index=0; index<COSA_DML_LINK_TYPE_TOTAL; index++)
    {
        if(g_linktype_map[index].LinkType == LinkType)
        {
            AnscTraceFlow(("%s: return index %d\n", __FUNCTION__, index));
            return g_linktype_map[index].LinkTypeStr;
        }
    }
    return NULL;
}

char* CosaUtilGetLinkTypePath(COSA_DML_LINK_TYPE LinkType)
{
    int index=0;
    AnscTraceFlow(("%s: %d\n", __FUNCTION__, LinkType));
    for(index=0; index<COSA_DML_LINK_TYPE_TOTAL; index++)
    {
        if(g_linktype_map[index].LinkType == LinkType)
        {
            AnscTraceFlow(("%s: return index %d\n", __FUNCTION__, index));
            return g_linktype_map[index].LinkTypePath;
        }
    }
    return NULL;
}

COSA_DML_LINK_TYPE CosaUtilGetLinkTypeFromPath(char*pLinkTypePath)
{
    int index=0;
    AnscTraceFlow(("%s: %s\n", __FUNCTION__, pLinkTypePath));
    for(index=0; index<COSA_DML_LINK_TYPE_TOTAL; index++)
    {
        //if(!strncmp(g_linktype_map[index].LinkTypePath, pLinkTypePath, 
        //        sizeof(g_linktype_map[index].LinkTypePath)))
        if(strstr(pLinkTypePath, g_linktype_map[index].LinkTypePath))
        {
            AnscTraceFlow(("%s: return index %d\n", __FUNCTION__, index));
            return g_linktype_map[index].LinkType;
        }
    }
    return COSA_DML_LINK_TYPE_LAST;
}

/*
 *  Retrieve the parameter Name of the LowerLayer
 */
ANSC_STATUS
CosaUtilGetLowerLayerName
    (
        COSA_DML_LINK_TYPE          LinkType,
        ULONG                       InstNumber,
        char*                       pParamValueBuf,
        PULONG                      pBufLen        
    )
{
    ANSC_STATUS                     returnStatus;
    char                            pParamPath[256];
    char                           *linkTypePath;

    linkTypePath = CosaUtilGetLinkTypePath(LinkType);

    if (LinkType == COSA_DML_LINK_TYPE_Bridge)
    {
        /* Special processing for Bridge type LowerLayers */
        snprintf(pParamPath, sizeof(pParamPath), "%s%d.Port.1.Name", linkTypePath, (int) InstNumber);
    }
    else
    {
        snprintf(pParamPath, sizeof(pParamPath), "%s%d.Name", linkTypePath, (int) InstNumber);
    }

    returnStatus = CosaGetParamValueString(pParamPath, pParamValueBuf, pBufLen);

    if (returnStatus != ANSC_STATUS_SUCCESS)
    {
        AnscTraceWarning(("CosaUtilGetLowerLayerName -- failure %lu, to retrieve %s\n", returnStatus, pParamPath));
        return returnStatus;
    }

    AnscTraceFlow(("CosaUtilGetLowerLayerName -- value %s\n", pParamValueBuf));

    return ANSC_STATUS_SUCCESS;
}

/*
 * Find the management port in Bridge and return name from that port
 * It is the name of the bridge.
 */
PUCHAR
CosaUtilFindBridgeName(char* pBridgePath)
{
    ULONG                           j                           = 0;
    ULONG                           ulEntryNameLen              = 256;
    CHAR                            ucEntryParamName[256]       = {0};
    CHAR                            ucEntryNameValue[256]       = {0};
   
    CHAR                            ucLowerEntryPath[256]       = {0};
    CHAR                            ucLowerEntryName[256]       = {0};
    ULONG                           ulEntryPortNum              = 0;
    char*                          pMatchedBridgeName          = NULL;
    BOOL                            bMgrPort                    = FALSE;
    /* Coverity Fix CID:73664 DC.STRING_BUFFER */
    snprintf(ucLowerEntryPath,sizeof(ucLowerEntryPath), "%s%s", pBridgePath, ".PortNumberOfEntries"); 
                        
    ulEntryPortNum = CosaGetParamValueUlong(ucLowerEntryPath);  
    AnscTraceFlow(("%s: Param:%s,port num:%lu\n", __FUNCTION__, ucLowerEntryPath, ulEntryPortNum));

    for ( j = 1; j<= ulEntryPortNum; j++) {
       /* Coverity Fix CID:73664 DC.STRING_BUFFER */
        snprintf(ucLowerEntryName,sizeof(ucLowerEntryName), "%s%s%lu", pBridgePath, ".Port.", j);
        snprintf(ucEntryParamName,sizeof(ucEntryParamName), "%s%s%lu%s", pBridgePath, ".Port.", j, ".ManagementPort");
        AnscTraceFlow(("%s: Param:%s,Param2:%s\n",__FUNCTION__, ucLowerEntryName, ucEntryParamName)); 
                        
        bMgrPort = CosaGetParamValueBool(ucEntryParamName);
        if(bMgrPort)
        {
             /* Coverity Fix CID:73664 DC.STRING_BUFFER */
            snprintf(ucEntryParamName,sizeof(ucEntryParamName), "%s%s%lu%s", pBridgePath, ".Port.", j, ".Name");
                
            ulEntryNameLen = sizeof(ucEntryNameValue);
            if (0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) 
            {
                // not include port instance.
                pMatchedBridgeName =  AnscCloneString(ucEntryNameValue);
                AnscTraceFlow(("%s: J:%lu, Name:%s\n", __FUNCTION__, j, pMatchedBridgeName));
                break;
            }
        }
    }

    return (PUCHAR)pMatchedBridgeName;
}

/*
 * Return Device.Bridging.Bridge.x
 * not include port instance number.
 */
PUCHAR
CosaUtilFindBridgePath(char* pBridgeName)
{
    ULONG                           ulNumOfEntries              = 0;
    ULONG                           i                           = 0;
    ULONG                           j                           = 0;
    ULONG                           ulEntryNameLen              = 256;
    CHAR                            ucEntryParamName[256+18]       = {0};
    CHAR                            ucEntryNameValue[256]       = {0};
    CHAR                            ucEntryFullPath[256]        = {0};
    CHAR                            ucLowerEntryPath[256+25]       = {0};
    CHAR                            ucLowerEntryName[256+7]       = {0};
    ULONG                           ulEntryInstanceNum          = 0;
    ULONG                           ulEntryPortNum              = 0;
    char*                           pMatchedLowerLayer          = NULL;
    errno_t        rc = -1;
    int            ind = -1;
    

    ulNumOfEntries =  CosaGetParamValueUlong("Device.Bridging.BridgeNumberOfEntries");
    CcspTraceInfo(("----------CosaUtilGetLowerLayers, bridgenum:%lu\n", ulNumOfEntries));
    AnscTraceFlow(("%s: bridgenum:%lu\n", __FUNCTION__, ulNumOfEntries));
    for ( i = 0 ; i < ulNumOfEntries; i++ )
    {
        ulEntryInstanceNum = CosaGetInstanceNumberByIndex("Device.Bridging.Bridge.", i);
        CcspTraceInfo(("----------CosaUtilGetLowerLayers, instance num:%lu\n", ulEntryInstanceNum));
        AnscTraceFlow(("%s: instance num:%lu\n", __FUNCTION__, ulEntryInstanceNum));
        if ( ulEntryInstanceNum )
        {
            /*Coverity Fix CID:73664 DC.STRING_BUFFER */
            snprintf(ucEntryFullPath,sizeof(ucEntryFullPath), "%s%lu", "Device.Bridging.Bridge.", ulEntryInstanceNum);
            snprintf(ucLowerEntryPath,sizeof(ucLowerEntryPath), "%s%s", ucEntryFullPath, ".PortNumberOfEntries"); 
                        
            ulEntryPortNum = CosaGetParamValueUlong(ucLowerEntryPath);  
            CcspTraceInfo(("----------CosaUtilGetLowerLayers, Param:%s,port num:%lu\n",ucLowerEntryPath, ulEntryPortNum));

            for ( j = 1; j<= ulEntryPortNum; j++) {
            /*Coverity Fix CID:73664 DC.STRING_BUFFER */
                snprintf(ucLowerEntryName,sizeof(ucLowerEntryName), "%s%s%lu", ucEntryFullPath, ".Port.", j);
                snprintf(ucEntryParamName,sizeof(ucEntryParamName), "%s%s%lu%s", ucEntryFullPath, ".Port.", j, ".Name");
                CcspTraceInfo(("----------CosaUtilGetLowerLayers, Param:%s,Param2:%s\n", ucLowerEntryName, ucEntryParamName));
                        
                ulEntryNameLen = sizeof(ucEntryNameValue);
                rc = strcmp_s(ucEntryNameValue,ulEntryNameLen, pBridgeName,&ind);
                ERR_CHK(rc);
                if( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) && (ind == 0) && (rc == EOK))
                {
                    // not include port instance.
                    pMatchedLowerLayer =  AnscCloneString(ucEntryFullPath);
                    CcspTraceInfo(("----------CosaUtilGetLowerLayers, J:%lu, LowerLayer:%s\n", j, pMatchedLowerLayer));
                    break;
                }
            }
        }
    }

    return (PUCHAR)pMatchedLowerLayer;
}
/*
    CosaUtilGetFullPathNameByKeyword
    
   Description:
        This funcation serves for searching other pathname  except lowerlayer.
        
    PUCHAR                      pTableName
        This is the Table names divided by ",". For example 
        "Device.Ethernet.Interface., Device.Dhcpv4." 
        
    PUCHAR                      pParameterName
        This is the parameter name which hold the keyword. eg: "name"
        
    PUCHAR                      pKeyword
        This is keyword. eg: "wan0".

    return value
        return result string which need be free by the caller.
*/
PUCHAR
CosaUtilGetFullPathNameByKeyword
    (
        PUCHAR                      pTableName,
        PUCHAR                      pParameterName,
        PUCHAR                      pKeyword
    )
{

    ULONG                           ulNumOfEntries              = 0;
    ULONG                           i                           = 0;
    ULONG                           ulEntryNameLen              = 256;
    CHAR                            ucEntryParamName[256+780]       = {0};
    CHAR                            ucEntryNameValue[256]       = {0};
    CHAR                            ucTmp[128+1024]                  = {0};
    CHAR                            ucTmp2[128+400]                 = {0};
    CHAR                            ucEntryFullPath[256+384]        = {0};
    char*                           pMatchedLowerLayer          = NULL;
    ULONG                           ulEntryInstanceNum          = 0;
    PANSC_TOKEN_CHAIN               pTableListTokenChain        = (PANSC_TOKEN_CHAIN)NULL;
    PANSC_STRING_TOKEN              pTableStringToken           = (PANSC_STRING_TOKEN)NULL;
    char*                           pString                     = NULL;
    char*                           pString2                    = NULL;
    errno_t        rc = -1;
    int            ind = -1;

    if ( !pTableName || AnscSizeOfString((char*)pTableName) == 0 ||
         !pKeyword   || AnscSizeOfString((char*)pKeyword) == 0   ||
         !pParameterName   || AnscSizeOfString((char*)pParameterName) == 0
       )
    {
        return NULL;
    }

    pTableListTokenChain = AnscTcAllocate((char*)pTableName, ",");

    if ( !pTableListTokenChain )
    {
        return NULL;
    }

    while ((pTableStringToken = AnscTcUnlinkToken(pTableListTokenChain)))
    {
	/* CID 62085 Array compared against 0 */
        if ( pTableStringToken->Name[0] != '\0' )
        {
            /* Get the string XXXNumberOfEntries */
            pString2 = &pTableStringToken->Name[0];
            pString  = pString2;
            for (i = 0;pTableStringToken->Name[i]; i++)
            {
                if ( pTableStringToken->Name[i] == '.' )
                {
                    pString2 = pString;
                    pString  = &pTableStringToken->Name[i+1];
                }
            }

            pString--;
            pString[0] = '\0';
            /*Coveity Fix CID: 73664 DC.STRING_BUFFER */
            snprintf(ucTmp2, sizeof(ucTmp2),"%s%s", pString2, "NumberOfEntries");                
            pString[0] = '.';

            /* Enumerate the entry in this table */
            if ( TRUE )
            {
                pString2--;
                pString2[0]='\0';
            /*Coveity Fix CID: 73664 DC.STRING_BUFFER */
                snprintf(ucTmp,sizeof(ucTmp), "%s.%s", pTableStringToken->Name, ucTmp2);                
                pString2[0]='.';
                ulNumOfEntries =       CosaGetParamValueUlong(ucTmp);

                for ( i = 0 ; i < ulNumOfEntries; i++ )
                {
                    ulEntryInstanceNum = CosaGetInstanceNumberByIndex(pTableStringToken->Name, i);

                    if ( ulEntryInstanceNum )
                    {
            /*Coveity Fix CID: 73664 DC.STRING_BUFFER */
                        snprintf(ucEntryFullPath, sizeof(ucEntryFullPath),"%s%lu%s", pTableStringToken->Name, ulEntryInstanceNum, ".");

                        snprintf(ucEntryParamName,sizeof(ucEntryParamName), "%s%s", ucEntryFullPath, pParameterName);
               
                        ulEntryNameLen = sizeof(ucEntryNameValue);
                        rc = strcmp_s(ucEntryNameValue,ulEntryNameLen, (char*)pKeyword,&ind);
                        ERR_CHK(rc); 
                        if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                             ( (ind == 0) && (rc == EOK)) )
                        {
                            pMatchedLowerLayer =  AnscCloneString(ucEntryFullPath);

                            break;
                        }
                    }
                }
            }

            if ( pMatchedLowerLayer )
            {
                AnscFreeMemory(pTableStringToken);
                break;
            }
        }

        AnscFreeMemory(pTableStringToken);
    }

    if ( pTableListTokenChain )
    {
        AnscTcFree((ANSC_HANDLE)pTableListTokenChain);
    }

/*
    CcspTraceWarning
        ((
            "CosaUtilGetFullPathNameByKeyword: %s matched parameters(%s) with keyword %s in the table %s(%s)\n",
            pMatchedLowerLayer ? "Found a":"Not find any",
            pMatchedLowerLayer ? pMatchedLowerLayer : "",
            pKeyword,
            pTableName,
            pParameterName
        ));
*/

    return (PUCHAR)pMatchedLowerLayer;
}


ULONG
CosaUtilChannelValidate
    (
        UINT                       uiRadio,
        ULONG                      Channel
    )
{
    unsigned long channelList_5G [] = {36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165};
    int i;
    switch(uiRadio)
    {
        case 1:
             if((Channel < 1) || (Channel > 11))
                return 0;
             return 1;
        case 2:
             for(i=0; i<13; i++)
             {
                if(Channel == channelList_5G[i])
                  return 1;
             }
             return 0;
             break;
        default:
             break;
     }
     return 0;
}

ULONG
CosaUtilChannelValidate2
    (
        UINT                       uiRadio,
        ULONG                      Channel,
        char                       *channelList
    )
{
    // This should be updated to use the possible channels list  Device.WiFi.Radio.1.PossibleChannels instead of a static list.
    unsigned long channelList_5G [] = {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165};
    int i;

    // Channel maybe 0 if radio is disabled or auto channel was set
    if (Channel == 0) {
        return 1;
    }

    // If channelList is provided use it.
    if (channelList != NULL) {
        char chan[4];
        sprintf(chan,"%lu",Channel);
        if (strstr(channelList,chan) != NULL) {
            return 1;
        } 
    }

    switch(uiRadio)
    {
        case 1:
             if(((int)Channel < 0) || (Channel > 11))
                return 0;
             return 1;
        case 2:
             for(i=0; i<24; i++)
             {
                if(Channel == channelList_5G[i])
                  return 1;
             }
             return 0;
             break;
        default:
             break;
     }
     return 0;
}

ULONG CosaUtilIoctlXXX(char * if_name, char * method, void * input)
{
    ULONG ret = 0;
    struct ifreq ifr;
	int sock;
    errno_t rc = -1;
    int ind = -1;

    if (!if_name || !method)
    {
        return 0;
    }
    
	sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0 ) 
    {
        return 0;
    }

    rc = strncpy_s(ifr.ifr_name,sizeof(ifr.ifr_name), if_name, IF_NAMESIZE);
    if(rc != EOK)
    {
        ERR_CHK(rc);
        close(sock);
	return -1;
        
    }

    rc = strcmp_s( "mtu",strlen("mtu"),method,&ind);
    ERR_CHK(rc);
    if((!ind) && (rc == EOK))
    {
        if (ioctl(sock, SIOCGIFMTU, &ifr) == 0)
        {
            ret = ifr.ifr_mtu;
            goto _EXIT;
        }
        else 
        {
            goto _EXIT;
        }
    }
    rc = strcmp_s( "setmtu",strlen("setmtu"),method,&ind);
    ERR_CHK(rc);
    if((!ind) && (rc == EOK))

    {
        ULONG mtu = *(ULONG *)input;
        ifr.ifr_mtu = mtu;

        ret = ioctl(sock, SIOCSIFMTU, &ifr);
        goto _EXIT;
    }
    else
    {
        rc = strcmp_s("status",strlen("status"),method,&ind);
        ERR_CHK(rc);
       if((!ind) && (rc == EOK))
       {
           if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
           {
            ret = ifr.ifr_flags;
            goto _EXIT;

           }
           else 
          {
            goto _EXIT;
          }        
      }
      else
      { 
          rc = strcmp_s( "netmask",strlen("netmask"),method,&ind);
          ERR_CHK(rc);
          if((!ind) && (rc == EOK))
         {
            if (ioctl(sock, SIOCGIFNETMASK, &ifr) == 0)
           {
             rc =  memcpy_s(&ret, sizeof(ret),ifr.ifr_netmask.sa_data + 2,4);
             if(rc != EOK)
            {
              ERR_CHK(rc);
              close(sock);
              return -1;
             }


              goto _EXIT;

           }
           else 
           {
            goto _EXIT;
           }        
         }
         else
         {
             rc = strcmp_s( "set_netmask",strlen("set_netmask"),method,&ind);
             ERR_CHK(rc);
             if((!ind) && (rc == EOK))
             {
                ULONG mask = *(ULONG *)input;

        /*first get netmask then modify it*/
               if (ioctl(sock, SIOCGIFNETMASK, &ifr) == 0)
                {
                 ULONG *pmask = &mask;
                 rc = memcpy_s(ifr.ifr_netmask.sa_data + 2,sizeof(ifr.ifr_netmask.sa_data)-2, pmask, sizeof(mask));
                  if(rc != EOK)
                  {
                    ERR_CHK(rc);
                    close(sock);
                    return -1;
                  }

                 ret = ioctl(sock, SIOCSIFNETMASK, &ifr);
                 goto _EXIT;
                 }
                 else 
                 {
                   ret = -1;
                   goto _EXIT;
                  }        
                }
           }
         }
       } 
_EXIT:
    close(sock);
    return ret;
}



ULONG NetmaskToNumber(char *netmask)
{
    char * pch;
    ULONG val;
    ULONG i;
    ULONG count = 0;

    pch = strtok(netmask, ".");
    while (pch != NULL)
    {
        val = atoi(pch);
        for (i=0;i<8;i++)
        {
            if (val&0x01)
            {
                count++;
            }
            val = val >> 1;
        }
        pch = strtok(NULL,".");
    }
    return count;
}

ANSC_STATUS
CosaUtilGetStaticRouteTable
    (
        UINT                        *count,
        StaticRoute                 **out_sroute
    )
{
    UNREFERENCED_PARAMETER(count);
    UNREFERENCED_PARAMETER(out_sroute);
    return ANSC_STATUS_SUCCESS;

}

#define IPV6_ADDR_LOOPBACK      0x0010U
#define IPV6_ADDR_LINKLOCAL     0x0020U
#define IPV6_ADDR_SITELOCAL     0x0040U
#define IPV6_ADDR_COMPATv4      0x0080U
#define IPV6_ADDR_SCOPE_MASK    0x00f0U

#if  !defined(_COSA_SIM_)

/* function: is_usg_in_bridge_mode
   description: judge if USG is in bridge mode or not
   input: none
   output: TRUE in bridge mode, otherwise FALSE
   return: ANSC_STATUS_SUCCESS/ANSC_STATUS_FAILURE
 */
ANSC_STATUS is_usg_in_bridge_mode(BOOL *pBridgeMode)
{
    ULONG ulEntryNameLen;
    char ucEntryNameValue[256] = {0};
    parameterValStruct_t varStruct;
    errno_t        rc = -1;
    int            ind = -1;

    varStruct.parameterName = "Device.X_CISCO_COM_DeviceControl.LanManagementEntry.1.LanMode";
    varStruct.parameterValue = ucEntryNameValue;

    ulEntryNameLen = sizeof(ucEntryNameValue);
    if (ANSC_STATUS_SUCCESS == COSAGetParamValueByPathName(g_MessageBusHandle,&varStruct,&ulEntryNameLen))
    {
        rc = strcmp_s( "bridge-static",strlen( "bridge-static"),varStruct.parameterValue,&ind);
        ERR_CHK(rc);
        if( (ind == 0) && (rc == EOK))
        {
	     *pBridgeMode = TRUE;
        }
        else{
            *pBridgeMode = FALSE;
        }
	
        return ANSC_STATUS_SUCCESS;
    }
    else{
        return ANSC_STATUS_FAILURE;
    }
    
}

/*caller must free(*pp_info)*/
#define _PROCNET_IFINET6  "/proc/net/if_inet6"
int CosaUtilGetIpv6AddrInfo (char * ifname, ipv6_addr_info_t ** pp_info, int * p_num)
{
    FILE * fp = NULL;
	char addr6p[8][5];
	int plen, scope, dad_status, if_idx;    
	char addr6[40], devname[20];
	struct sockaddr_in6 sap;
        errno_t rc = -1;
        int ind = -1;
    ipv6_addr_info_t * p_ai = NULL;
    int    i = 0;
    
    if (!ifname || !pp_info || !p_num)
        return -1;

    *p_num = 0;

    fp = fopen(_PROCNET_IFINET6, "r");
    if (!fp){
        return -1;
    }
    
	while (fscanf
		   (fp, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n",
			addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
			addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
			&dad_status, devname) != EOF
          )
    {
        rc = strcmp_s(devname,sizeof(devname),ifname,&ind);
        ERR_CHK(rc);
        if((rc == EOK) && (!ind))      
        {
            snprintf(addr6, sizeof(addr6), "%s:%s:%s:%s:%s:%s:%s:%s",
					addr6p[0], addr6p[1], addr6p[2], addr6p[3],
					addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
			inet_pton(AF_INET6, addr6,
					  (struct sockaddr *) &sap.sin6_addr);
			sap.sin6_family = AF_INET6;
            inet_ntop(AF_INET6, (struct sockaddr *) &sap.sin6_addr, addr6, sizeof(addr6));
            
            (*p_num)++;
            *pp_info = realloc(*pp_info,  *p_num * sizeof(ipv6_addr_info_t));
            if (!*pp_info) 
            {
                fclose(fp);
                return -1;
            }

            p_ai = &(*pp_info)[*p_num-1];
            char *pAddr6 = addr6;
            rc = strncpy_s(p_ai->v6addr, sizeof(p_ai->v6addr), pAddr6, sizeof(p_ai->v6addr));
            
            if(rc != EOK)
            {
              ERR_CHK(rc);
              fclose(fp);
              return -1;
             }

            switch (scope & IPV6_ADDR_SCOPE_MASK) {
            case 0:
                p_ai->scope = IPV6_ADDR_SCOPE_GLOBAL;
				break;
			case IPV6_ADDR_LINKLOCAL:
                p_ai->scope = IPV6_ADDR_SCOPE_LINKLOCAL;
				break;
			case IPV6_ADDR_SITELOCAL:
                p_ai->scope = IPV6_ADDR_SCOPE_SITELOCAL;
				break;
			case IPV6_ADDR_COMPATv4:
                p_ai->scope = IPV6_ADDR_SCOPE_COMPATv4;
				break;
			case IPV6_ADDR_LOOPBACK:
                p_ai->scope = IPV6_ADDR_SCOPE_LOOPBACK;
				break;
			default:
                p_ai->scope = IPV6_ADDR_SCOPE_UNKNOWN;
			}
            
           rc =  memset_s(p_ai->v6pre,sizeof(p_ai->v6pre), 0, sizeof(p_ai->v6pre));
           ERR_CHK(rc);
            for (i=0; (i< ( plen%16 ? (plen/16+1):plen/16)) && i<8; i++)
                snprintf(p_ai->v6pre + strlen(p_ai->v6pre), sizeof(p_ai->v6pre) - strlen(p_ai->v6pre), "%s:", addr6p[i]);
            snprintf(p_ai->v6pre+strlen(p_ai->v6pre), sizeof(p_ai->v6pre) - strlen(p_ai->v6pre), ":/%d", plen);
            
        }
    }

    fclose(fp);
    return 0;
}

#else
ANSC_STATUS is_usg_in_bridge_mode(BOOL *pBridgeMode)
{
    *pBridgeMode = FALSE;
    return ANSC_STATUS_SUCCESS;
}

int CosaUtilGetIpv6AddrInfo (char * ifname, ipv6_addr_info_t ** pp_info, int * p_num)
{
    UNREFERENCED_PARAMETER(ifname);
    *p_num = 0;
    *pp_info = NULL;
    return 0;
}
#endif

int  __v6addr_mismatch(char * addr1, char * addr2, int pref_len)
{
    int i = 0;
    int num = 0;
    int mask = 0;
    char addr1_buf[128] = {0};
    char addr2_buf[128] = {0};
    struct in6_addr in6_addr1;
    struct in6_addr in6_addr2;
    errno_t rc = -1;

    if (!addr1 || !addr2)
        return -1;
    
     rc = strcpy_s(addr1_buf,sizeof(addr1_buf),addr1);
     if(rc != EOK)
     {
        ERR_CHK(rc);
	return -1;
    }
    rc = strcpy_s(addr2_buf,sizeof(addr2_buf),addr2);
    if(rc != EOK)
     {
        ERR_CHK(rc);
        return -1;
    }


    if ( inet_pton(AF_INET6, addr1_buf, &in6_addr1) != 1)
        return -8;
    if ( inet_pton(AF_INET6, addr2_buf, &in6_addr2) != 1)
        return -9;

    num = (pref_len%8)? (pref_len/8+1) : pref_len/8;
    if (pref_len%8)
    {
        for (i=0; i<num-1; i++)
            if (in6_addr1.s6_addr[i] != in6_addr2.s6_addr[i])
                return -3;

        for (i=0; i<pref_len%8; i++)
            mask += 1<<(7-i);
        
        if ( (in6_addr1.s6_addr[num-1] &  mask) == (in6_addr2.s6_addr[num-1] & mask))
            return 0;
        else
            return -4;
    }
    else 
    {
        for (i=0; i<num; i++)
            if (in6_addr1.s6_addr[i] != in6_addr2.s6_addr[i])
                return -5;
    }

    return 0;
}

int  __v6addr_mismatches_v6pre(char * v6addr,char * v6pre)
{
    int pref_len = 0;
    char addr_buf[128] = {0};
    char pref_buf[128] = {0};
    char * p = NULL;
    errno_t rc = -1;

    if (!v6addr || !v6pre)
        return -1;
    
    rc = strcpy_s(addr_buf,sizeof(addr_buf),v6addr);
     if(rc != EOK)
     {
        ERR_CHK(rc);
        return -1;
     }   
    rc = strcpy_s(pref_buf,sizeof(pref_buf),v6pre);
    if(rc != EOK)
     {
        ERR_CHK(rc);
        return -1;
    }


    if (!(p = strchr(pref_buf, '/')))
        return -1;

    if (sscanf(p, "/%d", &pref_len) != 1)
        return -2;
    *p = 0;

    return __v6addr_mismatch(addr_buf, pref_buf, pref_len);
}

int  __v6pref_mismatches(char * v6pref1,char * v6pref2)
{
    int pref1_len = 0;
    int pref2_len = 0;
    char pref1_buf[128] = {0};
    char pref2_buf[128] = {0};
    char * p = NULL;
    errno_t rc = -1;
    if (!v6pref1 || !v6pref2)
        return -1;
    
    rc = strcpy_s(pref1_buf,  sizeof(pref1_buf),v6pref1);
      if(rc != EOK)
     {
        ERR_CHK(rc);
        return -1;
     }
    rc = strcpy_s(pref2_buf, sizeof(pref2_buf),v6pref2);
     if(rc != EOK)
     { 
        ERR_CHK(rc);
        return -1;
     }


    if (!(p = strchr(pref1_buf, '/')))
        return -1;

    if (sscanf(p, "/%d", &pref1_len) != 1)
        return -2;
    *p = 0;

    if (!(p = strchr(pref2_buf, '/')))
        return -1;

    if (sscanf(p, "/%d", &pref2_len) != 1)
        return -2;
    *p = 0;

    if (pref1_len != pref2_len)
        return -7;

    return __v6addr_mismatch(pref1_buf, pref2_buf, pref1_len);
}

int CosaDmlV6AddrIsEqual(char * p_addr1, char * p_addr2)
{
    if (!p_addr1 || !p_addr2)
        return 0;

    return !__v6addr_mismatch(p_addr1, p_addr2, 128);
}

int CosaDmlV6PrefIsEqual(char * p_pref1, char * p_pref2)
{
    if (!p_pref1 || !p_pref2)
        return 0;

    return !__v6pref_mismatches(p_pref1, p_pref2);
}

/*
 * IPV4 Address check functions
 */
/* addr is in network order */
int IPv4Addr_IsSameNetwork(uint32_t addr1, uint32_t addr2, uint32_t mask)
{
    return (addr1 & mask) == (addr2 & mask);
}

/* addr is in network order */
int IPv4Addr_IsLoopback(uint32_t addr)
{
    return (addr & htonl(0xff000000)) == htonl(0x7f000000);
}

/* addr is in network order */
int IPv4Addr_IsMulticast(uint32_t addr)
{
    return (addr & htonl(0xf0000000)) == htonl(0xe0000000);
}

/* addr is in network order */
int IPv4Addr_IsBroadcast(uint32_t addr, uint32_t net, uint32_t mask)
{
    /* all ones or all zeros (old) */
    if (addr == 0xffffffff)
        return 1;

    /* on the same sub network and host bits are all ones */
    if (IPv4Addr_IsSameNetwork(addr, net, mask)
            && (addr & ~mask) == (0xffffffff & ~mask))
        return 1;

    return 0;
}

/* addr is in network order */
int IPv4Addr_IsNetworkAddr(uint32_t addr, uint32_t net, uint32_t mask)
{
    if (IPv4Addr_IsSameNetwork(addr, net, mask)
            && (addr & ~mask) == 0)
        return 1;

    return 0;
}

/* addr is in network order */
int IPv4Addr_IsNetmaskValid(uint32_t netmask)
{
    uint32_t mask;
    uint32_t hostorder = ntohl(netmask);

    /* first zero */
    for (mask = 1UL << 31 ; mask != 0; mask >>= 1)
        if ((hostorder & mask) == 0)
            break;

    /* there is no one ? */
    for (; mask != 0; mask >>= 1)
        if ((hostorder & mask) != 0)
            return 0;

    return 1;
}

int IPv4Addr_IsClassA(uint32_t addr)
{
    return IPV4_CLASSA(ntohl(addr));
}

int IPv4Addr_IsClassB(uint32_t addr)
{
    return IPV4_CLASSB(ntohl(addr));
}

int IPv4Addr_IsClassC(uint32_t addr)
{
    return IPV4_CLASSC(ntohl(addr));
}

int IPv4Addr_IsClassD(uint32_t addr)
{
    return IPV4_CLASSD(ntohl(addr));
}

int get_if_hwaddr(const char *ifname, char *mac, size_t size)
{
    int sockfd;
    struct ifreq ifr;
    unsigned char *ptr;

    if (!ifname || !mac || size < sizeof("00:00:00:00:00:00"))
        return -1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) {
        perror("ioctl");
        close(sockfd);
        return -1;
    }

    ptr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    snprintf(mac, size, "%02x:%02x:%02x:%02x:%02x:%02x",
            ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);

    close(sockfd);
    return 0;
}

int IsFileExists(char *file_name)
{
    struct stat file;

    return (stat(file_name, &file));
}

#ifdef IPV4ADDR_TEST

#define NEXTARG    argc--, argv++

enum {
    CMD_ISLOOP,
    CMD_ISMULT,
    CMD_COMP,
    CMD_ISBORAD,
    CMD_ISNET,
};

struct arguments {
    int             cmd;
    struct in_addr  addr;
    struct in_addr  net;
    struct in_addr  mask;
};

static uint32_t InaddrGetU32(struct in_addr *addr)
{
    return (uint32_t)addr->s_addr;
}

static void Usage(void)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "    ./v4addr l ADDR\n");
    fprintf(stderr, "    ./v4addr m ADDR\n");
    fprintf(stderr, "    ./v4addr c ADDR1 ADDR2 MASK\n");
    fprintf(stderr, "    ./v4addr b ADDR NET MASK\n");
    fprintf(stderr, "    ./v4addr n ADDR NET MASK\n");
}

static int ParseArgs(int argc, char *argv[], struct arguments *args)
{
    if (argc < 3) {
        Usage();
        exit(1);
    }
   errno_t rc = -1;
   int ind = -1;

    bzero(args, sizeof(*args));

    NEXTARG;
    rc = strcmp_s("l",strlen("l"),*argv,&ind);
    ERR_CHK(rc);
     if((!ind) && (rc == EOK))
     {
     
        args->cmd = CMD_ISLOOP;

        NEXTARG;
        if (inet_pton(AF_INET, *argv, &args->addr) <= 0) {
            fprintf(stderr, "invalid addr\n");
            exit(1);
        }
    }
    else
    {
        rc = strcmp_s("m",strlen("m"),*argv,&ind);
         ERR_CHK(rc);
        if((!ind) && (rc == EOK))
        {
          args->cmd = CMD_ISMULT;
    
           NEXTARG;
           if (inet_pton(AF_INET, *argv, &args->addr) <= 0) {
            fprintf(stderr, "invalid addr\n");
            exit(1);
           }
        }
   
        else
        {
              const  char  *key[MAX_VALUE] = {"c","b","n"};
              for(i = 0; i < 3; i++)
            {
               rc = strcmp_s(key[i],strlen(key[i]),*argv,&ind);
               ERR_CHK(rc);
               if((rc == EOK) && (!ind))
               {
                 if (i == 0)
                 {
                   args->cmd = CMD_COMP;
                   break;
                 }
                  else if (i == 1)
                  {
                     args->cmd = CMD_ISBORAD;
                     break;
                  } 
                    else if (i == 2)
                   {
                       args->cmd = CMD_ISNET;
                       break;
                   }
                   
                  }
                }
                     
        NEXTARG;
        if (inet_pton(AF_INET, *argv, &args->addr) <= 0) {
            fprintf(stderr, "invalid addr\n");
            exit(1);
        }

        NEXTARG;
        if (inet_pton(AF_INET, *argv, &args->net) <= 0) {
            fprintf(stderr, "invalid addr2/net\n");
            exit(1);
        }

        NEXTARG;
        if (inet_pton(AF_INET, *argv, &args->mask) <= 0
                || !IPv4Addr_IsNetmaskValid(InaddrGetU32(&args->mask))) {
            fprintf(stderr, "invalid mask\n");
            exit(1);
          }
        
       }
}   

     else {
        fprintf(stderr, "unknow command `%s'\n", *argv);
        exit(1);
      }

    return 0;
}

int main(int argc, char *argv[])
{
    struct arguments args;

    if (ParseArgs(argc, argv, &args) != 0)
        exit(1);

    switch (args.cmd) {
    case CMD_ISLOOP:
        if (IPv4Addr_IsLoopback(InaddrGetU32(&args.addr)))
            CcspTraceInfo(("Is loopback\n"));
        else
            CcspTraceInfo(("Not loopback\n"));
        break;

    case CMD_ISMULT:
        if (IPv4Addr_IsMulticast(InaddrGetU32(&args.addr)))
            CcspTraceInfo(("Is multicast\n"));
        else
            CcspTraceInfo(("Not multicast\n"));
        break;

    case CMD_COMP:
        if (IPv4Addr_IsSameNetwork(InaddrGetU32(&args.addr), 
                    InaddrGetU32(&args.net), InaddrGetU32(&args.mask)))
            CcspTraceInfo(("Is in same network\n"));
        else
            CcspTraceInfo(("Not in same network\n"));
        break;

    case CMD_ISBORAD:
        if (IPv4Addr_IsBroadcast(InaddrGetU32(&args.addr), 
                    InaddrGetU32(&args.net), InaddrGetU32(&args.mask)))
            CcspTraceInfo(("Is broadcast\n"));
        else
            CcspTraceInfo(("Not broadcast\n"));
        break;

    case CMD_ISNET:
        if (IPv4Addr_IsNetworkAddr(InaddrGetU32(&args.addr), 
                    InaddrGetU32(&args.net), InaddrGetU32(&args.mask)))
            CcspTraceInfo(("Is network address\n"));
        else
            CcspTraceInfo(("Not network address\n"));
        break;

    default:
        Usage();
        exit(1);
    }

    return 0;
}

#endif
