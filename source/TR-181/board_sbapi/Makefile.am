##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2018 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
AM_CFLAGS =

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libCcspEthAgent_board_sbapi.la

libCcspEthAgent_board_sbapi_la_CPPFLAGS = -I$(top_srcdir)/source/TR-181/include -I$(top_srcdir)/source/TR-181/middle_layer_src
libCcspEthAgent_board_sbapi_la_SOURCES = cosa_ethernet_apis.c eth_hal_interface.c ../middle_layer_src/cosa_apis_util.c ../middle_layer_src/cosa_apis_busutil.c cosa_ethernet_manager.c
libCcspEthAgent_board_sbapi_la_SOURCES += source-arm/cosa_ethernet_apis_arm.c
if FEATURE_WAN_FAIL_OVER
libCcspEthAgent_board_sbapi_la_DEPENDENCIES = ${top_builddir}/source/TR-181/middle_layer_src/libCcspEthAgent_middle_layer_src.la
libCcspEthAgent_board_sbapi_la_LIBADD = $(libCcspEthAgent_board_sbapi_la_DEPENDENCIES)
endif
libCcspEthAgent_board_sbapi_la_LDFLAGS = -lccsp_common -lutapi -lutctx -lhal_ethsw -lhal_platform -llmapi -lsecure_wrapper -lrt -lsysevent
