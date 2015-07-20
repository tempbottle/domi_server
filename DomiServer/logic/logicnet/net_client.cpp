/******************************************************************** 
创建时间:        2015/07/16 19:42
文件名称:        tcp_client.cpp
文件作者:        Domi
*********************************************************************/

#include "logic/logicnet/net_client.h"

void CNetClient::onDisconnect()
{
	//player.logout();

	// 不要在网络io线程处理耗时的上层逻辑
	/*if (m_pPlayer)
	{
	m_pPlayer->logout();
	}*/
	
	//printf("CNetClient::onDisconnect！\n");
	initialize();	// 清0，用来做复用
}

void CNetClient::initialize()
{
	CTcpConnection::initialize();

	// 其他的一些上层数据的清空
}
