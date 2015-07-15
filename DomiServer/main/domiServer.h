/******************************************************************** 
����ʱ��:        2015/07/10 15:50
�ļ�����:        domiServer.h
�ļ�����:        Domi
����˵��:        ��Ϸ������  
����˵��:         
*********************************************************************/

#include "common/singleton/singleton.h"
#include "common/serverroot/serverRoot.h"
#include "logic/logicnet/net_server.h"

class CDomiServer : public Singleton<CDomiServer>
				  , public CServerRoot
{
public:
	CDomiServer();
	~CDomiServer();

public:
	void ShowServerInfo();

	bool CDomiServer::initialize(uint16 uServerID);	// ��ʼ��server
	bool CDomiServer::startServices();				// ����server	
	void CDomiServer::timerProcess();				// timer�����̣߳��߼������ﴦ��
	void CDomiServer::stopServices();				// �ر�server
	void CDomiServer::denyServices();				// �ܾ�server

public:
	CNetServer m_netServer;		// �������ӷ����
};
