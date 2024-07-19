#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <stdbool.h>
#include "CcspEthAgent_mock.h"
using namespace std;
using std::experimental::filesystem::exists;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

extern utopiaMock *g_utopiaMock;
extern SyscfgMock * g_syscfgMock ;
extern "C"
{
       #include "plugin_main_apis.h" 
       #include "ansc_platform.h"
       extern  BOOL getSystemUpstream();
       char*   g_SubsystemPrefix;
}

ACTION_TEMPLATE(SetArgNPointeeTo,HAS_1_TEMPLATE_PARAMS(unsigned ,uIndex),AND_2_VALUE_PARAMS(pData,uiDataSize))
{
      memcpy(std::get<uIndex>(args),pData,uiDataSize);
}

TEST_F(CcspEthagentTestFixture, PossitiveCasegetSystemUpstream)
{
	char buf[8]="1";
        EXPECT_CALL(*g_syscfgMock,syscfg_get( _,StrEq("Ethwan_Disable_Upstream"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
                        ::testing::Return(0)
                        ));
        EXPECT_EQ(FALSE, getSystemUpstream());
}

TEST_F(CcspEthagentTestFixture, NegtiveCasegetSystemUpstream)
{
        char buf[8]="0";
        EXPECT_CALL(*g_syscfgMock,syscfg_get( _,StrEq("Ethwan_Disable_Upstream"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
                        ::testing::Return(0)
                        ));
       EXPECT_EQ(TRUE, getSystemUpstream());
}

TEST_F(CcspEthagentTestFixture, FailCasegetSystemUpstream)
{
        char buf[8]="0";
        EXPECT_CALL(*g_syscfgMock,syscfg_get( _,StrEq("Ethwan_Disable_Upstream"), _, _))
                .Times(1)
                .WillOnce(::testing::DoAll(
                        SetArgNPointeeTo<2>(std::begin(buf),sizeof(buf)),
                        ::testing::Return(-1)
                        ));
       EXPECT_EQ(TRUE, getSystemUpstream());
}

