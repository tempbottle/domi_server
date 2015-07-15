/******************************************************************** 
创建时间:        2015/06/22 20:04
文件名称:        tcp_thread.h
文件作者:        Domi
功能说明:        tcp线程对像  
其他说明:         
*********************************************************************/

#pragma once
#include <vector>
#include <functional>
#include "common/thread/thread.h"
#include "network/libevent.h"

class CTcpServer;
class CTcpThread : public CThread
{
public:
	CTcpThread();
	~CTcpThread();

public:
	inline void	enter_thread() { m_count++; }
	inline void	exit_thread	() { if(m_count) m_count--;}

	bool initialize(CTcpServer* pTcpServer);		// 初始化

	static void on_timeout(evutil_socket_t fd, short events, void * arg);

public:
	uint64		 m_count;			// 被引用次数
	CTcpServer*	 m_clTcpServer;		// 归属对像
	event_base*	 m_base;			// 根基事件
	bufferevent* m_pbufferevent[2];	// 线程同步
	struct event eTimeout;
};

