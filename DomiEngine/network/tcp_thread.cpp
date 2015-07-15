/******************************************************************** 
创建时间:        2015/07/15 15:24
文件名称:        tcp_thread.cpp
文件作者:        Domi
*********************************************************************/

#include "network/tcp_thread.h"
#include "common/log/log.h"

CTcpThread::CTcpThread()
	: m_count(0)
	, m_clTcpServer(NULL)
	, m_base(NULL)
{
}

CTcpThread::~CTcpThread()
{
	event_base_free(m_base);
	m_clTcpServer = nullptr;
}

// 初始化
bool CTcpThread::initialize(CTcpServer* pTcpServer)
{
	if (!pTcpServer)
		return false;

	m_clTcpServer = pTcpServer;
	m_base = new_event_base();

	if (!m_base)
		return false;

	//struct timeval tTimeout = { 1, 0 };
	////回调函数的参数为bufferevent
	//event_assign(&eTimeout, m_base, -1, EV_TIMEOUT | EV_PERSIST, &CTcpThread::on_timeout, NULL);
	//evtimer_add(&eTimeout, &tTimeout);

	return true;
}

void CTcpThread::on_timeout(evutil_socket_t fd, short events, void * arg)
{
	//printf("------------\n");
	//printf("CTcpThread::on_timeout线程 %d\n", CThread::getCurrentThreadID());
}