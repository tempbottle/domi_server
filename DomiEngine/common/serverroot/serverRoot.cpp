/******************************************************************** 
创建时间:        2015/07/09 11:09
文件名称:        serverRoot.cpp
文件作者:        Domi
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
CCondEvent CServerRoot::m_clExitEvent;	//退出事件
#ifdef WIN32
HANDLE	CServerRoot::m_hServerEvent = nullptr;
#endif

// 信号中断处理
static void	onBreak(int)
{
#ifdef WIN32
	::AllocConsole();
#endif// WIN32
	CServerRoot::m_clExitEvent.set_event();	// 触发退出事件
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

	// dump信息 
	// TODO
}

CServerRoot::~CServerRoot()
{
	stopServices();
}

//-------------------------------------------------------------------
// private
// 事件初始化
bool CServerRoot::initEvent()
{
#ifdef WIN32
	// 在windows系统下，如果开启了一个server进程，
	// 通过唯一事件，可以判断是否有相同事件的进程存在
	char szEvent[256] = { 0 };	/*唯一事件*/
	dSprintf(szEvent, sizeof(szEvent), "%s_%d", m_szEventExist, m_uServerID);

	// 如果可以打开指定事件名的事件，说明已经存在该进程
	m_hServerEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, szEvent);
	if (m_hServerEvent)
		return false;

	m_hServerEvent = ::CreateEvent(nullptr, FALSE, FALSE, szEvent);
#endif

	//创建退出事件
	if (!m_clExitEvent.isValid() && !m_clExitEvent.create_event(true, false))
		return false;
	m_clExitEvent.reset_event();

	//保持服务事件
	if (!m_clMaintainEvent.isValid() && !m_clMaintainEvent.create_event(true, false))
		return false;

	m_clMaintainEvent.reset_event();

	return true;
}

// 设置信号捕捉
void CServerRoot::installBreakHandlers()
{
#ifdef WIN32
	::signal(SIGBREAK, onBreak);
#else
	::signal(SIGSTOP, onBreak);//停止进程
	::signal(SIGTSTP, onBreak);//终端上发出的停止信号
	::signal(SIGKILL, onBreak);//杀死进程信号
	::signal(SIGHUP, onBreak);//在控制终端上是挂起信号, 或者控制进程结束
#endif

	::signal(SIGTERM, onBreak);	//使用kill指令时
	::signal(SIGINT, onBreak);	//Ctrl+C的时候进程收到信号，触发OnBreak函数
}

// 从本地配置文件
bool CServerRoot::loadConfig()
{
	return true;
}

//-------------------------------------------------------------------
// public
// 设置服务器标题
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

	// 设置控制台标题
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

// 初始化
bool CServerRoot::initialize(uint16 uServerID)
{
	// 日志初始化
    char logprev[ 256 ] = { 0 };
    dSprintf( logprev, sizeof( logprev ), "%s_%u", "test_log_", uServerID );
    CLog::initialize( NULL, logprev );

    // 这里必须在CLog::initilize之后
    // 守护进程 TODO

	if(m_bInitFlag)
		return true;

	::srand((uint32)getTime());		// 设置随机种子
	m_uServerID = uServerID;		// 设置服务器id

	// load 服务器配置
	if(!loadConfig())
	{
		CLog::error("配置文件装载失败!...");
		return false;
	}
	
	installBreakHandlers();

	if(!initEvent())
	{
		CLog::error("事件初始失败!...");
		return false;
	}

	m_tmStartTime = getTime();

	m_bInitFlag = true;
	return true;
}

// 能否启动server
bool CServerRoot::canStart()
{
	return true;
}

// 开启server
bool CServerRoot::startServices()
{
	showToConsole("<Prepare Game Service...>");

	//把自己加入定时器
	// 注意：需要先addtimer，再startTimer，因为没有pending事件或active事件，event_base_dispatch会退出循环，导致timer线程提前结束
	CTimer::GetSingleton().addTimer(std::bind(&CServerRoot::timerProcess, this), 10);

	//启动定时器，创建了一个timer线程，这个线程就是处理消息包的线程
	if(!CTimer::GetSingleton().startTimer())
		return false;

	while(!m_clExitEvent.wait_event(1000))
	{
		if (canStart())
			break;
	}

	return true;
}

// 保持server
void CServerRoot::maintenances()
{
	//保持服务  没有信号表示服务继续运行
	while(!m_clMaintainEvent.wait_event(1000))
	{
		// 触发退出事件，server开始拒绝服务，并断开所有连接
		if (m_clExitEvent.wait_event(0))
		{
			if (m_bExitServices)
				CLog::warn("正在退出，请勿重复操作……");

			CCritLocker clLock(this->getRootLock());
			m_bExitServices = true;
			m_clExitEvent.reset_event();
			denyServices();
		}

		//能否退出，每1s循环检查，是否已经退出完成
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

		/*处理控制台数据*/
		/*处理控制台输入*/
		// TODO
		//CLog::print(__FUNCTION__);
	}
}

// 停止服务
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

// 拒绝服务
void CServerRoot::denyServices()
{
	showToConsole("<Deny user login service ...>");

	showToConsole("<Waiting for Queue process ...>");
}

// 能否退出
bool CServerRoot::canExitServices()
{
	// 逻辑层判断玩家是否全部下线
	// 逻辑层判断玩家数据是否都已经入库了

	return true;
}

// timer工作线程
void CServerRoot::timerProcess()
{

}

// load游戏数据
bool CServerRoot::loadData()
{
	return true;
}

//-------------------------------------------------------------------
// static
// windows 的messagebox
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
