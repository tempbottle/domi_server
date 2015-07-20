/******************************************************************** 
创建时间:        2015/06/22 14:44
文件名称:        GameServer.cpp
文件作者:        Domi
*********************************************************************/

#include <iostream>
#include "domiServer.h"

// 处理消息
static uint64 STnowTime = getSecond();
static uint32 STnum = 0;
static uint32 STCount = 0;

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
	m_netServer.Initialize(2,17777);

	return true;
}

// 启动server
bool CDomiServer::startServices()
{
	CServerRoot::startServices();
	m_netServer.StartServer();

	//CTimer::GetSingleton().addTimer(test, 100, 10);
	return true;
}

// timer工作线程，逻辑在这里处理
void CDomiServer::timerProcess()
{
	_stNetMessage* pMsg = nullptr;
	m_netServer.m_clMessageQueue.swap_queue();
	while (pMsg = m_netServer.m_clMessageQueue.front()){
		if (pMsg){
			CTcpConnection* client = m_netServer.getClient(pMsg->_context, pMsg->_apply_key);
			if (client){
				++STnum;
				++STCount;
				client->SendMsg(pMsg);
				//client->setApplyLock(false);
			}
		}
		m_netServer.m_clMessageQueue.pop();
	}

	if ((getSecond() - STnowTime)>=60){
		CLog::error("tps = %d,%d", STnum / 60, STCount);
		STnowTime = getSecond();
		STnum = 0;
	}
}

// 能否退出,这里做数据落地操作
bool CDomiServer::canExitServices()
{
	// 数据落地
	return true;
}

// 关闭server
void CDomiServer::stopServices()
{
	// 各种底层的shutdown
	CLog::shutdown();
}

// 拒绝server
void CDomiServer::denyServices()
{
	showToConsole("<Deny service ...>");

	CLog::warn("开始拒绝世界网络链接...");
	m_netServer.StopServer();

	//g_pGamePlayerMgr->shutdown();
	// 各种逻辑层的关闭以及数据落地

	showToConsole("<Waiting for Queue process ...>");
}