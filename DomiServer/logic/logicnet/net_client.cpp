/******************************************************************** 
����ʱ��:        2015/07/16 19:42
�ļ�����:        tcp_client.cpp
�ļ�����:        Domi
*********************************************************************/

#include "logic/logicnet/net_client.h"

void CNetClient::onDisconnect()
{
	//player.logout();

	// ��Ҫ������io�̴߳����ʱ���ϲ��߼�
	/*if (m_pPlayer)
	{
	m_pPlayer->logout();
	}*/
	
	//printf("CNetClient::onDisconnect��\n");
	initialize();	// ��0������������
}

void CNetClient::initialize()
{
	CTcpConnection::initialize();

	// ������һЩ�ϲ����ݵ����
}
