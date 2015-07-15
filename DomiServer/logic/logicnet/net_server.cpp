/******************************************************************** 
创建时间:        2015/07/06 16:48
文件名称:        game_server.cpp
文件作者:        Domi
*********************************************************************/

#include "logic/logicnet/net_server.h"

CTcpConnection*	CNetServer::ConnectNew(CTcpContext* pContext)
{
	if (!pContext)
		return nullptr;

	CNetClient* pClient = m_mapClient.allocate(pContext);
	if (pClient)
	{
		pClient->initialize();
		pClient->bindContext(pContext);
	}

	return pClient;
}
