/******************************************************************** 
创建时间:        2015/06/22 21:07
文件名称:        libevent.h
文件作者:        Domi
功能说明:        libevent 接口封装  
其他说明:         
*********************************************************************/

#pragma once
#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/listener.h"
#include "event2/thread.h"
#include "event2/bufferevent.h"
#include "event2/bufferevent_compat.h"
#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/util.h>
#ifdef WIN32
//#include "iocp-internal.h"
#endif//WIN32

#define EV_ALL	(EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL | EV_PERSIST | EV_ET)

//-------------------------------------------------------------------
//根据系统创建IOCP/Epool事件
inline event_base* new_event_base()
{
	struct event_base *base = NULL;
	struct event_config *cfg = event_config_new();
	if (cfg)
	{
#ifdef WIN32
		evthread_use_windows_threads();
		event_config_set_flag(cfg,EVENT_BASE_FLAG_STARTUP_IOCP);
#else
		evthread_use_pthreads();
		event_config_set_flag(cfg, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);
#endif 

		base = event_base_new_with_config(cfg);
		event_config_free(cfg);
	}
	return base;
}
