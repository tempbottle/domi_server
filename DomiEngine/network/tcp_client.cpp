/******************************************************************** 
����ʱ��:        2015/07/16 19:42
�ļ�����:        tcp_client.cpp
�ļ�����:        Domi
*********************************************************************/

#include "tcp_client.h"

void CTcpClient::onDisconnect()
{
	//player.logout();

	/*if (m_pPlayer)
	{
	m_pPlayer->logout();
	}*/

	initialize();	// ��0������������
}

void CTcpClient::initialize()
{
	CTcpConnection::initialize();

	// ������һЩ�ϲ����ݵ����
}

