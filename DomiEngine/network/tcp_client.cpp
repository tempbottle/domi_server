/******************************************************************** 
创建时间:        2015/07/16 19:42
文件名称:        tcp_client.cpp
文件作者:        Domi
*********************************************************************/

#include "tcp_client.h"

void CTcpClient::onDisconnect()
{
	//player.logout();

	/*if (m_pPlayer)
	{
	m_pPlayer->logout();
	}*/

	initialize();	// 清0，用来做复用
}

void CTcpClient::initialize()
{
	CTcpConnection::initialize();

	// 其他的一些上层数据的清空
}

