/******************************************************************** 
����ʱ��:        2015/06/22 14:44
�ļ�����:        GameServer.cpp
�ļ�����:        Domi
*********************************************************************/

#include <iostream>
#include "domiServer.h"

// ������Ϣ
static uint64 STnowTime = getSecond();
static uint32 STnum = 0;
static uint32 STCount = 0;

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
	m_netServer.Initialize(2,17777);

	return true;
}

// ����server
bool CDomiServer::startServices()
{
	CServerRoot::startServices();
	m_netServer.StartServer();

	//CTimer::GetSingleton().addTimer(test, 100, 10);
	return true;
}

// timer�����̣߳��߼������ﴦ��
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

// �ܷ��˳�,������������ز���
bool CDomiServer::canExitServices()
{
	// �������
	return true;
}

// �ر�server
void CDomiServer::stopServices()
{
	// ���ֵײ��shutdown
	CLog::shutdown();
}

// �ܾ�server
void CDomiServer::denyServices()
{
	showToConsole("<Deny service ...>");

	CLog::warn("��ʼ�ܾ�������������...");
	m_netServer.StopServer();

	//g_pGamePlayerMgr->shutdown();
	// �����߼���Ĺر��Լ��������

	showToConsole("<Waiting for Queue process ...>");
}