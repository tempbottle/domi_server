/******************************************************************** 
����ʱ��:        2015/07/06 16:46
�ļ�����:        tcp_client.h
�ļ�����:        Domi
����˵��:        ��װ�Ŀͻ�������
����˵��:        
*********************************************************************/

#pragma once
#include "network/tcp_connection.h"

class CNetClient : public CTcpConnection
{
public:
	CNetClient() {}
	~CNetClient() {}

public:
	void initialize();		// ��ʼ��
	void onDisconnect();	// �Ͽ����ӵĻص������ʱ�������������ߴ���

private:
};