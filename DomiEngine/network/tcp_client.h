/******************************************************************** 
����ʱ��:        2015/07/06 16:46
�ļ�����:        tcp_client.h
�ļ�����:        Domi
����˵��:        �ͻ�������
����˵��:         
*********************************************************************/

#pragma once
#include "network/tcp_connection.h"

class CTcpClient : public CTcpConnection
{
public:
	CTcpClient() {}
	~CTcpClient() {}

public:
	void initialize();		// ��ʼ��
	void onDisconnect();	// �Ͽ����ӵĻص������ʱ�������������ߴ���

private:
};