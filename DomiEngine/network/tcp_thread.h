/******************************************************************** 
����ʱ��:        2015/06/22 20:04
�ļ�����:        tcp_thread.h
�ļ�����:        Domi
����˵��:        tcp�̶߳���  
����˵��:         
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

	bool initialize(CTcpServer* pTcpServer);		// ��ʼ��

	static void on_timeout(evutil_socket_t fd, short events, void * arg);

public:
	uint64		 m_count;			// �����ô���
	CTcpServer*	 m_clTcpServer;		// ��������
	event_base*	 m_base;			// �����¼�
	bufferevent* m_pbufferevent[2];	// �߳�ͬ��
	struct event eTimeout;
};

