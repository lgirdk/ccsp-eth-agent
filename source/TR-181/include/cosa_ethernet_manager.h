/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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

    module: cosa_ethernet_manager.h

        For CCSP Component:  eth agent

    ---------------------------------------------------------------

    copyright:

        Comcast, Corp., 2019
        All Rights Reserved.

    ---------------------------------------------------------------

    description:

        This sample implementation file gives the function call prototypes and
        structure definitions used for the RDK-Broadband
        Ethernet Manager


    ---------------------------------------------------------------

    environment:

        This layer is intended to controls Ethernet state machine
        through an open API.

    ---------------------------------------------------------------

**********************************************************************/

#ifndef __COSA_ETH_MANAGER_H__
#define __COSA_ETH_MANAGER_H__

/* ---- Include Files ---------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

/**
 * Stucrure holds the information used in State machine.
 */
typedef struct _ETH_SM_PRIVATE_INFO
{
    char Name[64];
} ETH_SM_PRIVATE_INFO, *PETH_SM_PRIVATE_INFO;

void
CosaEthManager_Start_StateMachine(PETH_SM_PRIVATE_INFO stPrivateInfo);
#endif /* __COSA_ETH_MANAGER_H__ */
