/******************************************************************** 
����ʱ��:        2015/07/08 23:58
�ļ�����:        timers.cpp
�ļ�����:        Domi
*********************************************************************/

#include "common/basic/timeFunction.h"
#include "common/basic/memoryFunctions.h"
#include "common/timer/timers.h"
#include "common/log/log.h"

// ԭ�Ӳ���
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
		CLog::error("addTimer ����ʧ�ܡ���");
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
		_timeout.tv_sec = (long)_tick / 1000;								//��
		_timeout.tv_usec = (long)(_tick - _timeout.tv_sec * 1000) * 1000;	//΢��

		//���������������Ϊ0.1����
		if (_timeout.tv_sec && _timeout.tv_usec)
			_timeout.tv_usec = 100;

		CTimerObject* pTimer = new CTimerObject(this, cbFunc, _tick, count);
		if (!pTimer){
			CLog::error("timer���������.");
			return;
		}
		event_assign(&(pTimer->m_ev_timeout), m_event_base, -1, EV_TIMEOUT | EV_PERSIST, CTimer::on_timeout, pTimer);	//�󶨸����¼�

		//�����¼����ò�������event����δ��(pending)״̬
		if (!event_add(&(pTimer->m_ev_timeout), &_timeout) == 0)
			return;

		//�������
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
		if (_evnet & EV_TIMEOUT)	//ӵ���¼�
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

	// ��ʽһ��socket pair
	/*
	if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, m_pair) == -1)
		return false;

	event_assign(&m_event, m_event_base, m_pair[1], EV_READ | EV_PERSIST, CTimer::on_wakeup, this);	//�󶨸����¼�,���¼�
	if (!event_add(&m_event, NULL) == 0)
		return false;
	*/

	// ��ʽ����bufferevent pair
	if (bufferevent_pair_new(m_event_base, BEV_OPT_THREADSAFE, m_pbufferevent) == -1)
		return false;

	bufferevent_setcb(m_pbufferevent[1], &CTimer::on_wakeup, NULL, NULL, this);
	bufferevent_enable(m_pbufferevent[1], EV_READ);
	bufferevent_enable(m_pbufferevent[0], EV_WRITE);

	m_shutdown	= false;
	//���������߳�
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

	//�ȴ��˳�
	while(true)
	{
		dSleep(10);
		
		if(!m_thread.is_runing())
			break;
	}

	m_thread.shutdown();
	clearTimer();	//ɾ������timer����
	m_pendingfunc_vec.clear();
}

void CTimer::clearTimer()
{
	// ������õ��˺�����˵�� event_base �Ѿ��˳���loop���߳̽�������ʱ�̰߳�ȫ
	try
	{
		int32 _evnet = 0;
		CTimerObject* pTimer= nullptr;

		for (auto& kv:m_object_map)
		{
			pTimer = kv.second;
			int32 _evnet = event_pending(&(pTimer->m_ev_timeout), EV_ALL, nullptr);
			if (_evnet & EV_TIMEOUT)	//ӵ���¼�
				event_del(&(pTimer->m_ev_timeout));

			delete pTimer;
		}

		m_object_map.clear();
	}
	catch (...)
	{
	}
}

// ��Ӷ�ʱ��
uint32 CTimer::addTimer(TimerCallback cbFunc, uint64 _tick, uint32 count)
{
	if (cbFunc == nullptr)
	{
		CLog::error("�ص�����δ���ã��޷���ӵ���ʱ��.");
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
// socket pair��ʽ��void CTimer::on_wakeup(evutil_socket_t fd, short events, void *arg)
// bufferevent pair��ʽ��void CTimer::on_wakeup(struct bufferevent* bev, void *arg);
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

// ��ʱ�¼��Ļص�
void CTimer::on_timeout(evutil_socket_t fd,short events,void * arg)
{
	//printf("on_timeout�߳� %d\n", CThread::getCurrentThreadID());
	//����һ��ϵͳ����ʱ��
	setSystemTick64();
	CTimerObject* pTimer = (CTimerObject*)arg;
	if (pTimer && pTimer->m_timer)
	{
		if (pTimer->m_timer->loop_break())	// event_base �Ƿ���ѭ��
			return;

		uint64 uBegin = getTickCount64();

		if (pTimer->IsRepeat())	// ѭ����һֱѭ��
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
				if (_evnet & EV_TIMEOUT)	//ӵ���¼�
					event_del(&(pTimer->m_ev_timeout));
				
				pTimer->m_timer->m_object_map.erase(pTimer->GetSequence());
				delete pTimer;
			}
		}
		uint64 uEnd = getTickCount64();

		// ���ú�ʱ
		uint64 uTick = uEnd - uBegin;
		//printf("CTimer::on_timeout(%llu->%llu)\n", pTimer->m_timeout, uTick);
		//if (uTick > 1)
		//	printf("CTimer::on_timeout(%llu->%llu)\n", pTimer->m_timeout, uTick);
	}
}

// timer�߳�
THREAD_RETURN CTimer::_timer_thread_(void* _param)
{
	CLog::info("[timer]�߳�����,id = %d����",CThread::getCurrentThreadID());
	CTimer* _this = (CTimer*)_param;
	if(_this)
		//event_base_loop(_this->m_event_base, EVLOOP_NO_EXIT_ON_EMPTY);
		event_base_dispatch(_this->m_event_base);

	CLog::info("[timer]�߳̽�������");
	return 0;
}
