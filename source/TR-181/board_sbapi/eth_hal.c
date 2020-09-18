/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

    module: eth_hal.c

        For CCSP Component:  ccsp-ethagent

    description:

        This sample implementation file gives the function call prototypes and
        structure definitions used for the RDK-Broadband
        Ethernet Agent hardware abstraction layer


    ---------------------------------------------------------------

    environment:

        This HAL layer is intended to support Ethernet drivers
        through an open API.

    ---------------------------------------------------------------

**********************************************************************/
#if defined (FEATURE_RDKB_WAN_MANAGER)
/*****************************************************************************
* STANDARD INCLUDE FILES
*****************************************************************************/
#include <json-c/json.h>
#include <limits.h>

/*****************************************************************************
* PROJECT-SPECIFIC INCLUDE FILES
*****************************************************************************/
#include "eth_hal.h"
#include "json_hal_client.h"
#include "ansc_platform.h"
/***************************************************************************************
* GLOBAL SYMBOLS
****************************************************************************************/
static void *eventcb(const char *msg, const int len);

/* Callback function for EthAgent.*/
ethsw_ethLinkEventCallback eth_link_event_cb_for_eth_agent = NULL; //Callback function for EthAgent.

/****************************************************************************************/
#define ETH_JSON_CONF_PATH "/etc/rdk/conf/eth_manager_conf.json"

#define JSON_RPC_FIELD_PARAMS "params"

#define HAL_CONNECTION_RETRY_MAX_COUNT 10
#define ETH_LINKSTATUS  "Device.Ethernet.X_RDK_Interface.4.Status"

#define CHECK(expr)                                                \
    if (!(expr))                                                   \
    {                                                              \
        CcspTraceError(("%s - %d Invalid parameter error \n!!!")); \
        return RETURN_ERR;                                         \
    }

#define FREE_JSON_OBJECT(expr) \
    if(expr)                   \
    {                          \
        json_object_put(expr); \
    }

/***************Static API definitions***********************************/

/* Indicates client connected or not. */
static int is_client_connected;

/**
 * @brief send subscription json request for eth-link event.
 */
static int subscribe_eth_link_event();
/************************************************************************/

/* Intializer. */
/* eth_hal_init :  */
/**
* Description: Do what needed to intialize the Eth hal.
* Parameters : None
*
* @return The status of the operation.
* @retval RETURN_OK if successful.
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous.
* @sideeffect None.

*
* @note This function must not suspend and must not invoke any blocking system 
* calls. It should probably just send a message to a driver event handler task. 
*
*/
INT eth_hal_init()
{
    int rc = RETURN_OK;
    rc = json_hal_client_init(ETH_JSON_CONF_PATH);
    if (rc != RETURN_OK)
    {
        CcspTraceError(("%s-%d Failed to initialize hal client. \n",__FUNCTION__,__LINE__));
        return RETURN_ERR;
    }

    rc = json_hal_client_run();
    if (rc != RETURN_OK)
    {
        CcspTraceError(("%s-%d Failed to start hal client. \n", __FUNCTION__,__LINE__));
        return RETURN_ERR;
    }
    /**
     * Make sure HAL client connected to server.
     * Here waits for 10seconds time to check whether connection established or not.
     * If connection not established within this time, returned error.
     */
    int retry_count = 0;
    int is_client_connected = 0;
    while (retry_count < HAL_CONNECTION_RETRY_MAX_COUNT)
    {
        if (!json_hal_is_client_connected())
        {
            sleep(1);
            retry_count++;
        }
        else
        {
            CcspTraceError(("%s-%d Hal-client connected to the hal server \n", __FUNCTION__, __LINE__));
            is_client_connected = TRUE;
            break;
        }
    }

    if (is_client_connected != TRUE)
    {
        CcspTraceError(("Failed to connect to the hal server. \n"));
        return RETURN_ERR;
    }

    /* Event subscription for eth link */
    rc = subscribe_eth_link_event();

    if (rc != RETURN_OK)
    {
        CcspTraceError(("Failed to subscribe ETH link event \n"));
    }

    return rc;
}


/*==========================================================================================
 * @brief void eth_hal_registerLinkEventCallback()
 * @description This is callback register function which will register the callback function
 * requested from EthAgent.
 *========================================================================================== */
void eth_hal_registerLinkEventCallback(ethsw_ethLinkEventCallback callback_proc)
{
    eth_link_event_cb_for_eth_agent = callback_proc;
    CcspTraceError(("eth_hal_registerLinkEventCallback being called \n"));
}

static void *eventcb(const char *msg, const int len)
{
    json_object *msg_param = NULL;
    json_object *msg_param_val = NULL;
    char event_name[256] = {'\0'};
    char event_val[256] = {'\0'};

    if(msg == NULL) {
        return;
    }

    /* Parse message and find event received. */
    json_object *jobj = json_tokener_parse(msg);
    CHECK(jobj);

    if (json_object_object_get_ex(jobj, JSON_RPC_FIELD_PARAMS, &msg_param))
    {
        json_object *jsubs_param_array = json_object_array_get_idx(msg_param, 0);
        if (jsubs_param_array == NULL)
        {
            CcspTraceError(("Failed to get params data from subscription json message \n"));
            FREE_JSON_OBJECT(jobj);
            return;
        }

        if (json_object_object_get_ex(jsubs_param_array, "name", &msg_param_val))
        {
            strcpy(event_name, json_object_get_string(msg_param_val));
            CcspTraceError(("Event name = %s \n", event_name));
        }
        else
        {
            CcspTraceError(("Failed to get event name data from subscription json message \n"));
            FREE_JSON_OBJECT(jobj);
            return;
        }

        if (json_object_object_get_ex(jsubs_param_array, "value", &msg_param_val))
        {
            strcpy(event_val, json_object_get_string(msg_param_val));
            CcspTraceError(("Event value = %s \n", event_val));
        }
        else
        {
            CcspTraceError(("Failed to get event value data from subscription json message \n"));
            FREE_JSON_OBJECT(jobj);
            return;
        }
    }

    if (strncmp(event_name, ETH_LINKSTATUS, sizeof(ETH_LINKSTATUS)) == 0)
    {
        CcspTraceError(("Event got for %s and its value =%s \n", event_name, event_val));
        if (eth_link_event_cb_for_eth_agent)
        {
            CcspTraceError(("Notifying EthAgent for the link event \n"));
            eth_link_event_cb_for_eth_agent("eth3", event_val);
        }
    }

    FREE_JSON_OBJECT(jobj);
}

static int subscribe_eth_link_event()
{
    int rc = RETURN_ERR;
    rc = json_hal_client_subscribe_event(eventcb, ETH_LINKSTATUS, "onChange");
    return rc;
}
#endif //FEATURE_RDKB_WAN_MANAGER
