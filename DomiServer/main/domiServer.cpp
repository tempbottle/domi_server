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
	m_netServer.StartServer();

	CTimer::GetSingleton().addTimer(test, 100, 10);
	return true;
}

// timer�����̣߳��߼������ﴦ��
void CDomiServer::timerProcess()
{
	static int test = 0;
	// ������Ϣ
	_stNetMessage* pMsg = nullptr;
	m_netServer.m_clMessageQueue.swap_queue();
	while (pMsg = m_netServer.m_clMessageQueue.front())
	{
		if (pMsg)
		{
			CTcpConnection* client = m_netServer.getClient(pMsg->_context, pMsg->_apply_key);
			if (client)
			{
				//client->disconnect();
				//printf("------------> %s\n",pMsg->m_buffer);
				//std::cout << pMsg->_context << std::endl;
				/*++test;
				if (test>=10)
				{
					client->disconnect();
				}*/
				client->setApplyLock(false);
			}
		}
		m_netServer.m_clMessageQueue.pop();
	}
}

// �ر�server
void CDomiServer::stopServices()
{
	CTimer::GetSingleton().stopTimer();
	m_netServer.StopServer();

	CLog::shutdown();
}

// �ܾ�server
void CDomiServer::denyServices()
{

}