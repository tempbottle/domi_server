/******************************************************************** 
创建时间:        2015/06/22 14:44
文件名称:        GameServer.cpp
文件作者:        Domi
*********************************************************************/

#include <iostream>
#include "domiServer.h"

static void test()
{
	CLog::print("这是一个timer测试 ……");
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

// 初始化server
bool CDomiServer::initialize(uint16 uServerID)
{
	ShowServerInfo();

	CServerRoot::initialize(uServerID);

	return true;
}

// 启动server
bool CDomiServer::startServices()
{
	CServerRoot::startServices();
	m_tcpServer.StartServer();

	CTimer::GetSingleton().addTimer(test, 100, 50);
	return true;
}

// timer工作线程，逻辑在这里处理
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

// 关闭server
void CDomiServer::stopServices()
{
	CTimer::GetSingleton().stopTimer();
	m_tcpServer.StopServer();

	CLog::shutdown();
}

// 拒绝server
void CDomiServer::denyServices()
{

}