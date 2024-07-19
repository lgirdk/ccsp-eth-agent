#ifndef CCSPETHAGENT_MOCK_H
#define CCSPETHAGENT_MOCK_H
#include "gtest/gtest.h"
#include <sys/socket.h>
#include <sys/types.h>
#include "utctx/utctx_api.h"
#include <sys/ioctl.h>
#include <experimental/filesystem>
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <mocks/mock_syscfg.h>
#include <mocks/mock_file_io.h>
#include <mocks/mock_psm.h>
#include <mocks/mock_socket.h>
#include <mocks/mock_sysevent.h>
#include <mocks/mock_securewrapper.h>
#include <mocks/mock_ansc_memory.h>
#include <mocks/mock_base_api.h>
#include <mocks/mock_usertime.h>
#include <mocks/mock_ansc_wrapper_api.h>
#include <mocks/mock_trace.h>
#include <syscfg/syscfg.h>
#include <mocks/mock_safec_lib.h>
#include <mocks/mock_ethsw_hal.h>
#include "safec_lib_common.h"
#include <user_time.h>
#include <mocks/mock_ccspethagent.h>
#include <mocks/mock_ethsw_hal.h>
#include <mocks/mock_pthread.h>
#include <mocks/mock_platform_hal.h>
#include <mocks/mock_dslh_dmagnt_exported.h>
#include <mocks/mock_utopia.h>
#include "mocks/mock_ccspethagent.h"
#include <mocks/mock_rbus.h>

class CcspEthagentTestFixture : public :: testing::TestWithParam<int>{

        protected:
        SyscfgMock mockedSyscfg;
        FileIOMock mockedFileIO;
        SocketMock mockedSocket;
        SyseventMock mockedSysevent;
        PsmMock mockedPsm;
        SecureWrapperMock mockedsecurewrapper;
        AnscMemoryMock mockedanscMemoryMock;
        BaseAPIMock mockedbaseapi;
        UserTimeMock mockedUsertime;
        AnscWrapperApiMock mockedAnscWrapperApi;
        TraceMock mockedTrace;
        CcspEthagentEthernetMock mockedCcspEthagentEthernet;
        EthSwHalMock mockedethSwHAL;
        SafecLibMock mockedsafecLib;
        PtdHandlerMock  mockedPtdHandler;
        PlatformHalMock mockedplatformHAL;
        utopiaMock mockedutopiaMock;
        rbusMock mockedrbus;
        CcspEthagentTestFixture();
        virtual ~CcspEthagentTestFixture();
        virtual void  SetUp() override;
        virtual void TearDown() override;
        virtual void TestBody() override;
  };


#endif //CCSPETHAGENT_MOCK_H
