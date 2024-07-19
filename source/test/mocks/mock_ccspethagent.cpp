
#include "mocks/mock_ccspethagent.h"

using namespace std;

extern CcspEthagentEthernetMock *g_CcspEthagentEthernetMock;

extern "C" void CcspHalExtSw_SendNotificationForAllHosts( void ) 
{
	if(!g_CcspEthagentEthernetMock)
        {
                return ;
        }
        g_CcspEthagentEthernetMock->CcspHalExtSw_SendNotificationForAllHosts();
}

