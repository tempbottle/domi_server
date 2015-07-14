/******************************************************************** 
����ʱ��:        2015/06/28 18:41
�ļ�����:        condEvent.cpp
�ļ�����:        Domi
*********************************************************************/

#ifndef WIN32
#include <sys/time.h> 
#endif//WIN32
#include "common/thread/condEvent.h"

CCondEvent::CCondEvent()
{
	m_bCreated		= false;
#ifdef WIN32
	m_hEvent		= INVALID_HANDLE_VALUE;
#else//linux
	m_manual_reset	= false;
	m_signaled		= false;
#endif//WIN32
}

CCondEvent::~CCondEvent()
{
	close_event();
}

// �����¼�
// @bManualReset  �Ƿ��ֶ���ԭ�¼�  true=��Ҫ�ֶ���ԭ  false= ���̱߳��ͷź��¼�����ԭ�����ź�״̬
// @bInitialState ָ���¼�����ĳ�ʼ״̬�����ΪTRUE����ʼ״̬Ϊ���ź�״̬������Ϊ���ź�״̬��
bool CCondEvent::create_event(bool bManualReset,bool bInitialState)
{
	close_event();

#ifdef WIN32
	m_hEvent = ::CreateEvent(nullptr,bManualReset,bInitialState,nullptr);
	if (m_hEvent == INVALID_HANDLE_VALUE)
		return false;

	::ResetEvent(m_hEvent);
#else//linux
	if(pthread_cond_init(&m_pthread_cond,nullptr) != 0)
		return false;

	m_manual_reset	= bManualReset;
	m_signaled		= bInitialState;
#endif//WIN32
	
	m_bCreated = true;
	return true;
}

// �ر��¼�
void CCondEvent::close_event()
{
	if(!m_bCreated)
		return;

#ifdef WIN32
	::CloseHandle(m_hEvent);
	m_hEvent = INVALID_HANDLE_VALUE;
#else//linux
	pthread_cond_destroy(&m_pthread_cond);
#endif//WIN32

	m_bCreated = false;
}

// �����¼�
void CCondEvent::set_event()
{
	if(!m_bCreated)
		return;

#ifdef WIN32
	::SetEvent(m_hEvent);	// ����Ϊ���ź�״̬
#else//linux
	CCritLocker clLock(m_clLocker);
	if(m_manual_reset)
		pthread_cond_broadcast(&m_pthread_cond);//������������߳�
	else
		pthread_cond_signal(&m_pthread_cond);//��������ض��߳�
	
	m_signaled = true;
#endif//WIN32
}

// ��λ�¼�
void CCondEvent::reset_event()
{
	if(!m_bCreated)
		return;

#ifdef WIN32
	::ResetEvent(m_hEvent);		// ��ԭ�¼������ź�״̬
#else//linux
	CCritLocker clLock(m_clLocker);
	m_signaled = false;

#endif//WIN32
}

// �ȴ��¼�[����false��ʾ��ʱ]
bool CCondEvent::wait_event(uint64 milliseconds)
{
	if(!m_bCreated)
		return true;

#ifdef WIN32
	if(::WaitForSingleObject(m_hEvent,(DWORD)milliseconds) == WAIT_TIMEOUT)	// �ȴ��źţ�û�о�һֱ����
		return false;
#else//linux
	//��Ҫʹ��lock wait unlock����ʹ��
	CCritLocker clLock(m_clLocker);
	if(m_signaled)
	{
		//����Ҫ�ֶ���λ
		if (!m_manual_reset)
			m_signaled = false;

		return true;
	}

	//δ���ó�ʱʱ��
	if(!milliseconds)
		return false;
	timeval _now;
	gettimeofday(&_now,nullptr);

	timespec _abstime;
	_abstime.tv_sec = _now.tv_sec + milliseconds / 1000 + (((milliseconds % 1000) * 1000 + _now.tv_usec) / 1000000);
	_abstime.tv_nsec= (((milliseconds % 1000) * 1000 + _now.tv_usec) % 1000000) * 1000;

	int ret = 0;
	do
	{
		if(milliseconds != INFINITE)
			ret = pthread_cond_timedwait(&m_pthread_cond,m_clLocker.get_mutex(),&_abstime);
		else
			ret = pthread_cond_wait(&m_pthread_cond,m_clLocker.get_mutex());

	} while (ret == 0 && !m_signaled);

	//��ʱ
	if(ret == ETIMEDOUT)
		return false;

#endif//WIN32

	return true;
}


/*TODO::������������
 ��������ʼ���뻥����һ��ʹ��
�������Ϊ�٣��߳�ͨ�������������������������ԭ�ӷ�ʽ�ͷŵȴ������仯�Ļ������������һ���̸߳��������������߳̿��ܻ�����ص��������������źţ��Ӷ�ʹһ�������ȴ����߳�ִ�����²�����
	����
	�ٴλ�ȡ������
	������������

 ����������£����������������ڽ���֮��ͬ���̣߳�
	�߳����ڿ���д����ڴ��з����
	�ڴ���Э�����̹���

�����������Ա��������� pthread_condattr_destroy ���³�ʼ�����������
*/
/*
��ʼ��������������	int	pthread_condattr_init(pthread_condattr_t *cattr);//cattr��Χ���ܵ�ֵΪ PTHREAD_PROCESS_PRIVATE[ȱʡֵ] �� PTHREAD_PROCESS_SHARED
ɾ��������������	int	pthread_condattr_destroy(pthread_condattr_t *cattr);
�������������ķ�Χ	int	pthread_condattr_setpshared(pthread_condattr_t *cattr,int pshared);//PTHREAD_PROCESS_SHARED/PTHREAD_PROCESS_PRIVATE
��ȡ���������ķ�Χ	int	pthread_condattr_getpshared(const pthread_condattr_t *cattr,int *pshared);
*/
/*
Solaris			POSIX							���� 
USYNC_PROCESS	PTHREAD_PROCESS_SHARED			����ͬ���ý��̺����������е��߳� 
USYNC_THREAD	PTHREAD_PROCESS_PRIVATE			���ڽ�ͬ���ý����е��߳�
*/

/*TODO:ʹ����������
��ʼ����������			int	pthread_cond_init(pthread_cond_t *cv,const pthread_condattr_t *cattr);//cattr��ΪNULL=ȱʡ
����������������		int	pthread_cond_wait(pthread_cond_t *cv,pthread_mutex_t *mutex);//�������߳̿���ͨ�� pthread_cond_signal() �� pthread_cond_broadcast() ���ѣ�Ҳ�������źŴ��ͽ����ж�ʱ����
��ָ����ʱ��֮ǰ����	int	pthread_cond_timedwait(pthread_cond_t *cv,pthread_mutex_t *mp, const struct timespec *abstime);//ETIMEDOUT=��ʱ
��ָ����ʱ����������	int pthread_cond_reltimedwait_np(pthread_cond_t *cv,pthread_mutex_t *mp,const struct timespec *reltime);
��������ض��߳�		int	pthread_cond_signal(pthread_cond_t *cv);
������������߳�		int	pthread_cond_broadcast(pthread_cond_t *cv);
������������״̬		int	pthread_cond_destroy(pthread_cond_t *cv);

******
���Ѷ�ʧ����
1 һ���̵߳��� pthread_cond_signal() �� pthread_cond_broadcast() 
2 ��һ���߳��Ѿ������˸�������������δ���� pthread_cond_wait() 
3 û�����ڵȴ����߳�
4 �źŲ������ã���˽��ᶪʧ
*/
