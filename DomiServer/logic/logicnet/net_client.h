/******************************************************************** 
创建时间:        2015/07/06 16:46
文件名称:        tcp_client.h
文件作者:        Domi
功能说明:        封装的客户端连接
其他说明:        
*********************************************************************/

#pragma once
#include "network/tcp_connection.h"

class CNetClient : public CTcpConnection
{
public:
	CNetClient() {}
	~CNetClient() {}

public:
	void initialize();		// 初始化
	void onDisconnect();	// 断开连接的回调，这个时候可以做玩家下线处理

private:
};