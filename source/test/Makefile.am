##########################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
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

AM_CFLAGS = -fprofile-arcs -ftest-coverage
AM_CPPFLAGS =  -Wall -g -Werror
AM_CXXFLAGS = -std=c++11 -fprofile-arcs -ftest-coverage
AM_LDFLAGS = -lccsp_common
ACLOCAL_AMFLAGS = -I m4
AUTOMAKES_OPTIONS = subdir-objects
bin_PROGRAMS = CcspEthAgent_gtest.bin

CcspEthAgent_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/ccsp \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/glib-2.0 \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(libdir)/glib-2.0/include \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/libqmi-glib \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/dbus-1.0 \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(libdir)/dbus-1.0/include \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/libsafec \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/rbus \
                                  -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/rtmessage \
                                  -I${top_srcdir}/gtest/include \
                                  -I${top_srcdir}/source \
                                  -I${top_srcdir}/mocks \
                                  -I${top_srcdir}/source/TR-181/include \
                                  -I${top_srcdir}/source/EthSsp \
                                  -I${top_srcdir}/../hal/include \
                                  -I${top_srcdir}/../CcspCommonLibrary/source/cosa/include \
                                  -I${top_srcdir}/../CcspCommonLibrary/source/ccsp/components/include \
                                  -I${top_srcdir}/source/TR-181/board_sbapi \
                                  -I${top_srcdir}/source/TR-181/middle_layer_src \
                                  -I${top_srcdir}/source/TR-181/board_sbapi/source-arm

CcspEthAgent_gtest_bin_CPPFLAGS += \
				   -DRBUS_BUILD_FLAG_ENABLE \
				   -DFEATURE_RDKB_WAN_UPSTREAM \
				   -D_COSA_INTEL_USG_ARM_ \
				   -DINTEL_PUMA7 -DFEATURE_WAN_MANAGER \
				   -DENABLE_ETH_WAN \
				   -DFEATURE_RDKB_WAN_MANAGER \
				   -DFEATURE_RDKB_AUTO_PORT_SWITCH

CcspEthAgent_gtest_bin_SOURCES  = ${top_srcdir}/source/TR-181/board_sbapi/cosa_ethernet_apis.c \
                                  ${top_srcdir}/source/TR-181/board_sbapi/cosa_ethernet_manager.c \
                                  ${top_srcdir}/source/TR-181/middle_layer_src/cosa_apis_util.c \
                                  ${top_srcdir}/source/TR-181/middle_layer_src/cosa_apis_busutil.c \
                                  ${top_srcdir}/source/TR-181/middle_layer_src/cosa_ethernet_internal.c \
                                  ${top_srcdir}/source/TR-181/middle_layer_src/cosa_rbus_handler_apis.c \
                                  ${top_srcdir}/source/TR-181/board_sbapi/source-arm/cosa_ethernet_apis_arm.c \
                                  cosa_Ethernet_apiTest.cpp \
                                  CcspEthAgent_mock.cpp \
                                  cosa_Ethernet_apis_arm.cpp \
                                  mocks/mock_ccspethagent.cpp \
                                  gtest_main.cpp

CcspEthAgent_gtest_bin_LDFLAGS  = -lgtest -lgmock -lgcov -ldl -lpthread  -lcurl -lrt --coverage 


CcspEthAgent_gtest_bin_LDADD = $(HOME)/usr/local/lib/libmock_rbus.la \
         $(HOME)/usr/local/lib/libmock_sysevent.la \
         $(HOME)/usr/local/lib/libmock_syscfg.la \
         $(HOME)/usr/local/lib/libmock_socket.la \
         $(HOME)/usr/local/lib/libmock_psm.la \
   	 $(HOME)/usr/local/lib/libmock_file_io.la \
         $(HOME)/usr/local/lib/libmock_secure_wrapper.la \
         $(HOME)/usr/local/lib/libmock_ccsp_common.la \
         $(HOME)/usr/local/lib/libmock_ethsw_hal.la \
         $(HOME)/usr/local/lib/libmock_platform_hal.la \
         $(HOME)/usr/local/lib/libmock_safec_lib.la \
         $(HOME)/usr/local/lib/libmock_utopia.la 
