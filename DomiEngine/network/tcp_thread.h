/******************************************************************** 
创建时间:        2015/06/22 20:04
文件名称:        tcp_thread.h
文件作者:        Domi
功能说明:        tcp线程对像  
其他说明:         
*********************************************************************/

#pragma once
#include "common/thread/thread.h"
#include "common/thread/condEvent.h"
#include "libevent.h"

class CTcpServer;
class CTcpThread : public CThread
{
public:
	uint64		m_count;			//被引用次数
	CCondEvent*	m_exit;				//退事件,条件变量
	CTcpServer* m_clTcpServer;		//归属对像
	event_base* m_base;				//根基事件

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