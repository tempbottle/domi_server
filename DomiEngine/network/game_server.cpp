/******************************************************************** 
创建时间:        2015/07/06 16:48
文件名称:        game_server.cpp
文件作者:        Domi
*********************************************************************/

#include "network/game_server.h"

CTcpConnection*	GameServer::ConnectNew(CTcpContext* pContext)
{
	if (!pContext)
		return nullptr;

	CTcpClient* pClient = m_mapClient.allocate(pContext);
	if (pClient)
	{
		pClient->initialize();
		pClient->bindContext(pContext);
	}

	return pClient;
}
