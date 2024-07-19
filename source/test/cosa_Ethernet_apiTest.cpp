#include <iostream>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include "CcspEthAgent_mock.h"
#define PSM_BRLAN0_ETH_MEMBERS    "dmsb.l2net.1.Members.Eth"
#define WHITE 0
#define BLINK   1
#define SOLID   0
#define ETHWAN_DEF_INTF_NAME "eth0"

extern void UserGetLocalTime(USER_SYSTEM_TIME*  pSystemTime);

using namespace std;
using std::experimental::filesystem::exists;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::HasSubstr;
extern "C"
{ 
        #include "safe_config.h"
        #include "safe_lib_errno.h"
        #include "safe_types.h"
        #include "safe_compile.h"
        #include "safe_mem_lib.h"
        #include "cosa_apis.h"
        #include "cosa_apis_util.h"
        #include "cosa_ethernet_apis.h"
        #include "cosa_ethernet_internal.h"
        #include "cosa_ethernet_manager.h"
        #include "ccsp_psm_helper.h"
        #include <platform_hal.h>
        #include "ccsp_hal_ethsw.h"
        #include "ansc_platform.h"
        #include "ansc_common_structures.h"
        #include "plugin_main_apis.h"
        #include "ansc_wrapper_api.h"
        INT g_iTraceLevel;
        INT sysevent_fd;
        ANSC_HANDLE bus_handle;
        token_t sysevent_token;
        char *pComponentName;
        char g_Subsystem[32];
        ANSC_HANDLE g_EthObject;
        COSAGetParamValueUlongProc  g_GetParamValueUlong; 
        COSAGetParamValueStringProc g_GetParamValueString;
        COSAGetParamValueBoolProc   g_GetParamValueBool;
        COSAGetInstanceNumberByIndexProc   g_GetInstanceNumberByIndex;
        COSAGetHandleProc                  g_GetRegistryRootFolder;
        COSARegisterCallBackAfterInitDmlProc  g_RegisterCallBackAfterInitDml;
        COSARepopulateTableProc            g_COSARepopulateTable;
        ANSC_HANDLE   g_MessageBusHandle;
        DslhDmagntExportedMock* g_dslhDmagntExportedMock;
        extern BOOL isEthWanEnabled();
        extern ANSC_STATUS  CosaDmlEthGetLogStatus(PCOSA_DML_ETH_LOG_STATUS pMyObject);
        extern void getInterfaceMacAddress(macaddr_t* macAddr,char *pIfname);
        extern INT BridgeNfDisable( const char* bridgeName, bridge_nf_table_t table, BOOL disable );
        extern ANSC_STATUS CosaDmlEthWanGetCfg(PCOSA_DATAMODEL_ETH_WAN_AGENT  pMyObject);
        extern int EthWanSetLED (int color, int state, int interval);
        extern PCOSA_DML_ETH_PORT_GLOBAL_CONFIG gpstEthGInfo;
}

void * g_pDslhDmlAgent;
extern SafecLibMock* g_safecLibMock;
extern SyscfgMock * g_syscfgMock ;
extern FileIOMock * g_fileIOMock;
extern EthSwHalMock *g_ethSwHALMock;
extern SecureWrapperMock *g_securewrapperMock;
extern utopiaMock  *g_utopiaMock;
//extern PtdHandlerMock * g_PtdHandlerMock;
extern PsmMock * g_psmMock;
extern PlatformHalMock *g_platformHALMock;


ACTION_TEMPLATE(SetArgNPointeeTo,HAS_1_TEMPLATE_PARAMS(unsigned ,uIndex),AND_2_VALUE_PARAMS(pData,uiDataSize))
{
   memcpy(std::get<uIndex>(args),pData,uiDataSize);
}
ACTION_P(SetstrcmpArg3,value)
{

        *static_cast<int*>(arg3)=value;
}
ACTION_P(CheckIntArg2, expectedValue) {
  EXPECT_EQ(expectedValue, static_cast<int>(arg2)); // Assuming arg2 is of type const int
}


TEST_F(CcspEthagentTestFixture,positivecaseCosaDmlEthGetLogStatus)
{
	COSA_DML_ETH_LOG_STATUS MyObject;
	char buf[16]="true";
	char pbuf[16]="566";
        int ind = 0,i=0;
	errno_t safec_rc=0;
	EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("eth_log_enabled"), _, _))
		.Times(1)
		.WillOnce(::testing::DoAll(
					SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
					::testing::Return(0)));
	EXPECT_CALL(*g_safecLibMock,_strcmp_s_chk(_, _, _, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetstrcmpArg3(ind),
                          ::testing::Return(0)));

        EXPECT_CALL(*g_safecLibMock,_memset_s_chk( _, _, _, _, _))
                .Times(1)
                .WillOnce([](void *dest,rsize_t dmax, int value,rsize_t n,const size_t destbos){memset(dest,0,sizeof(dest)); return 0;});
       
	EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("eth_log_period"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(pbuf),sizeof(pbuf)),
                        ::testing::Return(0)
                        ));
	EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthGetLogStatus(&MyObject));
	EXPECT_EQ(1,MyObject.Log_Enable);
	EXPECT_EQ(566,MyObject.Log_Period);
}

TEST_F(CcspEthagentTestFixture,negtivecaseCosaDmlEthGetLogStatus)
{
        COSA_DML_ETH_LOG_STATUS MyObject;
        char buf[16]="false";
        char pbuf[16]="567";
        int ind = 1;

        EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("eth_log_enabled"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
                        ::testing::Return(0)
                        ));
         EXPECT_CALL(*g_safecLibMock,_strcmp_s_chk(_, _, _, _, _, _))
               .Times(1)
               .WillOnce(::testing::DoAll(
                          SetstrcmpArg3(ind),
                          ::testing::Return(0)));

	  EXPECT_CALL(*g_safecLibMock,_memset_s_chk( _, _, _, _, _))
                .Times(1)
                .WillOnce([](void *dest,rsize_t dmax, int value,rsize_t n,const size_t destbos){memset(dest,0,sizeof(dest)); return 0;});

         EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("eth_log_period"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(pbuf),sizeof(pbuf)),
                        ::testing::Return(0)
                        ));

        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthGetLogStatus(&MyObject));      
        EXPECT_NE(1,MyObject.Log_Enable);
        EXPECT_NE(566,MyObject.Log_Period);
}
TEST_F(CcspEthagentTestFixture,FailcaseCosaDmlEthGetLogStatus)
{
        COSA_DML_ETH_LOG_STATUS MyObject;
        char buf[16]="false";
        char pbuf[16]="567";
        int ind = 0;

        EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("eth_log_enabled"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
                        ::testing::Return(-1)
                        ));

         EXPECT_CALL(*g_safecLibMock,_memset_s_chk( _, _, _, _, _))
                .Times(1)
                .WillOnce([](void *dest,rsize_t dmax, int value,rsize_t n,const size_t destbos){memset(dest,0,sizeof(dest)); return 0;});

         EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("eth_log_period"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(pbuf),sizeof(pbuf)),
                        ::testing::Return(-1)
                        ));

        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthGetLogStatus(&MyObject));
        EXPECT_NE(1,MyObject.Log_Enable);
        EXPECT_NE(566,MyObject.Log_Period);
}

TEST_F(CcspEthagentTestFixture, PositivecaseisEthWanEnabled)
{
	
         char buf[16]="true";
//printf("PositivecaseisEthWanEnabled\n");
         EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq("eth_wan_enabled"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                    SetArgNPointeeTo<2>(std::begin(buf), sizeof(buf)),
                    ::testing::Return(0)
                ));
	
         EXPECT_CALL(*g_fileIOMock, access(StrEq("/nvram/ETHWAN_ENABLE"),F_OK))
                .Times(1)
                .WillOnce(Return(0));
         EXPECT_EQ(TRUE, isEthWanEnabled());
}
TEST_F(CcspEthagentTestFixture, NegtivecaseisEthWanEnabled)
{
	char buf[16]="false";
       	EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq("eth_wan_enabled"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                    SetArgNPointeeTo<2>(std::begin(buf), sizeof(buf)),
                    ::testing::Return(0)
                ));
	EXPECT_EQ(FALSE, isEthWanEnabled());
}

TEST_F(CcspEthagentTestFixture, FailcaseisEthWanEnabled)
{
  char buf[16]="true";
  EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq("eth_wan_enabled"), _, _))
         .Times(1)
         .WillOnce(::testing::DoAll(
           SetArgNPointeeTo<2>(std::begin(buf), sizeof(buf)),
           ::testing::Return(-1)
        ));
  EXPECT_EQ(FALSE, isEthWanEnabled());
}

#if defined(INTEL_PUMA7)
#ifdef TEST_CASE_UNDER_DEBUGGING //Open api not working in Gtest
TEST_F(CcspEthagentTestFixture, PositiveCaseBridgeNfDisable)
{
	char wanPhyName[16]="erouter0";
        //int fd=0;
        //fd=open("nf_disable_ip6tables.txt",O_WRONLY);
        //printf("fd:%d\n",fd);
        EXPECT_EQ(0, BridgeNfDisable(wanPhyName, NF_ARPTABLE, TRUE));

        EXPECT_EQ(0, BridgeNfDisable(wanPhyName, NF_IPTABLE, TRUE));
	EXPECT_EQ(0, BridgeNfDisable(wanPhyName, NF_IP6TABLE, TRUE));
}
#endif
 TEST_F(CcspEthagentTestFixture, negtiveCaseBridgeNfDisable)
{
	char wanPhyName[16]="erouter0";
	EXPECT_EQ(-1, BridgeNfDisable(wanPhyName,(bridge_nf_table_t)3, TRUE));
}
 TEST_F(CcspEthagentTestFixture, FailedCaseBridgeNfDisable)
{
	char *wanPhyName=NULL;	
	EXPECT_EQ(-1, BridgeNfDisable(wanPhyName, NF_ARPTABLE, TRUE));
	EXPECT_EQ(-1, BridgeNfDisable(wanPhyName, NF_IPTABLE, TRUE));
	EXPECT_EQ(-1, BridgeNfDisable(wanPhyName, NF_IP6TABLE, TRUE));
}

#endif //INTEL_PUMA7
/*
ACTION_P(SetBridgeMode2,value)
{
     *static_cast<const char*>(arg2)=*value;
}
*/

ACTION_P(SetBridgeMode2, value)
{
    char* mutableCopy = strdup(static_cast<const char*>(arg2));
    *mutableCopy = *value;
    // Use mutableCopy as needed
    free(mutableCopy); // Remember to free the allocated memory
}
#ifdef TEST_CASE_UNDER_DEBUGGING //open api not working in Gtest 
 TEST_F(CcspEthagentTestFixture, PositiveCaseEthwanEnableWithoutReboot)
{
	BOOL bEnable=TRUE;
	char buf[16] = "1";
	int bridge_mode=0;
	char bridge_Enable[16]="true";
	char cmd[256];


        EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("bridge_mode"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
                        ::testing::Return(0)
                        ));
	
        EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_setEthWanPort(_))
               .Times(1)
	       .WillOnce(Return(ANSC_STATUS_SUCCESS));
			   
    EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_setEthWanEnable(_))
                .Times(1)
                .WillOnce(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_securewrapperMock, v_secure_system(::testing::HasSubstr("touch /nvram/ETHWAN_ENABLE"),_))
	    .Times(1)
	    .WillOnce(::testing::Return(0));
    EXPECT_CALL(*g_syscfgMock,syscfg_set_nns_commit( _, _))
                .Times(1)
                .WillOnce(Return(0));
    EXPECT_EQ(ANSC_STATUS_SUCCESS, EthwanEnableWithoutReboot(TRUE));
	
}



 TEST_F(CcspEthagentTestFixture, NegtiveCaseEthwanEnableWithoutReboot)
{
        BOOL bEnable=FALSE;
	char buf[16] = "0";
	int bridge_mode=0,i=0;
	char bridge_Enable[16]="false";
	char cmd[256]={0},cmd1[256]={0};

        EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("bridge_mode"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
                        ::testing::Return(0)
                        ));
                /* memset(cmd,0,sizeof(cmd));TEST_CASE_UNDER_DEBUGGING
                 snprintf(cmd,sizeof(cmd),"ip link set %s down",WAN_IF_NAME_PRIMARY);

                 EXPECT_CALL(*g_securewrapperMock, v_secure_system(_,_))
                        .Times(1)
                        .WillOnce(Return(0));

                 memset(cmd,0,sizeof(cmd));
                 snprintf(cmd,sizeof(cmd),"ip link set %s name %s",WAN_IF_NAME_PRIMARY,ETHWAN_DEF_INTF_NAME);

                 EXPECT_CALL(*g_securewrapperMock, v_secure_system(_,_))
                        .Times(1)
                        .WillOnce(::testing::Return(0));

                 memset(cmd,0,sizeof(cmd));
                 snprintf(cmd,sizeof(cmd),"ip link set dummy-rf name %s",WAN_IF_NAME_PRIMARY);
                 EXPECT_CALL(*g_securewrapperMock, v_secure_system(_,_))
                        .Times(1)
                        .WillOnce(::testing::Return(0));*/
		for(i=0;i<4;i++){
                 //memset(cmd,0,sizeof(cmd));
                 //snprintf(cmd,sizeof(cmd),"ip link set %s up;ip link set %s up",ETHWAN_DEF_INTF_NAME,WAN_IF_NAME_PRIMARY);
	
                 EXPECT_CALL(*g_securewrapperMock, v_secure_system(_,_))
                        .Times(1)
                        .WillOnce(::testing::Return(0));
		}
                // EXPECT_CALL(*g_securewrapperMock, v_secure_system(StrEq(cmd1),_))
                  //      .Times(1)
                    //    .WillOnce(::testing::Return(0));
          EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_setEthWanPort(_))
               .Times(1)
	       .WillOnce(Return(ANSC_STATUS_SUCCESS));
			   
    EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_setEthWanEnable(_))
                .Times(1)
                .WillOnce(Return(ANSC_STATUS_SUCCESS));
    strcpy(cmd1,"rm /nvram/ETHWAN_ENABLE");
    EXPECT_CALL(*g_securewrapperMock, v_secure_system(StrEq(cmd1),_))
	    .Times(1)
	    .WillOnce(::testing::Return(0));
	
	
    EXPECT_CALL(*g_syscfgMock,syscfg_set_nns_commit(_, _))
                .Times(1)
                .WillOnce(Return(0));			
    EXPECT_EQ(ANSC_STATUS_SUCCESS, EthwanEnableWithoutReboot(TRUE));
	
}

 TEST_F(CcspEthagentTestFixture, FailedCaseEthwanEnableWithoutReboot)
{
	BOOL bEnable=FALSE;
	char buf[16] = "0";
	int bridge_mode=0,i=0;
	char bridge_Enable[16]="false";
	char cmd[256];
        EXPECT_CALL(*g_syscfgMock,syscfg_get(_,StrEq("bridge_mode"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
                        ::testing::Return(0)
                        ));
         char expectedcmd[256]={0};
	
         memset(expectedcmd,0,sizeof(expectedcmd));
         snprintf(expectedcmd,sizeof(expectedcmd),"ip link set %s down",WAN_IF_NAME_PRIMARY);

          EXPECT_CALL(*g_securewrapperMock, v_secure_system(::testing::HasSubstr(expectedcmd), _))
                        .Times(1)
                        .WillOnce(Return(0));

          memset(cmd,0,sizeof(cmd));
          snprintf(cmd,sizeof(cmd),"ip link set %s name %s",WAN_IF_NAME_PRIMARY,ETHWAN_DEF_INTF_NAME);

          EXPECT_CALL(*g_securewrapperMock, v_secure_system(HasSubstr(cmd),_))
                        .Times(1)
                        .WillOnce(::testing::Return(0));

          memset(cmd,0,sizeof(cmd));
          snprintf(cmd,sizeof(cmd),"ip link set dummy-rf name %s",WAN_IF_NAME_PRIMARY);
          EXPECT_CALL(*g_securewrapperMock, v_secure_system(HasSubstr(cmd),_))
                        .Times(1)
                        .WillOnce(::testing::Return(0));

          memset(cmd,0,sizeof(cmd));
          snprintf(cmd,sizeof(cmd),"ip link set %s up;ip link set %s up",ETHWAN_DEF_INTF_NAME,WAN_IF_NAME_PRIMARY);
          EXPECT_CALL(*g_securewrapperMock, v_secure_system(HasSubstr(cmd),_))
                        .Times(1)
                        .WillOnce(::testing::Return(0));

	EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_setEthWanPort(_))
               .Times(1)
               .WillOnce(Return(ANSC_STATUS_FAILURE));

        EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_setEthWanEnable(_))
                .Times(1)
                .WillOnce(Return(ANSC_STATUS_FAILURE));
			
        EXPECT_EQ(ANSC_STATUS_FAILURE, EthwanEnableWithoutReboot(TRUE));
	
}

#endif

ACTION_P(SetLinkStatus3,value)
{
     *static_cast<CCSP_HAL_ETHSW_LINK_STATUS*>(arg3)=*value;
     //	arg3=value;
}
TEST_F(CcspEthagentTestFixture, PositiveCaseCosaDmlEthWanLinkStatus)
{
        CCSP_HAL_ETHSW_LINK_STATUS LinkStatus = CCSP_HAL_ETHSW_LINK_Up;
        EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(_, _, _, _))
               .Times(1)
               .WillOnce(::testing::DoAll(
                SetLinkStatus3(&LinkStatus),
                ::testing::Return(RETURN_OK))); 
        EXPECT_EQ(TRUE, CosaDmlEthWanLinkStatus());
}
TEST_F(CcspEthagentTestFixture, NegtiveCaseCosaDmlEthWanLinkStatus)
{
        CCSP_HAL_ETHSW_LINK_STATUS LinkStatus = CCSP_HAL_ETHSW_LINK_Down;
        EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(_, _, _, _))
               .Times(1)
               .WillOnce(::testing::DoAll(
               SetLinkStatus3(&LinkStatus),
               ::testing::Return(RETURN_OK)));

        EXPECT_EQ(FALSE, CosaDmlEthWanLinkStatus());

}
TEST_F(CcspEthagentTestFixture, FailedCaseCosaDmlEthWanLinkStatus)
{
        CCSP_HAL_ETHSW_LINK_STATUS LinkStatus = CCSP_HAL_ETHSW_LINK_Down;
        EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(_, _, _, _))
               .Times(1)
               .WillOnce(::testing::DoAll(
                SetLinkStatus3(&LinkStatus),
                ::testing::Return(RETURN_ERR)));

	EXPECT_EQ(FALSE, CosaDmlEthWanLinkStatus());
}

#ifdef TEST_CASE_UNDER_DEBUGGING //open api not working in gtest 

TEST_F(CcspEthagentTestFixture, PositiveCasegetInterfaceMacAddress)
{
        char buf[256]="1c:9e:cc:21:7d:ee";
	int i=0;
	macaddr_t macAddr;
	macaddr_t mac;
	char Ifname[50]="erouter0";
	EXPECT_CALL(*g_fileIOMock,getline(_, _, _))
               .Times(1)
	       .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
              ::testing::Return(0)));

       //getInterfaceMacAddress(&macAddr,Ifname);
        sscanf(buf, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac.hw[0], &mac.hw[1], &mac.hw[2], &mac.hw[3], &mac.hw[4], &mac.hw[5]);	
        getInterfaceMacAddress(&macAddr,Ifname);
        for (i=0;i<=5;i++)
        {
           EXPECT_EQ(mac.hw[i], macAddr.hw[0]);
        }
}

TEST_F(CcspEthagentTestFixture, NegtiveCasegetInterfaceMacAddress)
{
	char buf[256]="1c:9e:cc:21:7d:ee",buffer[256]="76:2e:32:43:35:32";
	macaddr_t macAddr;
	macaddr_t mac;
	char Ifname[50]="erouter0";
	int i=0;
	EXPECT_CALL(*g_fileIOMock,getline(_, _, _))
               .Times(1)
	       .WillOnce(::testing::DoAll(
                          SetArgNPointeeTo<2>(std::begin(buffer),sizeof(buf)),
                          ::testing::Return(0)));

	//getInterfaceMacAddress(&macAddr,Ifname);

        sscanf(buf, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac.hw[0], &mac.hw[1], &mac.hw[2], &mac.hw[3], &mac.hw[4], &mac.hw[5]);	
        getInterfaceMacAddress(&macAddr,Ifname);   	
        for (i=0;i<=5;i++)
        {
          EXPECT_EQ(mac.hw[i], macAddr.hw[0]);
        }
}


TEST_F(CcspEthagentTestFixture, FailedCasegetInterfaceMacAddress)
{
	char buf[256]="1c:9e:cc:21:7d:ee";
	char Ifname[50]="erouter0";
	macaddr_t macAddr;
	macaddr_t mac;
	int i=0;
	EXPECT_CALL(*g_fileIOMock,getline(_, _, _))
               .Times(1)
	       .WillOnce(::testing::DoAll(
                 SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
		 ::testing::Return(-1)));

       // getInterfaceMacAddress(&macAddr,Ifname);
        sscanf(buf, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac.hw[0], &mac.hw[1], &mac.hw[2], &mac.hw[3], &mac.hw[4], &mac.hw[5]);
	getInterfaceMacAddress(&macAddr,Ifname);
        for (i=0;i<=5;i++)
        {
          EXPECT_EQ(mac.hw[i], macAddr.hw[0]);
        }



}
#endif


ACTION_P(SetUintArg0,value)
{

       *static_cast<UINT*>(arg0)=value;
}
ACTION_P(SetBoolArg0,value)
{

       *static_cast<BOOLEAN*>(arg0)=value;
}


TEST_F(CcspEthagentTestFixture, PositiveCaseCosaDmlEthWanGetCfg)
{
	COSA_DATAMODEL_ETH_WAN_AGENT  MyObject;
	bool Enable=true;
	unsigned int port=5001;
	//EXPECT_CALL(*g_safecLibMock,_memset_s_chk(&MyObject,sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ),0,sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ),sizeof(MyObject)))
         //      .Times(1)
            //    .WillOnce(::testing::Return(0));
        EXPECT_CALL(*g_safecLibMock,_memset_s_chk( _, _, _, _, _))
                .Times(1)
                .WillOnce([](void *dest,rsize_t dmax, int value,rsize_t n,const size_t destbos){memset(dest,0,sizeof(dest)); return 0;});

	#if defined (ENABLE_ETH_WAN)
	EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_getEthWanEnable(_))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetBoolArg0(Enable),
                          ::testing::Return(RETURN_OK)));
        EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_getEthWanPort(_))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetUintArg0(port),
                          ::testing::Return(RETURN_OK)));
        #endif /*  ENABLE_ETH_WAN */
        EXPECT_EQ(ANSC_STATUS_SUCCESS, CosaDmlEthWanGetCfg(&MyObject));
}
TEST_F(CcspEthagentTestFixture, NegativeCaseCosaDmlEthWanGetCfg)
{
	COSA_DATAMODEL_ETH_WAN_AGENT  MyObject;
	bool Enable=false;
	unsigned int port=-6777;
        //EXPECT_CALL(*g_safecLibMock,_memset_s_chk(&MyObject,sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ),0,sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ),sizeof(MyObject)))
  //              .Times(1)
    //            .WillOnce(::testing::Return(0));
         EXPECT_CALL(*g_safecLibMock,_memset_s_chk( _, _, _, _, _))
                .Times(1)
                .WillOnce([](void *dest,rsize_t dmax, int value,rsize_t n,const size_t destbos){memset(dest,0,sizeof(dest)); return 0;});

        #if defined (ENABLE_ETH_WAN)
         EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_getEthWanEnable(_))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetBoolArg0(Enable),
                          ::testing::Return(RETURN_OK)));
         EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_getEthWanPort(_))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetUintArg0(port),
                          ::testing::Return(RETURN_OK)));
         #endif /*  ENABLE_ETH_WAN */
         EXPECT_EQ(ANSC_STATUS_SUCCESS, CosaDmlEthWanGetCfg(&MyObject));
}

TEST_F(CcspEthagentTestFixture, FailedCaseCosaDmlEthWanGetCfg)
{
	COSA_DATAMODEL_ETH_WAN_AGENT  MyObject;
	bool Enable=true;
	unsigned int port=5001;
	//EXPECT_CALL(*g_safecLibMock,_memset_s_chk(&MyObject,sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ),0,sizeof( COSA_DATAMODEL_ETH_WAN_AGENT ),sizeof(MyObject)))
         //      .Times(1)
            //    .WillOnce(::testing::Return(0));
          EXPECT_CALL(*g_safecLibMock,_memset_s_chk( _, _, _, _, _))
                .Times(1)
                .WillOnce([](void *dest,rsize_t dmax, int value,rsize_t n,const size_t destbos){memset(dest,0,sizeof(dest)); return 0;});

        #if defined (ENABLE_ETH_WAN)
        EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_getEthWanEnable(_))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetBoolArg0(Enable),
                          ::testing::Return(!RETURN_OK)));
        EXPECT_CALL(*g_ethSwHALMock,CcspHalExtSw_getEthWanPort(_))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetUintArg0(port),
                          ::testing::Return(!RETURN_OK)));
        #endif /*  ENABLE_ETH_WAN */
        EXPECT_EQ(ANSC_STATUS_SUCCESS, CosaDmlEthWanGetCfg(&MyObject));
}


ACTION_P(SetLinkRateArg1,value)
{

     *static_cast<CCSP_HAL_ETHSW_LINK_RATE*>(arg1)=value;
}
TEST_F(CcspEthagentTestFixture, PositiveCaseethGetPHYRate)
{
        CCSP_HAL_ETHSW_PORT PortId = CCSP_HAL_ETHSW_EthPort1;
       CCSP_HAL_ETHSW_LINK_RATE LinkRate = CCSP_HAL_ETHSW_LINK_10Mbps;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
     EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
     .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#endif
    EXPECT_EQ(10, ethGetPHYRate(PortId));


     LinkRate = CCSP_HAL_ETHSW_LINK_100Mbps;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
     EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
     .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#endif
    EXPECT_EQ(100, ethGetPHYRate(PortId));


     LinkRate = CCSP_HAL_ETHSW_LINK_1Gbps;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
     EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#endif
    EXPECT_EQ(1000, ethGetPHYRate(PortId));

#ifdef _2_5G_ETHERNET_SUPPORT_

    LinkRate = CCSP_HAL_ETHSW_LINK_2_5Gbps;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                 SetLinkRateArg1(LinkRate);
                 ::testing::Return(RETURN_OK)));
	#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                SetLinkRateArg1(LinkRate),
                ::testing::Return(RETURN_OK)));
#endif
    EXPECT_EQ(2500, ethGetPHYRate(PortId));

    LinkRate = CCSP_HAL_ETHSW_LINK_5Gbps;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                SetLinkRateArg1(LinkRate),
                ::testing::Return(RETURN_OK)));
#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                SetLinkRateArg1(LinkRate),
                ::testing::Return(RETURN_OK)));
#endif
    EXPECT_EQ(5000, ethGetPHYRate(PortId));

#endif //_2_5G_ETHERNET_SUPPORT_


LinkRate = CCSP_HAL_ETHSW_LINK_10Gbps;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
     EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
     .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#endif
    EXPECT_EQ(10000, ethGetPHYRate(PortId));

 LinkRate = CCSP_HAL_ETHSW_LINK_Auto;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
     EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
     .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#endif
    EXPECT_EQ(1000, ethGetPHYRate(PortId));
}

TEST_F(CcspEthagentTestFixture, NegtiveCaseethGetPHYRate)
{
        CCSP_HAL_ETHSW_PORT PortId = CCSP_HAL_ETHSW_EthPort1;
        CCSP_HAL_ETHSW_LINK_RATE LinkRate = CCSP_HAL_ETHSW_LINK_NULL;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
     EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
     .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(RETURN_OK)));
#endif
    EXPECT_EQ(0, ethGetPHYRate(PortId));
}
TEST_F(CcspEthagentTestFixture, FailedCaseethGetPHYRate)
{
        CCSP_HAL_ETHSW_PORT PortId = CCSP_HAL_ETHSW_EthPort1;
        CCSP_HAL_ETHSW_LINK_RATE LinkRate = CCSP_HAL_ETHSW_LINK_NULL;
#if defined(_CBR_PRODUCT_REQ_) || defined(_COSA_BCM_MIPS_) || defined(_SCER11BEL_PRODUCT_REQ_) || ( defined (_XB6_PRODUCT_REQ_) && defined (_COSA_BCM_ARM_))
     EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortStatus(PortId, _, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(!RETURN_OK)));
#else
    EXPECT_CALL(*g_ethSwHALMock,CcspHalEthSwGetPortCfg(PortId, _, _))
     .Times(1)
                .WillOnce(::testing::DoAll(
                          SetLinkRateArg1(LinkRate),
                          ::testing::Return(!RETURN_OK)));
#endif
    EXPECT_EQ(0, ethGetPHYRate(PortId));
}


#if defined (FEATURE_RDKB_WAN_MANAGER)
TEST_F(CcspEthagentTestFixture, PositiveCaseEthWanSetLED)
{
#if defined(_XB6_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_)
#if !defined(FEATURE_RDKB_LED_MANAGER_PORT)
    EXPECT_CALL(*g_platformHALMock,platform_hal_setLed(_))
     .Times(1)
     .WillOnce(::testing::Return(RETURN_OK));
#endif
#endif
     EXPECT_EQ(0,EthWanSetLED(WHITE, BLINK, 1));
#if defined(_XB6_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_)
#if !defined(FEATURE_RDKB_LED_MANAGER_PORT)
    EXPECT_CALL(*g_platformHALMock,platform_hal_setLed(_))
     .Times(1)
     .WillOnce(::testing::Return(RETURN_OK));
#endif
#endif
    EXPECT_EQ(0,EthWanSetLED(WHITE, SOLID, 1));
#if defined(_XB6_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_)
#if !defined(FEATURE_RDKB_LED_MANAGER_PORT)
    EXPECT_CALL(*g_platformHALMock,platform_hal_setLed(_))
     .Times(1)
     .WillOnce(::testing::Return(RETURN_OK));
#endif
#endif
    EXPECT_EQ(0,EthWanSetLED(WHITE, BLINK, 5));
}

TEST_F(CcspEthagentTestFixture, NegtiveCaseEthWanSetLED)
{
    int flag=0;
#if defined(_XB6_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_)
#if !defined(FEATURE_RDKB_LED_MANAGER_PORT)
    EXPECT_CALL(*g_platformHALMock,platform_hal_setLed(_))
     .Times(1)
     .WillOnce(::testing::Return(RETURN_ERR));
     EXPECT_EQ(1,EthWanSetLED(WHITE, BLINK, 1));
     flag=1;
#endif
#endif
     if(flag==0){
     EXPECT_EQ(0,EthWanSetLED(WHITE, BLINK, 1));
     }
}
#endif

#if defined (FEATURE_RDKB_WAN_AGENT)
TEST_F(CcspEthagentTestFixture, PositiveCaseCosaDmlEthPortGetWanStatus)
{
        INT IfIndex=0;
        COSA_DML_ETH_WAN_STATUS wan_status;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_UP;
        printf("debug%d",gpstEthGInfo[IfIndex].WanStatus);
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_UP,wan_status);

        IfIndex=1;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_UP,wan_status);

        IfIndex=2;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_UP,wan_status);

        IfIndex=3;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_UP,wan_status);
}
TEST_F(CcspEthagentTestFixture, NegativeCaseCosaDmlEthPortGetWanStatus)
{
        INT IfIndex=0;
        COSA_DML_ETH_WAN_STATUS wan_status;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_DOWN;
        EXPECT_EQ(1,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_DOWN,wan_status);

        IfIndex=1;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_DOWN,wan_status);

        IfIndex=2;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_DOWN,wan_status);

        IfIndex=3;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_DOWN,wan_status);
}
TEST_F(CcspEthagentTestFixture, FailedCaseCosaDmlEthPortGetWanStatus)
{
        INT IfIndex=4;
        COSA_DML_ETH_WAN_STATUS wan_status;
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_FAILURE,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_DOWN,wan_status);
        gpstEthGInfo[IfIndex].WanStatus = ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_FAILURE,CosaDmlEthPortGetWanStatus(IfIndex, &wan_status));
        EXPECT_EQ(ETH_WAN_UP,wan_status);
}


TEST_F(CcspEthagentTestFixture, PositiveCaseCosaDmlEthPortSetWanStatus)
{
        INT IfIndex=0;
	COSA_DML_ETH_WAN_STATUS wan_status=ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);

        IfIndex=1;
        wan_status = ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);

        IfIndex=2;
        wan_status = ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);

        IfIndex=3;
        wan_status = ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);
}
TEST_F(CcspEthagentTestFixture, NegativeCaseCosaDmlEthPortSetWanStatus)
{
        INT IfIndex=0;
        COSA_DML_ETH_WAN_STATUS wan_status;
        wan_status = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);

        IfIndex=1;
        wan_status = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);

        IfIndex=2;
        wan_status = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);

        IfIndex=3;
        wan_status = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);
}
TEST_F(CcspEthagentTestFixture, FailedCaseCosaDmlEthPortSetWanStatus)
{
        INT IfIndex=4;
        COSA_DML_ETH_WAN_STATUS wan_status;
        wan_status = ETH_WAN_DOWN;
        EXPECT_EQ(ANSC_STATUS_FAILURE,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
	EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);
        wan_status = ETH_WAN_UP;
        EXPECT_EQ(ANSC_STATUS_FAILURE,CosaDmlEthPortSetWanStatus(IfIndex, wan_status));
        EXPECT_EQ(gpstEthGInfo[IfIndex].WanStatus,wan_status);
}


TEST_F(CcspEthagentTestFixture, PositiveCaseCosaDmlEthPortGetLinkStatus)
{
        INT IfIndex=0;
        COSA_DML_ETH_LINK_STATUS LinkStatus;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_UP,LinkStatus);

        IfIndex=1;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_UP,LinkStatus);

        IfIndex=2;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_UP,LinkStatus);

        IfIndex=3;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_UP;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_UP,LinkStatus);
}
TEST_F(CcspEthagentTestFixture, NegativeCaseCosaDmlEthPortGetLinkStatus)
{
        INT IfIndex=0;
        COSA_DML_ETH_LINK_STATUS LinkStatus;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_DOWN;
        EXPECT_EQ(1,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_DOWN,LinkStatus);

        IfIndex=1;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_DOWN,LinkStatus);

        IfIndex=2;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_DOWN,LinkStatus);

        IfIndex=3;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_DOWN;
        EXPECT_EQ(ANSC_STATUS_SUCCESS,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_DOWN,LinkStatus);
}
TEST_F(CcspEthagentTestFixture, FailedCaseCosaDmlEthPortGetLinkStatus)
{
        INT IfIndex=4;
        COSA_DML_ETH_LINK_STATUS LinkStatus;
        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_DOWN;
        EXPECT_EQ(ANSC_STATUS_FAILURE,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_UP,LinkStatus);

        gpstEthGInfo[IfIndex].LinkStatus = ETH_LINK_STATUS_UP;
        EXPECT_EQ(ANSC_STATUS_FAILURE,CosaDmlEthPortGetLinkStatus(IfIndex, &LinkStatus));
        EXPECT_EQ(ETH_LINK_STATUS_UP,LinkStatus);
}
#endif

#if defined (FEATURE_RDKB_WAN_MANAGER)
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
#ifdef TEST_CASE_UNDER_DEBUGGING //open API not working
ACTION_P(SetPsmValueArg4,value)
{
	*static_cast<char**>(arg4)=strdup(*value);
}
TEST_F(CcspEthagentTestFixture, PositivecaseEthagent_GetParamValuesFromPSM)
{
        char buf[64]="true";
        char *ptr=buf;
        char acGetParamName[256]={0},ReturnVal[64]={0};
        snprintf(acGetParamName, sizeof(acGetParamName), PSM_ETHMANAGER_CFG_ADDTOBRIDGE, 1);
	printf("acGetParamName:%s\n",acGetParamName);
        EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _,_, _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                    SetPsmValueArg4(&buf),
                    ::testing::Return(CCSP_SUCCESS)
                ));
        printf("debugtest1:%s\n",buf);
        EXPECT_EQ(CCSP_SUCCESS, Ethagent_GetParamValuesFromPSM(acGetParamName,ReturnVal,sizeof(ReturnVal)));
	printf("debugtest1:%s\n",ReturnVal);
        EXPECT_STREQ(buf,ReturnVal);
        printf("debugtest1:%s\n",ReturnVal);
}

TEST_F(CcspEthagentTestFixture, NegtivecaseEthagent_GetParamValuesFromPSM)
{

        char buf[64]="false";
        char acGetParamName[256]={0},ReturnVal[64]={0};
        snprintf(acGetParamName, sizeof(acGetParamName), PSM_ETHMANAGER_CFG_ADDTOBRIDGE, 1);	
        EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _,StrEq(acGetParamName), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                    SetPsmValueArg4(&buf),
                    ::testing::Return(CCSP_SUCCESS)
                ));

        EXPECT_EQ(CCSP_SUCCESS, Ethagent_GetParamValuesFromPSM(acGetParamName,ReturnVal,sizeof(ReturnVal)));
        EXPECT_STREQ(buf,ReturnVal);
}
TEST_F(CcspEthagentTestFixture, FailedcaseEthagent_GetParamValuesFromPSM)
{

        char buf[16]="true";
        char acGetParamName[256]={0},ReturnVal[16]={0};
        char *pacGetParamName,*pReturnVal;
        EXPECT_EQ(CCSP_FAILURE, Ethagent_GetParamValuesFromPSM(pacGetParamName,pReturnVal,0));
        snprintf(acGetParamName, sizeof(acGetParamName), PSM_ETHMANAGER_CFG_ADDTOBRIDGE, 1);	
        EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _,StrEq(acGetParamName), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                    SetPsmValueArg4(&buf),
                    ::testing::Return(CCSP_FAILURE)
                ));

        EXPECT_EQ(CCSP_FAILURE, Ethagent_GetParamValuesFromPSM(acGetParamName,ReturnVal,sizeof(ReturnVal)));
}

TEST_F(CcspEthagentTestFixture, PositivecaseEthagent_SetParamValuesToPSM)
{

        char ParamVal[128]="eth0 eth1 eth2 eth3";
        char Paramname[128]="dmsb.l2net.1.Members.Eth";
        EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _,StrEq(PSM_BRLAN0_ETH_MEMBERS), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                    SetPsmValueArg4(&ParamVal),
                    ::testing::Return(CCSP_SUCCESS)
                ));

         EXPECT_EQ(CCSP_SUCCESS, Ethagent_SetParamValuesToPSM(Paramname,ParamVal));
}
TEST_F(CcspEthagentTestFixture, NegtivecaseEthagent_SetParamValuesToPSM)
{

        char ParamVal[128]="eth0 eth1 eth2 eth3";
        char Paramname[128]="dmsb.l2net.1.Members.Eth";
        EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _,StrEq(PSM_BRLAN0_ETH_MEMBERS), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                    SetPsmValueArg4(&ParamVal),
                    ::testing::Return(CCSP_FAILURE)
                ));

         EXPECT_EQ(CCSP_FAILURE, Ethagent_SetParamValuesToPSM(Paramname,ParamVal));
}
TEST_F(CcspEthagentTestFixture, FailedcaseEthagent_SetParamValuesToPSM)
{

        char *pParamVal,*pParamName;
        EXPECT_EQ(CCSP_FAILURE, Ethagent_SetParamValuesToPSM(pParamName,pParamVal));
}

#endif
#endif
#endif
