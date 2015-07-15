/******************************************************************** 
����ʱ��:        2015/06/30 20:02
�ļ�����:        tcp_connection.cpp
�ļ�����:        Domi
*********************************************************************/

#include "network/tcp_connection.h"

CTcpConnection::CTcpConnection()
{
	m_apply_key = 0;
}

// ��ʼ��
void CTcpConnection::initialize()
{
	++m_apply_key;
	m_pContext = nullptr;
}

// �Ͽ�����
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

// �����ϵĻص�
void CTcpConnection::onConnect()
{

}

// �Ͽ�����
void CTcpConnection::onDisconnect()
{
	printf("CTcpConnection::onDisconnect��\n");
}