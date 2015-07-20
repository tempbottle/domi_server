/******************************************************************** 
����ʱ��:        2015/06/22 14:07
�ļ�����:        main.cpp
�ļ�����:        Domi
����˵��:        main   
����˵��:         
*********************************************************************/

#include <iostream>
#include "domiServer.h"

int main(int argc, char* argv[])
{
	if (!CDomiServer::GetSingleton().initialize(7))
	{
		CServerRoot::messageBoxOK("CGameServer", "CGameServer::Initialize()ʧ��!");
		return 0;
	}
	
	CLog::info("���߳�id = [%d]", CTcpThread::getCurrentThreadID());
	if (CDomiServer::GetSingleton().startServices())
		CDomiServer::GetSingleton().maintenances();
	else
		CServerRoot::messageBoxOK("��������ʧ��", ":StartServices()��������ʧ��!");

	CDomiServer::GetSingleton().stopServices();
	return 0;
}