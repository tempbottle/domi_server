/******************************************************************** 
����ʱ��:        2015/06/22 14:44
�ļ�����:        GameServer.cpp
�ļ�����:        Domi
*********************************************************************/

#include <iostream>
#include "domiServer.h"

static void test()
{
	CLog::print("����һ��timer���� ����");
}

CDomiServer::CDomiServer()
{
#ifdef WIN32
	dSprintf(m_szEventExist, sizeof(m_szEventExist), "ExistEvent_Game");
#endif
}

CDomiServer::~CDomiServer()
{
	
}

void CDomiServer::ShowServerInfo()
{
	showToConsole("***************************************************************");
	showToConsole("* Game Server Application Ver %d.%d", 0, 0);
	showToConsole("***************************************************************");

	setServicesTitle("Game:<%.4d>", 1);
}

// ��ʼ��server
bool CDomiServer::initialize(uint16 uServerID)
{
	ShowServerInfo();

	CServerRoot::initialize(uServerID);

	return true;
}

// ����server
bool CDomiServer::startServices()
{
	CServerRoot::startServices();
	m_tcpServer.StartServer();

	CTimer::GetSingleton().addTimer(test, 100, 50);
	return true;
}

// timer�����̣߳��߼������ﴦ��
void CDomiServer::timerProcess()
{
	//CLog::print("CDomiServer::timerProcess()");
	/*
	_stNetMessage* pMsg = nullptr;
	server.m_clMessageQueue.swap_queue();
	while (pMsg = server.m_clMessageQueue.front())
	{
		if (pMsg)
		{
			printf("------------\n");
			CTcpConnection* client = server.getClient(pMsg->_context, pMsg->_apply_key);
			if (client)
			{
				client->disconnect();
				//client->setApplyLock(false);
			}
		}
		server.m_clMessageQueue.pop();
	}
	*/
}

// �ر�server
void CDomiServer::stopServices()
{
	CTimer::GetSingleton().stopTimer();
	m_tcpServer.StopServer();

	CLog::shutdown();
}

// �ܾ�server
void CDomiServer::denyServices()
{

}