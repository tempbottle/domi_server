/******************************************************************** 
����ʱ��:        2015/06/28 18:36
�ļ�����:        condEvent.h
�ļ�����:        Domi
����˵��:        �¼�����
����˵��:         
*********************************************************************/

#pragma once
#include "common/basic/basicTypes.h"
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else//linux
#include <sys/types.h>//ϵͳ���Ͷ���
#include <signal.h>
#endif//WIN32
#include "csLocker.h"

#ifndef INFINITE
#define INFINITE	0xFFFFFFFF
#endif

class CCondEvent
{
protected:
	bool m_bCreated;	// �¼������Ƿ��Ѵ���
#ifdef WIN32
	HANDLE				m_hEvent;	//win32��Ϊ�¼����
#else//linux
	//linux��ʹ�û������������������
	pthread_cond_t		m_pthread_cond;
	CMutex				m_clLocker;
	bool				m_manual_reset;	//�ֶ���ʼ��
	bool				m_signaled;		//�Ƿ��Ѵ����ź�
#endif//WIN32

public:
	CCondEvent();
	~CCondEvent();

public:
	// �¼��Ƿ���Ч
	inline bool	isValid() { return m_bCreated; }

	bool create_event(bool bManualReset,bool bInitialState);	// �����¼�
	void close_event();	// �ر��¼�
	void set_event();	// �����¼�
	void reset_event();	// ��λ�¼�
	bool wait_event(uint64 milliseconds);	// �ȴ��¼�[����false��ʾ��ʱ]
};