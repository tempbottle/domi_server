/******************************************************************** 
����ʱ��:        2015/07/09 10:35
�ļ�����:        serverRoot.h
�ļ�����:        Domi
����˵��:        server ����  
����˵��:        
*********************************************************************/

#pragma once
#include <signal.h>
#include "common/timer/timers.h"
#include "common/thread/thread.h"
#include "common/thread/condEvent.h"
#include "common/log/log.h"
#include "common/basic/stringFunctions.h"

class CServerRoot
{
public:
	CServerRoot();
	virtual ~CServerRoot();

private:
	bool initEvent();				// ��ʼ���¼�
	void installBreakHandlers();	// �����źŲ�׽
	virtual bool loadConfig();		// �ӱ��������ļ�Config.ini��ȡ����

public:
	// ��ȡ������id����Ϣ inline
	inline void		setServerType(uint8 uType)		{ m_uServerType = uType; }
	inline uint8	getServerType()const			{ return m_uServerType; }
	inline void		setServerID(uint16 uServerID)	{ if (!m_uServerID)m_uServerID = uServerID; }
	inline uint16	getServerID()const				{ return m_uServerID; }
	inline CMutex*	getRootLock()					{ return CTimer::GetSingleton().get_timer_locket(); }
	
	// virtual
	virtual void setServicesTitle(const char* pTitle, ...);	//���÷���������
	virtual void showToConsole(const char* pFormat, ...);	//��ʾ��Ϣ
	virtual bool initialize(uint16 uServerID);	// ��ʼ��server
	virtual bool canStart();					// �ܷ�����server
	virtual bool startServices();				// ����server	
	virtual void maintenances();				// ���߳̿�ѭ�������ﲻ�����߼�
	virtual void timerProcess();				// timer�����̣߳��߼������ﴦ��
	virtual void stopServices();				// �ر�server
	virtual void denyServices();				// �ܾ�server
	virtual bool canExitServices();				// �ܷ��˳�
	virtual bool loadData();					// load��Ϸ����

	// static
	static void	messageBoxOK(const char* pCaption, const char* pText, ...);

public:/*Ctrl+C�˳��¼�*/
	static CCondEvent	m_clExitEvent;

private:
	volatile bool m_bExitServices;	//�˳�����(�յ��˳���Ϣ)
	CCondEvent	m_clMaintainEvent;	//���ַ����¼�

protected:
	char		m_szEventExist[256];	// ȫ���¼��� windowsϵͳ��Ч
	uint64		m_uPingTime;			// pingʱ��
	bool		m_bInitFlag;			// server�Ƿ��Ѿ���ʼ�����
	uint32		m_uLoadFlag;			// ��ȡ������ɱ�ʾ
	uint64		m_tmStartTime;			// ����ʱ��
	char		m_szTitle[256];			// ����
	uint8		m_uServerType;			// ����������
	uint16		m_uServerID;			// ������id

#ifdef WIN32
	static HANDLE m_hServerEvent;	// ȫ���¼������windows����Ψһ���̱�ʾ
#endif
};