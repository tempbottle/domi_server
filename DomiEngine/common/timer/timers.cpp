/******************************************************************** 
创建时间:        2015/07/08 23:58
文件名称:        timers.cpp
文件作者:        Domi
*********************************************************************/

#include "common/basic/timeFunction.h"
#include "common/basic/memoryFunctions.h"
#include "common/timer/timers.h"
#include "common/log/log.h"

// 原子操作
AtomicUint64  CTimerObject::s_numCreated;

CTimer::CTimer()
{
#ifdef WIN32
	WSADATA WSAData;
	WSAStartup(0x0202, &WSAData);
#endif
	m_shutdown	= false;
	m_event_base = new_event_base();

	::memset(&m_event, 0, sizeof(m_event));
}

CTimer::~CTimer()
{
	stopTimer();

	bufferevent_disable(m_pbufferevent[0], EV_READ | EV_WRITE);
	bufferevent_free(m_pbufferevent[0]);

	if(m_event_base)
		event_base_free(m_event_base);
}

//////////////////////////////////////////////////////////////////////////
void CTimer::wakeUp()
{
	const char* pWakeUp = "wakeup";
	//if (send(m_pair[0], pWakeUp, (int)strlen(pWakeUp) + 1, 0) < 0)
	//	return;

	int ret = bufferevent_write(m_pbufferevent[0], pWakeUp, (int)strlen(pWakeUp) + 1);
	if (ret == -1)
		CLog::error("addTimer 唤醒失败……");
}

bool CTimer::loop_break()
{
	if (!m_shutdown)
		return false;

	if (m_event_base)
	{
		timeval _val = { 0, 0 };
		event_base_loopbreak(m_event_base);
	}

	return true;
}

bool CTimer::isInLoopThread()
{
	int64 curThreadId = CThread::getCurrentThreadID();
	if (!m_thread.is_runing() || (m_thread.is_runing() && m_thread.getThreadID() == curThreadId))
		return true;

	return false;
}

void CTimer::addTimerInLoop(TimerCallback cbFunc, uint64 _tick, uint32 count)
{
	try
	{
		timeval _timeout;
		evutil_timerclear(&_timeout);
		_timeout.tv_sec = (long)_tick / 1000;								//秒
		_timeout.tv_usec = (long)(_tick - _timeout.tv_sec * 1000) * 1000;	//微秒

		//避免过多消耗设置为0.1毫秒
		if (_timeout.tv_sec && _timeout.tv_usec)
			_timeout.tv_usec = 100;

		CTimerObject* pTimer = new CTimerObject(this, cbFunc, _tick, count);
		if (!pTimer){
			CLog::error("timer对象构造错误.");
			return;
		}
		event_assign(&(pTimer->m_ev_timeout), m_event_base, -1, EV_TIMEOUT | EV_PERSIST, CTimer::on_timeout, pTimer);	//绑定根基事件

		//增加事件，该操作会是event进入未决(pending)状态
		if (!event_add(&(pTimer->m_ev_timeout), &_timeout) == 0)
			return;

		//加入队列
		m_object_map[pTimer->GetSequence()] = pTimer;

		return;
	}
	catch (...)
	{
	}
	return;
}

void CTimer::delTimerInLoop(uint64 timerId)
{
	if (timerId == 0)
		return;

	MAP_TIMEROBJ::iterator it = m_object_map.find(timerId);
	if (it!=m_object_map.end()){
		int32 _evnet = event_pending(&(it->second->m_ev_timeout), EV_ALL, nullptr);
		if (_evnet & EV_TIMEOUT)	//拥有事件
		event_del(&(it->second->m_ev_timeout));

		delete it->second;
	}
	m_object_map.erase(timerId);
}

//////////////////////////////////////////////////////////////////////////
bool CTimer::startTimer()
{
	if(!m_event_base)
		return false;

	// 方式一：socket pair
	/*
	if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, m_pair) == -1)
		return false;

	event_assign(&m_event, m_event_base, m_pair[1], EV_READ | EV_PERSIST, CTimer::on_wakeup, this);	//绑定根基事件,读事件
	if (!event_add(&m_event, NULL) == 0)
		return false;
	*/

	// 方式二：bufferevent pair
	if (bufferevent_pair_new(m_event_base, BEV_OPT_THREADSAFE, m_pbufferevent) == -1)
		return false;

	bufferevent_setcb(m_pbufferevent[1], &CTimer::on_wakeup, NULL, NULL, this);
	bufferevent_enable(m_pbufferevent[1], EV_READ);
	bufferevent_enable(m_pbufferevent[0], EV_WRITE);

	m_shutdown	= false;
	//启动监听线程
	if(!m_thread.is_runing() && !m_thread.create(CTimer::_timer_thread_,this))
	{
		m_shutdown	= true;
		return false;
	}

	return true;
}

void CTimer::stopTimer()
{
	m_shutdown = true;
	loop_break();

	//等待退出
	while(true)
	{
		dSleep(10);
		
		if(!m_thread.is_runing())
			break;
	}

	m_thread.shutdown();
	clearTimer();	//删除所有timer对象
	m_pendingfunc_vec.clear();
}

void CTimer::clearTimer()
{
	// 如果调用到此函数，说明 event_base 已经退出了loop，线程结束，此时线程安全
	try
	{
		int32 _evnet = 0;
		CTimerObject* pTimer= nullptr;

		for (auto& kv:m_object_map)
		{
			pTimer = kv.second;
			int32 _evnet = event_pending(&(pTimer->m_ev_timeout), EV_ALL, nullptr);
			if (_evnet & EV_TIMEOUT)	//拥有事件
				event_del(&(pTimer->m_ev_timeout));

			delete pTimer;
		}

		m_object_map.clear();
	}
	catch (...)
	{
	}
}

// 添加定时器
uint32 CTimer::addTimer(TimerCallback cbFunc, uint64 _tick, uint32 count)
{
	if (cbFunc == nullptr)
	{
		CLog::error("回调函数未设置，无法添加到定时器.");
		return 0;
	}

	if (!_tick || !m_event_base)
		return false;

	if (isInLoopThread())
	{
		addTimerInLoop(cbFunc, _tick, count);
	}
	else
	{
		CCritLocker clLock(m_mutex);
		m_pendingfunc_vec.push_back(std::bind(&CTimer::addTimerInLoop, this, cbFunc, _tick, count));

		wakeUp();
	}

	return 0;
}

bool CTimer::delTimer(uint64 timerId)
{
	if (timerId == 0)
		return false;

	CCritLocker clLock(m_mutex);
	m_pendingfunc_vec.push_back(std::bind(&CTimer::delTimerInLoop, this, timerId));
	wakeUp();

	return true;
}

//-------------------------------------------------------------------
// staic
// socket pair方式：void CTimer::on_wakeup(evutil_socket_t fd, short events, void *arg)
// bufferevent pair方式：void CTimer::on_wakeup(struct bufferevent* bev, void *arg);
void CTimer::on_wakeup(struct bufferevent* bev, void *arg)
{
	CTimer* pTimer = (CTimer*)arg;
	if (pTimer == nullptr)
		return;
	/*
	if (EV_READ & events)
		CLog::print("%s: WakeUp!", __FUNCTION__);

	len = recv(fd, buf, sizeof(buf), 0);
	*/

	char buf[10];
	int len;
	len = bufferevent_read(bev, buf, sizeof(buf));
	if (len>0)	// wakeup
	{
		CTimer::VEC_FUNCTOR functors;
		{
			CCritLocker clLock(pTimer->get_timer_locket());
			functors.swap(pTimer->m_pendingfunc_vec);
		}

		for (size_t i = 0; i < functors.size(); ++i)
		{
			functors[i]();
		}
	}
}

// 超时事件的回调
void CTimer::on_timeout(evutil_socket_t fd,short events,void * arg)
{
	//printf("on_timeout线程 %d\n", CThread::getCurrentThreadID());
	//更新一次系统毫秒时间
	setSystemTick64();
	CTimerObject* pTimer = (CTimerObject*)arg;
	if (pTimer && pTimer->m_timer)
	{
		if (pTimer->m_timer->loop_break())	// event_base 是否还在循环
			return;

		uint64 uBegin = getTickCount64();

		if (pTimer->IsRepeat())	// 循环的一直循环
		{
			pTimer->m_timerCb();
		}
		else
		{
			if (pTimer->m_count>0)
				pTimer->m_timerCb();
	
			--pTimer->m_count;
			if (pTimer->m_count<=0)
			{
				int32 _evnet = event_pending(&(pTimer->m_ev_timeout), EV_ALL, nullptr);
				if (_evnet & EV_TIMEOUT)	//拥有事件
					event_del(&(pTimer->m_ev_timeout));
				
				pTimer->m_timer->m_object_map.erase(pTimer->GetSequence());
				delete pTimer;
			}
		}
		uint64 uEnd = getTickCount64();

		// 调用耗时
		uint64 uTick = uEnd - uBegin;
		//printf("CTimer::on_timeout(%llu->%llu)\n", pTimer->m_timeout, uTick);
		//if (uTick > 1)
		//	printf("CTimer::on_timeout(%llu->%llu)\n", pTimer->m_timeout, uTick);
	}
}

// timer线程
THREAD_RETURN CTimer::_timer_thread_(void* _param)
{
	CLog::info("[timer]线程启动,id = %d……",CThread::getCurrentThreadID());
	CTimer* _this = (CTimer*)_param;
	if(_this)
		//event_base_loop(_this->m_event_base, EVLOOP_NO_EXIT_ON_EMPTY);
		event_base_dispatch(_this->m_event_base);

	CLog::info("[timer]线程结束……");
	return 0;
}
