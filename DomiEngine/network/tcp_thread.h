/******************************************************************** 
����ʱ��:        2015/06/22 20:04
�ļ�����:        tcp_thread.h
�ļ�����:        Domi
����˵��:        tcp�̶߳���  
����˵��:         
*********************************************************************/

#pragma once
#include "common/thread/thread.h"
#include "common/thread/condEvent.h"
#include "libevent.h"

class CTcpServer;
class CTcpThread : public CThread
{
public:
	uint64		m_count;			//�����ô���
	CCondEvent*	m_exit;				//���¼�,��������
	CTcpServer* m_clTcpServer;		//��������
	event_base* m_base;				//�����¼�

public:
	inline void	enter_thread() { m_count++; }
	inline void	exit_thread	() { if(m_count) m_count--;}

public:
	CTcpThread()
	{
		m_count	= 0;
		m_exit	= NULL;
		m_clTcpServer = NULL;
		m_base	= NULL;
	}
};