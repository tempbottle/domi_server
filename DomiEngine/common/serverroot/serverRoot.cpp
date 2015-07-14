/******************************************************************** 
����ʱ��:        2015/07/09 11:09
�ļ�����:        serverRoot.cpp
�ļ�����:        Domi
*********************************************************************/

#include "common/serverroot/serverRoot.h"
#include "common/basic/basicFunctions.h"
#include "common/basic/stringFunctions.h"
#include "common/basic/timeFunction.h"
#include "common/log/log.h"
#include "common/log/consoleColor.h"
#include "network/message_queue.h"
#include "common/basic/platform.h"

//////////////////////////////////////////////////////////////////////////
// static
CCondEvent CServerRoot::m_clExitEvent;	//�˳��¼�
#ifdef WIN32
HANDLE	CServerRoot::m_hServerEvent = nullptr;
#endif

// �ź��жϴ���
static void	onBreak(int)
{
#ifdef WIN32
	::AllocConsole();
#endif// WIN32
	CServerRoot::m_clExitEvent.set_event();	// �����˳��¼�
}

//////////////////////////////////////////////////////////////////////////
// server root
CServerRoot::CServerRoot()
{
	m_bExitServices		= false;
	m_bInitFlag			= false;
	m_uLoadFlag			= 0;
	m_tmStartTime		= 0;
	m_uPingTime			= 0;
	m_uServerType		= 1;
	m_uServerID			= 0;

	memset(m_szTitle,0,sizeof(m_szTitle));
	memset(m_szEventExist,0,sizeof(m_szEventExist));

	// dump��Ϣ 
	// TODO
}

CServerRoot::~CServerRoot()
{
	stopServices();
}

//-------------------------------------------------------------------
// private
// �¼���ʼ��
bool CServerRoot::initEvent()
{
#ifdef WIN32
	// ��windowsϵͳ�£����������һ��server���̣�
	// ͨ��Ψһ�¼��������ж��Ƿ�����ͬ�¼��Ľ��̴���
	char szEvent[256] = { 0 };	/*Ψһ�¼�*/
	dSprintf(szEvent, sizeof(szEvent), "%s_%d", m_szEventExist, m_uServerID);

	// ������Դ�ָ���¼������¼���˵���Ѿ����ڸý���
	m_hServerEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, szEvent);
	if (m_hServerEvent)
		return false;

	m_hServerEvent = ::CreateEvent(nullptr, FALSE, FALSE, szEvent);
#endif

	//�����˳��¼�
	if (!m_clExitEvent.isValid() && !m_clExitEvent.create_event(true, false))
		return false;
	m_clExitEvent.reset_event();

	//���ַ����¼�
	if (!m_clMaintainEvent.isValid() && !m_clMaintainEvent.create_event(true, false))
		return false;

	m_clMaintainEvent.reset_event();

	return true;
}

// �����źŲ�׽
void CServerRoot::installBreakHandlers()
{
#ifdef WIN32
	::signal(SIGBREAK, onBreak);
#else
	::signal(SIGSTOP, onBreak);//ֹͣ����
	::signal(SIGTSTP, onBreak);//�ն��Ϸ�����ֹͣ�ź�
	::signal(SIGKILL, onBreak);//ɱ�������ź�
	::signal(SIGHUP, onBreak);//�ڿ����ն����ǹ����ź�, ���߿��ƽ��̽���
#endif

	::signal(SIGTERM, onBreak);	//ʹ��killָ��ʱ
	::signal(SIGINT, onBreak);	//Ctrl+C��ʱ������յ��źţ�����OnBreak����
}

// �ӱ��������ļ�
bool CServerRoot::loadConfig()
{
	return true;
}

//-------------------------------------------------------------------
// public
// ���÷���������
void CServerRoot::setServicesTitle(const char* pTitle, ...)
{
	if (!pTitle && m_szTitle[0])
		return;

	char szTitle[256] = { 0 };
	if (pTitle)
	{
		va_list	argptr;
		va_start(argptr, pTitle);
		dVsprintf(szTitle, sizeof(szTitle), pTitle, argptr);
		va_end(argptr);
	}

	if (!m_tmStartTime)
		m_tmStartTime = getTime();

	tm	tmTime;
	getLocalTime(&tmTime, &m_tmStartTime);

	dSprintf(m_szTitle, sizeof(m_szTitle), "(V%d.%d.%d%.2d)%s StartTime: %02d-%02d %02d:%02d:%02d",
		1, 0, 1, 0, szTitle,
		tmTime.tm_mon, tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);

	// ���ÿ���̨����
#ifdef WIN32
	::SetConsoleTitle(m_szTitle);
#endif
}

void CServerRoot::showToConsole(const char* pFormat, ...)
{
	if (!pFormat || !*pFormat)
		return;

	char szStrint[2048] = { 0 };
	va_list	argptr;
	va_start(argptr, pFormat);
	dVsprintf(szStrint, sizeof(szStrint), pFormat, argptr);
	va_end(argptr);

	CConsoleColor clColor(_BIT32(CConsoleColor::color_green) | _BIT32(CConsoleColor::color_intensity));

	dPrintf("%s\n", szStrint);
}

// ��ʼ��
bool CServerRoot::initialize(uint16 uServerID)
{
	// ��־��ʼ��
    char logprev[ 256 ] = { 0 };
    dSprintf( logprev, sizeof( logprev ), "%s_%u", "test_log_", uServerID );
    CLog::initialize( NULL, logprev );

    // ���������CLog::initilize֮��
    // �ػ����� TODO

	if(m_bInitFlag)
		return true;

	::srand((uint32)getTime());		// �����������
	m_uServerID = uServerID;		// ���÷�����id

	// load ����������
	if(!loadConfig())
	{
		CLog::error("�����ļ�װ��ʧ��!...");
		return false;
	}
	
	installBreakHandlers();

	if(!initEvent())
	{
		CLog::error("�¼���ʼʧ��!...");
		return false;
	}

	m_tmStartTime = getTime();

	m_bInitFlag = true;
	return true;
}

// �ܷ�����server
bool CServerRoot::canStart()
{
	return true;
}

// ����server
bool CServerRoot::startServices()
{
	showToConsole("<Prepare Game Service...>");

	//���Լ����붨ʱ��
	// ע�⣺��Ҫ��addtimer����startTimer����Ϊû��pending�¼���active�¼���event_base_dispatch���˳�ѭ��������timer�߳���ǰ����
	CTimer::GetSingleton().addTimer(std::bind(&CServerRoot::timerProcess, this), 10);

	//������ʱ����������һ��timer�̣߳�����߳̾��Ǵ�����Ϣ�����߳�
	if(!CTimer::GetSingleton().startTimer())
		return false;

	while(!m_clExitEvent.wait_event(1000))
	{
		if (canStart())
			break;
	}

	return true;
}

// ����server
void CServerRoot::maintenances()
{
	//���ַ���  û���źű�ʾ�����������
	while(!m_clMaintainEvent.wait_event(1000))
	{
		// �����˳��¼���server��ʼ�ܾ����񣬲��Ͽ���������
		if (m_clExitEvent.wait_event(0))
		{
			if (m_bExitServices)
				CLog::warn("�����˳��������ظ���������");

			CCritLocker clLock(this->getRootLock());
			m_bExitServices = true;
			m_clExitEvent.reset_event();
			denyServices();
		}

		//�ܷ��˳���ÿ1sѭ����飬�Ƿ��Ѿ��˳����
		if (m_bExitServices)
		{
			bool bCanExit = false;
			{
				CCritLocker clLock(getRootLock());
				if (canExitServices())
					bCanExit = true;
			}

			if (bCanExit)
			{
				m_clMaintainEvent.set_event();
				CTimer::GetSingleton().stopTimer();
				return;
			}
		}

		/*�������̨����*/
		/*�������̨����*/
		// TODO
		//CLog::print(__FUNCTION__);
	}
}

// ֹͣ����
void CServerRoot::stopServices()
{
	m_clExitEvent.reset_event();
	m_clExitEvent.close_event();

	m_clMaintainEvent.reset_event();
	m_clMaintainEvent.close_event();

#ifdef WIN32
	::CloseHandle(m_hServerEvent);
	m_hServerEvent = nullptr;
#endif
}

// �ܾ�����
void CServerRoot::denyServices()
{
	showToConsole("<Deny user login service ...>");

	showToConsole("<Waiting for Queue process ...>");
}

// �ܷ��˳�
bool CServerRoot::canExitServices()
{
	// �߼����ж�����Ƿ�ȫ������
	// �߼����ж���������Ƿ��Ѿ������

	return true;
}

// timer�����߳�
void CServerRoot::timerProcess()
{

}

// load��Ϸ����
bool CServerRoot::loadData()
{
	return true;
}

//-------------------------------------------------------------------
// static
// windows ��messagebox
void CServerRoot::messageBoxOK(const char* pCaption,const char* pText,...)
{
	char szStrint[2048] = {0};
	if (pText)
	{
		va_list	argptr;
		va_start(argptr,pText);
		dVsprintf(szStrint,sizeof(szStrint),pText,argptr);
		va_end(argptr);
	}
	CConsoleColor clColor(_BIT32(CConsoleColor::color_blue)|_BIT32(CConsoleColor::color_intensity));
	dPrintf("message>%s:%s\n",pCaption ? pCaption : "",szStrint);

#ifdef WIN32
	::ShowCursor(true);
	::MessageBox(nullptr,szStrint,pCaption, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OK);
#endif
}
