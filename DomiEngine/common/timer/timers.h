/******************************************************************** 
创建时间:        2015/07/08 23:53
文件名称:        timers.h
文件作者:        Domi
功能说明:        定时器  
其他说明:        基于libevent
*********************************************************************/

#pragma once
#include <vector>
#include <map>
#include <functional>
#include "network/libevent.h"
#include "common/basic/basicTypes.h"
#include "common/thread/csLocker.h"
#include "common/thread/thread.h"
#include "common/thread/condEvent.h"
#include "common/basic/atomic.h"
#include "common/singleton/singleton.h"

typedef std::function<void()> TimerCallback;
typedef std::function<void()> PendingFunctors;
typedef CAtomicInteger<uint64> AtomicUint64;


//////////////////////////////////////////////////////////////////////////
// 计时器对像
class CTimer;
class CTimerObject
{
private:
	TimerCallback m_timerCb;			// 回调
	static AtomicUint64 s_numCreated;	// 定时器id

public:
	CTimer*	m_timer;
	uint64	m_timeout;				//超时时间
	event	m_ev_timeout;			//超时事件
	int32   m_count;				//调用次数
	uint64  m_sequence;				//定时器序列号
	bool	m_repeat;				//是否重复循环

public:
	friend class CTimer;

	CTimerObject(CTimer* pTimer, TimerCallback cb, uint64 tick, int32 count = 0)
		: m_timer(pTimer)
		, m_timerCb(cb)
		, m_timeout(tick)
		, m_count(count)
		, m_repeat(count==0)
		, m_sequence(s_numCreated.IncrementAndGet())
	{
		::memset(&m_ev_timeout, 0, sizeof(m_ev_timeout));
	}

	virtual ~CTimerObject()
	{
		m_timer = nullptr;
		m_timerCb = nullptr;
		m_sequence = 0;

		::memset(&m_ev_timeout, 0, sizeof(m_ev_timeout));
	}

public:
	inline bool IsRepeat()	{ return m_repeat; }
	virtual const uint64 GetSequence()	{ return m_sequence; }
};

//////////////////////////////////////////////////////////////////////////
// 计时器
/*
使用libevent多线程同步的方式：
1、pipe linux有效
2、socketpair 套接字对，需要消耗一对网络套接字
3、buffereventpair bufferevent 对，不需要消耗套接字，但是有一点点延迟
*/
class CTimer :public Singleton<CTimer>
{
public:
	typedef std::map<uint64, CTimerObject*> MAP_TIMEROBJ;
	typedef std::vector<PendingFunctors> VEC_FUNCTOR;

	CThread			m_thread;			// 定时器线程
	CCondEvent		m_exit_event;
	event_base*		m_event_base;
	event			m_event;			// 事件
	//evutil_socket_t m_pair[2];		// 用来做线程同步通知
	bufferevent*	m_pbufferevent[2];	// 线程同步
	bool			m_shutdown;
	//bool			m_isCalling;		// wake正在调用
	CMutex			m_mutex;
	MAP_TIMEROBJ	m_object_map;		// timer对象
	VEC_FUNCTOR		m_pendingfunc_vec;	// loop内回调函数集合

public:
	CTimer();
	virtual ~CTimer();

private:
	bool	loop_break();
	void	wakeUp();
	bool	isInLoopThread();
	void	addTimerInLoop(TimerCallback cbFunc, uint64 _tick, uint32 count);
	void	delTimerInLoop(uint64 timerId);

public:
	inline CMutex*	get_timer_locket()	{ return &m_mutex; }

	bool	startTimer();
	void	stopTimer();
	void	clearTimer();
	uint32  addTimer(TimerCallback cbFunc, uint64 _tick, uint32 count = 0);
	bool	delTimer(uint64 timerId);

public:
	static void				on_wakeup(struct bufferevent* bev, void *arg);
	static void				on_timeout(evutil_socket_t fd,short events,void * arg);
	static THREAD_RETURN	_timer_thread_(void* _param);
};