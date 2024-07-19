#ifndef MOCK_CCSPETHAGENT_H
#define MOCK_CCSPETHAGENT_H
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "cosa_apis.h"

class EthernetInterface
{
	public:
		virtual ~EthernetInterface(){}
		virtual void  CcspHalExtSw_SendNotificationForAllHosts(void)=0;
};

class CcspEthagentEthernetMock: public EthernetInterface {
	public:
		virtual ~CcspEthagentEthernetMock(){}
		MOCK_METHOD0(CcspHalExtSw_SendNotificationForAllHosts,void(void));
};

#endif //MOCK_CCSPETHAGENT_H
