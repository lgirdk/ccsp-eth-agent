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

    module: cosa_ethernet_internal.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  CosaEthernetCreate
        *  CosaEthernetInitialize
        *  CosaEthernetRemove
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

#include "cosa_ethernet_internal.h"
#include "cosa_ethernet_apis.h"
#include "ccsp_trace.h"
#include <sys/time.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include "safec_lib_common.h"
#include <syscfg/syscfg.h>
#include "secure_wrapper.h"
#if defined (FEATURE_RDKB_WAN_MANAGER) || defined (FEATURE_RDKB_WAN_AGENT)
#if !defined(WAN_MANAGER_UNIFICATION_ENABLED)
#include "cosa_ethernet_manager.h"
#endif
#endif //#if defined (FEATURE_RDKB_WAN_MANAGER)

#if defined (WAN_FAILOVER_SUPPORTED)  ||  defined(RBUS_BUILD_FLAG_ENABLE) || defined (_HUB4_PRODUCT_REQ_) || defined(_PLATFORM_RASPBERRYPI_)
#include "cosa_rbus_handler_apis.h"
#endif

extern void * g_pDslhDmlAgent;
extern ANSC_HANDLE g_EthObject;

ANSC_STATUS
CosaDmlEthGetLogStatus
    (
        PCOSA_DML_ETH_LOG_STATUS  pMyObject
    );

ANSC_STATUS
CosaDmlEthWanGetCfg
    (
        PCOSA_DATAMODEL_ETH_WAN_AGENT  pMyObject
    );

#define LENGTH_MAC_ADDRESS              (18)
#define LENGTH_DELIMITER                (2)

static void CosaEthernetLogger(void);
static int CosaEthTelemetryInit(void);

/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE CosaEthInterfaceCreate ( );

    description:

        This function constructs cosa Ethernet object and return handle.

    argument:

    return:     newly created Ethernet object.

**********************************************************************/
ANSC_HANDLE CosaEthInterfaceCreate ( VOID )
{
    PCOSA_DATAMODEL_ETHERNET    pMyObject    = (PCOSA_DATAMODEL_ETHERNET)NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PCOSA_DATAMODEL_ETHERNET)AnscAllocateMemory(sizeof(COSA_DATAMODEL_ETHERNET));

    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    pMyObject->Oid               = COSA_DATAMODEL_ETHERNET_OID;
    pMyObject->Create            = CosaEthInterfaceCreate;
    pMyObject->Remove            = CosaEthInterfaceRemove;
    pMyObject->Initialize        = CosaEthInterfaceInitialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS CosaEthInterfaceInitialize ( ANSC_HANDLE hThisObject );

    description:

        This function initiate  cosa Ethernet object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS CosaEthInterfaceInitialize ( ANSC_HANDLE hThisObject )
{
    ANSC_STATUS                     returnStatus        = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_ETHERNET        pMyObject           = (PCOSA_DATAMODEL_ETHERNET)hThisObject;
    ULONG                           ulEntryCount        = 0;
    ULONG                           ulIndex             = 0;
    ULONG                           ulNextInsNum        = 0;
    errno_t                         rc                  = -1;
    /* Initiation all functions */
    CosaDmlEthInterfaceInit(NULL, &pMyObject->hSbContext);

    _ansc_memset(pMyObject->EthernetPortFullTable, 0, sizeof(COSA_DML_ETH_PORT_FULL) * MAXINSTANCE);

    /* Initiation Device.Ethernet.Interface */

    ulEntryCount = CosaDmlEthPortGetNumberOfEntries(NULL);

    ulNextInsNum = 1;

    for ( ulIndex = 0; ulIndex < ulEntryCount; ulIndex++ )
    {
        CosaDmlEthPortGetEntry(NULL, ulIndex, &pMyObject->EthernetPortFullTable[ulIndex]);

        if ( pMyObject->EthernetPortFullTable[ulIndex].Cfg.InstanceNumber == 0 )
        {
           pMyObject->EthernetPortFullTable[ulIndex].Cfg.InstanceNumber = ulNextInsNum;
                                                                                                                                                                        rc = sprintf_s(pMyObject->EthernetPortFullTable[ulIndex].Cfg.Alias, sizeof(pMyObject->EthernetPortFullTable[ulIndex].Cfg.Alias),"Interface%d", (int)ulNextInsNum);
            if(rc < EOK)
            {
              ERR_CHK(rc);
              return ANSC_STATUS_FAILURE;
            }

            CosaDmlEthPortSetValues(NULL, ulIndex, ulNextInsNum, pMyObject->EthernetPortFullTable[ulIndex].Cfg.Alias);

            ulNextInsNum++;
        }
        else
        {
            ulNextInsNum = pMyObject->EthernetPortFullTable[ulIndex].Cfg.InstanceNumber + 1;
        }
    }
    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        CosaEthInterfaceRemove ( ANSC_HANDLE hThisObject );

    description:

        This function initiate  cosa Ethernet object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS CosaEthInterfaceRemove ( ANSC_HANDLE hThisObject )
{
    ANSC_STATUS                     returnStatus            = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_ETHERNET        pMyObject               = (PCOSA_DATAMODEL_ETHERNET)hThisObject;

    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);

    return returnStatus;
}

ANSC_STATUS CosaEthPortGetAssocDevices ( UCHAR *mac, CHAR *maclist, int numMacAddr )
{
    int     i = 0,j = 0;
    char macAddr[MACADDR_SZ+1];
    errno_t rc = -1;

    maclist[0] = '\0';

    for(j=0; j < numMacAddr; j++)
    {
        memset(macAddr,0,(MACADDR_SZ+1));
        if(i > 0)
            strncat(maclist, ",", sizeof(maclist) - strlen(maclist) - 1);
        rc = sprintf_s(macAddr, sizeof(macAddr),"%02x:%02x:%02x:%02x:%02x:%02x", mac[i], mac[i+1], mac[i+2], mac[i+3], mac[i+4], mac[i+5]);
        if(rc < EOK)
        {
          ERR_CHK(rc);
          return ANSC_STATUS_FAILURE;
        }
        strncat(maclist,macAddr, strlen(macAddr) - strlen(maclist) - 1);
        i += MAC_SZ;
    }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE
        CosaEthernetCreate
            (
            );

    description:

        This function constructs cosa Ethernet object and return handle.

    argument:  

    return:     newly created Ethernet object.

**********************************************************************/

ANSC_HANDLE
CosaEthernetCreate
    (
        VOID
    )
{
    PCOSA_DATAMODEL_ETHERNET    pMyObject    = (PCOSA_DATAMODEL_ETHERNET)NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PCOSA_DATAMODEL_ETHERNET)AnscAllocateMemory(sizeof(COSA_DATAMODEL_ETHERNET));

    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    pMyObject->Oid               = COSA_DATAMODEL_ETHERNET_OID;
    pMyObject->Create            = CosaEthernetCreate;
    pMyObject->Remove            = CosaEthernetRemove;
    pMyObject->Initialize        = CosaEthernetInitialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        CosaEthernetInitialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa Ethernet object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
CosaEthernetInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus        = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_ETHERNET        pMyObject           = (PCOSA_DATAMODEL_ETHERNET)hThisObject;
#if defined (FEATURE_RDKB_WAN_MANAGER)
    AnscSListInitializeHeader( &pMyObject->Q_EthList );
    pMyObject->ulPtNextInstanceNumber   = 1;
#endif //FEATURE_RDKB_WAN_MANAGER

    CosaDmlEthGetLogStatus(&pMyObject->LogStatus);
    CosaEthernetLogger();
    CosaDmlEthWanGetCfg(&pMyObject->EthWanCfg);

#if defined(FEATURE_RDKB_WAN_MANAGER)

#if defined (WAN_FAILOVER_SUPPORTED) || defined(RBUS_BUILD_FLAG_ENABLE) || defined (_HUB4_PRODUCT_REQ_) || defined(_PLATFORM_RASPBERRYPI_)
	ethAgentRbusInit();
#endif

    //Register callbacks with ethsw-hal for link events, so hal can update initial link status if needed
    appCallBack obj;
    memset (&obj, 0, sizeof(appCallBack));
    obj.pGWP_act_EthWanLinkDown = EthWanLinkDown_callback;
    obj.pGWP_act_EthWanLinkUP   = EthWanLinkUp_callback;
    GWP_RegisterEthWan_Callback ( &obj );
    //Initialise global data and initalise hal
    CosaDmlEthInit(NULL, (PANSC_HANDLE)pMyObject);

#elif defined(FEATURE_RDKB_WAN_AGENT)
    CosaDmlEthInit(NULL, (PANSC_HANDLE)pMyObject);
    CcspHalEthSw_RegisterLinkEventCallback(CosaDmlEthPortLinkStatusCallback); //Register cb for link event.
#endif
    CcspHalExtSw_ethAssociatedDevice_callback_register(CosaDmlEth_AssociatedDevice_callback);
    CosaEthTelemetryxOpsLogSettingsSync();
    if (CosaEthTelemetryInit() < 0) {
        CcspTraceError(("RDK_LOG_ERROR, CcspEth %s : CosaEthTelemetryInit create Error!!!\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        CosaEthernetRemove
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa Ethernet object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
CosaEthernetRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus            = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_ETHERNET        pMyObject               = (PCOSA_DATAMODEL_ETHERNET)hThisObject;

    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);

    return returnStatus;
}

static int ethGetClientsCount
    (
        CCSP_HAL_ETHSW_PORT PortId,
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
        if ((int)PortId == eth_device[idx].eth_port)
        {
            count_client++;
        }
    }

    return count_client;
}

static void ethGetClientMacDetails
    (
        CCSP_HAL_ETHSW_PORT PortId,
        LONG num_eth_device,
        eth_device_t *eth_device,
        int total_client,
        char *mac,
        int mem_size
    )
{
    UNREFERENCED_PARAMETER(total_client);
    int idx;
    char mac_addr[20];
    char isFirst = TRUE;
    errno_t rc = -1;

    if (!eth_device || !mac)
    {
        CcspTraceWarning(("ethGetClientMacDetails Invalid input Param\n"));
        return;
    }

    for (idx = 0; idx < num_eth_device; idx++)
    {
        if ((int)PortId == eth_device[idx].eth_port)
        {
            rc = memset_s(mac_addr,sizeof(mac_addr), 0,sizeof(mac_addr) );
            ERR_CHK(rc);
            if (isFirst)
            {
                sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
                        eth_device[idx].eth_devMacAddress[0],
                        eth_device[idx].eth_devMacAddress[1],
                        eth_device[idx].eth_devMacAddress[2],
                        eth_device[idx].eth_devMacAddress[3],
                        eth_device[idx].eth_devMacAddress[4],
                        eth_device[idx].eth_devMacAddress[5]);
                isFirst = FALSE;
            }
            else
            {
                sprintf(mac_addr, ", %02X:%02X:%02X:%02X:%02X:%02X",
                        eth_device[idx].eth_devMacAddress[0],
                        eth_device[idx].eth_devMacAddress[1],
                        eth_device[idx].eth_devMacAddress[2],
                        eth_device[idx].eth_devMacAddress[3],
                        eth_device[idx].eth_devMacAddress[4],
                        eth_device[idx].eth_devMacAddress[5]);
            }
             char *pMacaddr = mac_addr;
             rc = strcat_s(mac,(unsigned int)mem_size, pMacaddr);
             ERR_CHK(rc);
        }
    }
}

void Ethernet_Log(void)
{
    ULONG total_eth_device = 0;
    eth_device_t *output_struct = NULL;
    int i;
    int mem_size = 0;
    int total_port = 0;
    int count_client = 0;
    char *mac_address = NULL;
    int ret = ANSC_STATUS_FAILURE;

#if defined(_CBR_PRODUCT_REQ_)
    total_port = 8;
#elif defined(_XB6_PRODUCT_REQ_) && !defined(_XB7_PRODUCT_REQ_)
    total_port = 2;
#else
    total_port = 4;
#endif

    ret = CcspHalExtSw_getAssociatedDevice(&total_eth_device, &output_struct);
    if (ANSC_STATUS_SUCCESS != ret)
    {
        return;
    }

    //Port number start from 1
    for (i = 1; i <= total_port; i++)
    {
        count_client = ethGetClientsCount(i, total_eth_device, output_struct);
        CcspTraceWarning(("ETH_MAC_%d_TOTAL_COUNT:%d\n", i, count_client));
        if (count_client)
        {
            mem_size = (LENGTH_MAC_ADDRESS + LENGTH_DELIMITER) * count_client;
            mac_address = (char *)AnscAllocateMemory(mem_size);
            if (mac_address)
            {
                ethGetClientMacDetails(
                        i,
                        total_eth_device,
                        output_struct,
                        count_client,
                        mac_address,
                        mem_size );
                CcspTraceWarning(("ETH_MAC_%d:%s\n", i, mac_address));
                CcspTraceWarning(("ETH_PHYRATE_%d:%d\n", i, ethGetPHYRate(i)));

                AnscFreeMemory(mac_address);
                mac_address= NULL;
            }
            else
            {
                CcspTraceWarning(("Alloc Failed\n"));
            }
        }
    }

    if (output_struct)
    {
        AnscFreeMemory(output_struct);
        output_struct= NULL;
    }
}

void * Ethernet_Logger_Thread(void *data)
{
    UNREFERENCED_PARAMETER(data);
    LONG timeleft;
    ULONG Log_Period_old;

    pthread_detach(pthread_self());
    sleep(60);

    while(!g_EthObject)
        sleep(10);

    volatile PCOSA_DATAMODEL_ETHERNET pMyObject = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;

    while (1)
    {
        if (pMyObject->LogStatus.Log_Enable)
            Ethernet_Log();

        timeleft = pMyObject->LogStatus.Log_Period;
        while (timeleft > 0)
        {
			Log_Period_old = pMyObject->LogStatus.Log_Period;
			sleep(60);
			timeleft = timeleft - 60 + pMyObject->LogStatus.Log_Period - Log_Period_old;
        }
    }
}

static void CosaEthernetLogger(void)
{
    pthread_t logger_tid;
    int res;

    res = pthread_create(&logger_tid, NULL, Ethernet_Logger_Thread, NULL);
    if (res != 0) 
    {
        AnscTraceWarning(("CosaEthernetInitialize Create Ethernet_Logger_Thread error %d\n", res));
    }
    else
    {
        AnscTraceWarning(("CosaEthernetInitialize Ethernet_Logger_Thread Created\n"));
    }
}

#define ETH_LOG_FILE "/tmp/eth_telemetry_xOpsLogSettings.txt"
#define BUF_LEN (10 * (sizeof(struct inotify_event) + 255 + 1))

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static const char *eth_telemetry_log = "/rdklogs/logs/eth_telemetry.txt";

static unsigned int gEthLogInterval;
static char gEthLogEnable[7] = {0};

static char *get_formatted_time(char *time)
{
    struct tm *tm_info;
    struct timeval tv_now;
    char tmp[128];

    gettimeofday(&tv_now, NULL);
    tm_info = localtime(&tv_now.tv_sec);

    strftime(tmp, 128, "%y%m%d-%T", tm_info);

    if( snprintf(time, 128, "%s.%06d", tmp, (int)tv_now.tv_usec) >= 128) {
        CcspTraceWarning(( "%s Truncation while formatting time, expected_time:%s.%06d current_time:%s\n", __FUNCTION__ , tmp, (int)tv_now.tv_usec, time));
    }
    return time;
}

static void write_to_file(const char *file_name, char *fmt, ...)
{
    FILE *fp = NULL;
    va_list args;

    fp = fopen(file_name, "a+");
    if (fp == NULL) {
        return;
    }

    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);

    fflush(fp);
    fclose(fp);
}

static void read_updated_log_interval()
{
    FILE *fp = NULL;
    char buff[256] = {0}, tmp[64] = {0};
    fp = fopen(ETH_LOG_FILE, "r");
    errno_t rc = -1;
    if (fp == NULL) {
        CcspTraceError(("%s  %s file open error!!!\n", __func__, ETH_LOG_FILE));
	return;
    }
    if (fscanf(fp, "%d,%6s", &gEthLogInterval, gEthLogEnable) != 2)
    {
        CcspTraceError(("%s %d: unable to get log info\n", __func__, __LINE__));
    }
    fclose(fp);

    rc =  memset_s(tmp,sizeof(tmp),0,sizeof(tmp));
    ERR_CHK(rc);
    get_formatted_time(tmp);
    rc =  memset_s(buff,sizeof(buff),0,sizeof(buff));
    ERR_CHK(rc);
    snprintf(buff, 256, "%s ETH_TELEMETRY_LOG_PERIOD:%d\n", tmp, gEthLogInterval);
    write_to_file(eth_telemetry_log, buff);
    
    rc =  memset_s(tmp,sizeof(tmp),0,sizeof(tmp));
    ERR_CHK(rc);
    get_formatted_time(tmp);
    rc =  memset_s(buff,sizeof(buff),0,sizeof(buff));
    ERR_CHK(rc);
    snprintf(buff, 256, "%s ETH_TELEMETRY_LOG_ENABLED:%s\n", tmp, gEthLogEnable);
    write_to_file(eth_telemetry_log, buff);
}

static void EthTelemetryPush()
{
    ULONG total_eth_device = 0;
    eth_device_t *output_struct = NULL;
    int i;
    int mem_size = 0;
    int total_port = 0;
    int count_client = 0;
    char *mac_address = NULL;
    int ret = ANSC_STATUS_FAILURE;
    char tmp[128] = {0}, buff[2048] = {0};
    errno_t rc = -1;

#if defined(_CBR_PRODUCT_REQ_)
    total_port = 8;
#elif defined(_XB6_PRODUCT_REQ_) && !defined(_XB7_PRODUCT_REQ_)
    total_port = 2;
#else
    total_port = 4;
#endif

    ret = CcspHalExtSw_getAssociatedDevice(&total_eth_device, &output_struct);
    if (ANSC_STATUS_SUCCESS != ret) {
        return;
    }

    //Port number start from 1
    for (i = 1; i <= total_port; i++)
    {
	count_client = ethGetClientsCount(i, total_eth_device, output_struct);
	rc =  memset_s(tmp,sizeof(tmp),0,sizeof(tmp));
        ERR_CHK(rc);
	get_formatted_time(tmp);
	rc =  memset_s(buff,sizeof(buff),0,sizeof(buff));
        ERR_CHK(rc);
	snprintf(buff, 2048, "%s ETH_MAC_%d_TOTAL_COUNT:%d\n", tmp, i, count_client);
	write_to_file(eth_telemetry_log, buff);
	if (count_client)
	{
	    mem_size = (LENGTH_MAC_ADDRESS + LENGTH_DELIMITER) * count_client;
	    mac_address = (char *)AnscAllocateMemory(mem_size);
	    if (mac_address)
	    {
		ethGetClientMacDetails(
			i,
			total_eth_device,
			output_struct,
			count_client,
			mac_address,
                        mem_size);
	        rc =  memset_s(tmp,sizeof(tmp),0,sizeof(tmp));
                ERR_CHK(rc);
		get_formatted_time(tmp);
		rc =  memset_s(buff,sizeof(buff),0,sizeof(buff));
                ERR_CHK(rc);
		snprintf(buff, 2048, "%s ETH_MAC_%d:%s\n", tmp, i, mac_address);
		write_to_file(eth_telemetry_log, buff);
		rc =  memset_s(tmp,sizeof(tmp),0,sizeof(tmp));
                ERR_CHK(rc);
		get_formatted_time(tmp);
		rc = memset_s(buff,sizeof(buff),0,sizeof(buff));
                ERR_CHK(rc);
		snprintf(buff, 2048, "%s ETH_PHYRATE_%d:%d\n", tmp, i, ethGetPHYRate(i));
		write_to_file(eth_telemetry_log, buff);

		AnscFreeMemory(mac_address);
		mac_address= NULL;
	    }
	    else
	    {
		CcspTraceWarning(("Alloc Failed\n"));
	    }
        }
    }

    if (output_struct) {
        AnscFreeMemory(output_struct);
        output_struct= NULL;
    }
}

void*  EthTelemetryxOpsLogSettingsEventThread()
{
    struct inotify_event *event;
    int inotifyFd, numRead;
    char eth_log_buf[BUF_LEN] __attribute__ ((aligned(8))) = {0}, *ptr = NULL;

    while (!(IsFileExists(ETH_LOG_FILE) == 0)) {
        CcspTraceError(("%s  %s file not found wait for 5 seconds and check \n", __func__, ETH_LOG_FILE));
        sleep(5);
    }

    read_updated_log_interval();

    inotifyFd = inotify_init();
    inotify_add_watch(inotifyFd, ETH_LOG_FILE, IN_MODIFY);

    for (;;)
    {
        numRead = read(inotifyFd, eth_log_buf, BUF_LEN);
        if (numRead < 0) {
            CcspTraceError(("%s Error returned is = numRead %d  %d\n", __func__, numRead, __LINE__));
        }

        /* Process all of the events in buffer returned by read() */
        for (ptr = eth_log_buf; ptr < eth_log_buf + numRead; )
        {
            event = (struct inotify_event *) ptr;
            if (event->mask & IN_MODIFY)
            {
		read_updated_log_interval();
		pthread_cond_signal(&cond);
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }

    }
}

void* EthTelemetryLoggingThread()
{
    struct timeval now;
    struct timespec t;
    int rc = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    while (1) { 
	gettimeofday(&now,NULL);
	t.tv_nsec = 0;
	t.tv_sec = now.tv_sec + gEthLogInterval;
	pthread_mutex_lock(&mutex);
	rc = pthread_cond_timedwait(&cond, &mutex, &t);
	pthread_mutex_unlock(&mutex);
	if (rc == ETIMEDOUT) {
	    if (strncmp(gEthLogEnable, "true", 5) == 0) {
		EthTelemetryPush();
	    }
	}
    }
}

int CosaEthTelemetryInit()
{
    int i = 0;
    pthread_t tid[2];
    void *(*EthTelemetryThread[2])() = {EthTelemetryxOpsLogSettingsEventThread, EthTelemetryLoggingThread};
    pthread_attr_t tattr;

    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

    for (i = 0; i < 2; i++) {
	int err = pthread_create(&tid[i], &tattr, EthTelemetryThread[i], NULL);
	if (0 != err)
	{
	    CcspTraceError(("%s: Error creating the Eth telemetry thread %d!\n", __FUNCTION__, __LINE__));
	    pthread_attr_destroy( &tattr );
	    return -1;
	}
    }
    pthread_attr_destroy( &tattr );

    return ANSC_STATUS_SUCCESS;
}

#if defined (FEATURE_RDKB_WAN_MANAGER)
ANSC_STATUS InitEthIfaceEntry(ANSC_HANDLE hDml, PCOSA_DML_ETH_PORT_CONFIG pEntry)
{
    UNREFERENCED_PARAMETER(hDml);
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_ETHERNET    pEthernet = (PCOSA_DATAMODEL_ETHERNET)g_EthObject;
    /*
        For dynamic and writable table, we don't keep the Maximum InstanceNumber.
        If there is delay_added entry, we just jump that InstanceNumber.
    */
    do
    {
        if ( pEthernet->ulPtNextInstanceNumber == 0 )
        {
            pEthernet->ulPtNextInstanceNumber   = 1;
        }

        if ( !CosaSListGetEntryByInsNum(&pEthernet->Q_EthList, pEthernet->ulPtNextInstanceNumber) )
        {
            break;
        }
        else
        {
            pEthernet->ulPtNextInstanceNumber++;
        }
    }while(1);

    pEntry->ulInstanceNumber            = pEthernet->ulPtNextInstanceNumber;

    DML_ETHIF_INIT(pEntry);

    _ansc_sprintf( pEntry->Path, "Device.Ethernet.X_RDK_Interface.%lu", pEntry->ulInstanceNumber );
    _ansc_sprintf( pEntry->LowerLayers, "Device.Ethernet.X_RDK_Interface.%lu", pEntry->ulInstanceNumber );
    _ansc_sprintf( pEntry->Name, "eth%lu", pEntry->ulInstanceNumber - 1 );

    pEthernet->ulPtNextInstanceNumber++;

    return returnStatus;
}
#endif // FEATURE_RDKB_WAN_MANAGER

#ifdef FEATURE_RDKB_WAN_UPSTREAM
ANSC_STATUS CosaDmlSetWanOEMode (PCOSA_DML_ETH_PORT_FULL pEthernetPortFull,PCOSA_DML_ETH_PORT_CONFIG pEthLink)
{
    BOOL bUpstream = FALSE;
    CHAR ifName[BUFLEN_32] = {0};
    if (pEthernetPortFull == NULL && pEthLink != NULL)
    {
        /* Ethernet.X_RDK_Interface. */
        snprintf(ifName, sizeof(ifName), "%s", pEthLink->Name);
        bUpstream = pEthLink->Upstream;
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
        COSA_DML_ETH_PORT_CAP_OPER ePortCapability = pEthLink->PortCapability;

        AnscTraceInfo(("[%s][%d] ifName[%s],ePortCapability[%d],bUpstream[%d]\n",__FUNCTION__,  __LINE__, ifName,ePortCapability,bUpstream));
        if(ePortCapability == PORT_CAP_WAN_LAN)
        {
            if(0 != CcspHalExtSw_ethPortConfigure(ifName,bUpstream))
            {
                AnscTraceError(("[%s][%d] CcspHalExtSw_ethPortConfigure[Wan_lan Failed]\n",__FUNCTION__,  __LINE__));
                return ANSC_STATUS_FAILURE;
            }
            return ANSC_STATUS_SUCCESS;
        }
        else if(pEthLink->PortCapability == PORT_CAP_WAN )
        {
            if(0 != CcspHalExtSw_ethPortConfigure(ifName,TRUE))
            {
                AnscTraceError(("[%s][%d] CcspHalExtSw_ethPortConfigure[Wan Failed]\n",__FUNCTION__,  __LINE__));
                return ANSC_STATUS_FAILURE;
            }
            return ANSC_STATUS_SUCCESS;
        }
        else
        {
            return ANSC_STATUS_FAILURE;
        }
#endif
    }
    else if(pEthernetPortFull != NULL && pEthLink == NULL)
    {
        /* Ethernet.Interface. */
        bUpstream = pEthernetPortFull->StaticInfo.bUpstream;
        snprintf(ifName, sizeof(ifName), "%s", pEthernetPortFull->StaticInfo.Name);

        AnscTraceInfo(("[%s][%d] ifName[%s],bUpstream[%d]\n",__FUNCTION__,  __LINE__, ifName,bUpstream));
        if(0!=CcspHalExtSw_setEthWanEnable(bUpstream))
        {
            AnscTraceError(("[%s][%d] CcspHalExtSw_setEthWanEnable[Failed]\n",__FUNCTION__,  __LINE__));
            return ANSC_STATUS_FAILURE;
        }

        /** Update local storage with new value. **/
        if (syscfg_set_u_commit(NULL, "Ethwan_Disable_Upstream", !bUpstream) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
            return ANSC_STATUS_FAILURE;
        }
        return ANSC_STATUS_SUCCESS;
    }
    else
    {
        AnscTraceError(("[%s][%d] Invalid Input! \n",__FUNCTION__, __LINE__));
    }
    return ANSC_STATUS_FAILURE;
}
#endif
