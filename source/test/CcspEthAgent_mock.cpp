#include <gmock/gmock.h>
#include "CcspEthAgent_mock.h"

#ifndef CCSPETHAGENT_FIXTURE_G_MOCK
#define CCSPETHAGENT_FIXTURE_G_MOCK

EthSwHalMock *g_ethSwHALMock=NULL;
SafecLibMock* g_safecLibMock=NULL;
SyscfgMock * g_syscfgMock = NULL;
FileIOMock * g_fileIOMock = NULL;
SocketMock * g_socketMock = NULL;
SyseventMock * g_syseventMock = NULL;
PsmMock * g_psmMock = NULL;
SecureWrapperMock * g_securewrapperMock = NULL;
AnscMemoryMock * g_anscMemoryMock       = NULL;
BaseAPIMock * g_baseapiMock = NULL;
AnscWrapperApiMock * g_anscWrapperApiMock = NULL;
TraceMock * g_traceMock = NULL;
UserTimeMock * g_usertimeMock = NULL;
CcspEthagentEthernetMock *g_CcspEthagentEthernetMock=NULL;
PtdHandlerMock * g_PtdHandlerMock=NULL;
PlatformHalMock *g_platformHALMock = NULL;
utopiaMock *g_utopiaMock=NULL;
rbusMock *g_rbusMock;

CcspEthagentTestFixture :: CcspEthagentTestFixture()
{

 g_rbusMock=new rbusMock;
 g_ethSwHALMock=new EthSwHalMock;
 g_safecLibMock=new SafecLibMock;
 g_syscfgMock = new SyscfgMock;
 g_fileIOMock = new FileIOMock;
 g_socketMock = new SocketMock;
 g_syseventMock = new SyseventMock;
 g_psmMock = new PsmMock;
 g_securewrapperMock = new SecureWrapperMock;
 g_anscMemoryMock       = new AnscMemoryMock;
 g_baseapiMock = new BaseAPIMock;
 g_anscWrapperApiMock = new AnscWrapperApiMock;
 g_traceMock = new TraceMock;
 g_usertimeMock = new UserTimeMock;
 g_CcspEthagentEthernetMock=new CcspEthagentEthernetMock;
 g_PtdHandlerMock=new PtdHandlerMock;
 g_platformHALMock = new PlatformHalMock;
 g_utopiaMock= new utopiaMock;

 }


CcspEthagentTestFixture :: ~CcspEthagentTestFixture()
{
 delete g_rbusMock;
 delete g_ethSwHALMock;
 delete g_safecLibMock;
 delete g_syscfgMock;
 delete g_fileIOMock;
 delete g_socketMock;
 delete g_syseventMock;
 delete g_psmMock;
 delete g_securewrapperMock;
 delete g_anscMemoryMock ;
 delete g_baseapiMock;
 delete g_anscWrapperApiMock;
 delete g_traceMock;
 delete g_usertimeMock;
 delete g_CcspEthagentEthernetMock;
 delete g_PtdHandlerMock;
 delete g_platformHALMock;
 delete g_utopiaMock;

  g_ethSwHALMock=nullptr;
  g_safecLibMock=nullptr;
  g_syscfgMock=nullptr;
  g_fileIOMock=nullptr;
  g_socketMock=nullptr;
  g_syseventMock=nullptr;
  g_psmMock=nullptr;
  g_securewrapperMock=nullptr;
  g_anscMemoryMock =nullptr;
  g_baseapiMock=nullptr;
  g_anscWrapperApiMock=nullptr;
  g_traceMock=nullptr;
  g_usertimeMock=nullptr;
  g_CcspEthagentEthernetMock=nullptr;
  g_PtdHandlerMock=nullptr;
  g_platformHALMock=nullptr;
  g_utopiaMock=nullptr;
  g_rbusMock=nullptr;
 }

void CcspEthagentTestFixture::SetUp() {}
void CcspEthagentTestFixture::TearDown() {}
void CcspEthagentTestFixture::TestBody() {}
#endif //CCSPETHAGENT_FIXTURE_G_MOCK
