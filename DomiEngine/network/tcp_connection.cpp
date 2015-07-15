/******************************************************************** 
创建时间:        2015/06/30 20:02
文件名称:        tcp_connection.cpp
文件作者:        Domi
*********************************************************************/

#include "network/tcp_connection.h"

CTcpConnection::CTcpConnection()
{
	m_apply_key = 0;
}

// 初始化
void CTcpConnection::initialize()
{
	++m_apply_key;
	m_pContext = nullptr;
}

// 断开连接
void CTcpConnection::disconnect()
{
	if (m_pContext)
		m_pContext->disconnect();
}

void CTcpConnection::onKickOut()
{

}

pc_str CTcpConnection::remote_ip()
{
	return nullptr;
}

ulong CTcpConnection::remote_address()
{
	return 0;
}

// 连接上的回调
void CTcpConnection::onConnect()
{

}

// 断开连接
void CTcpConnection::onDisconnect()
{
	printf("CTcpConnection::onDisconnect！\n");
}