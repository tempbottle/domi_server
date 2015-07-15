/******************************************************************** 
创建时间:        2015/07/10 15:50
文件名称:        domiServer.h
文件作者:        Domi
功能说明:        游戏服务器  
其他说明:         
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

	bool CDomiServer::initialize(uint16 uServerID);	// 初始化server
	bool CDomiServer::startServices();				// 启动server	
	void CDomiServer::timerProcess();				// timer工作线程，逻辑在这里处理
	void CDomiServer::stopServices();				// 关闭server
	void CDomiServer::denyServices();				// 拒绝server

public:
	CNetServer m_netServer;		// 网络连接服务端
};
