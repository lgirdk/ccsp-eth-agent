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

bin_PROGRAMS = CcspEthAgent

CcspEthAgent_CPPFLAGS = -I$(top_srcdir)/source/TR-181/include -I$(top_srcdir)/source/TR-181/middle_layer_src

CcspEthAgent_DEPENDENCIES= \
	$(EXTRA_DEPENDENCIES) \
	${top_builddir}/source/TR-181/middle_layer_src/libCcspEthAgent_middle_layer_src.la \
	${top_builddir}/source/TR-181/board_sbapi/libCcspEthAgent_board_sbapi.la

CcspEthAgent_CFLAGS = $(SYSTEMD_CFLAGS) $(NONROOT_CFLAGS)
CcspEthAgent_SOURCES = ssp_messagebus_interface.c ssp_main.c ssp_action.c dm_pack_datamodel.c
CcspEthAgent_LDFLAGS = -rdynamic $(SYSTEMD_LDFLAGS) -pthread
CcspEthAgent_LDADD = $(CcspEthAgent_DEPENDENCIES) -lccsp_common -lsyscfg -lprivilege -ldl
