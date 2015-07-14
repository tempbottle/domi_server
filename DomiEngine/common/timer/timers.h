/******************************************************************** 
����ʱ��:        2015/07/08 23:53
�ļ�����:        timers.h
�ļ�����:        Domi
����˵��:        ��ʱ��  
����˵��:        ����libevent
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
// ��ʱ������
class CTimer;
class CTimerObject
{
private:
	TimerCallback m_timerCb;			// �ص�
	static AtomicUint64 s_numCreated;	// ��ʱ��id

public:
	CTimer*	m_timer;
	uint64	m_timeout;				//��ʱʱ��
	event	m_ev_timeout;			//��ʱ�¼�
	int32   m_count;				//���ô���
	uint64  m_sequence;				//��ʱ�����к�
	bool	m_repeat;				//�Ƿ��ظ�ѭ��

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
// ��ʱ��
/*
ʹ��libevent���߳�ͬ���ķ�ʽ��
1��pipe linux��Ч
2��socketpair �׽��ֶԣ���Ҫ����һ�������׽���
3��buffereventpair bufferevent �ԣ�����Ҫ�����׽��֣�������һ����ӳ�
*/
class CTimer :public Singleton<CTimer>
{
public:
	typedef std::map<uint64, CTimerObject*> MAP_TIMEROBJ;
	typedef std::vector<PendingFunctors> VEC_FUNCTOR;

	CThread			m_thread;			// ��ʱ���߳�
	CCondEvent		m_exit_event;
	event_base*		m_event_base;
	event			m_event;			// �¼�
	//evutil_socket_t m_pair[2];		// �������߳�ͬ��֪ͨ
	bufferevent*	m_pbufferevent[2];	// �߳�ͬ��
	bool			m_shutdown;
	//bool			m_isCalling;		// wake���ڵ���
	CMutex			m_mutex;
	MAP_TIMEROBJ	m_object_map;		// timer����
	VEC_FUNCTOR		m_pendingfunc_vec;	// loop�ڻص���������

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