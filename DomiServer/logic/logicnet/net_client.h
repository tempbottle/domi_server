/******************************************************************** 
����ʱ��:        2015/07/06 16:46
�ļ�����:        tcp_client.h
�ļ�����:        Domi
����˵��:        ��װ�Ŀͻ�������
����˵��:        ע�⣺����ֻ�ǳ�����������ӣ���Ҫ���ϲ��߼��ľ������
				 ���磺gameserver��player������Ҫ������������ϡ�
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
	void bindPlayer();		// ����Ҷ��󣬻�����������
private:
};