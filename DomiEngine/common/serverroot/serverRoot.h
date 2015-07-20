/******************************************************************** 
创建时间:        2015/07/09 10:35
文件名称:        serverRoot.h
文件作者:        Domi
功能说明:        server 基类  
其他说明:        
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
	bool initEvent();				// 初始化事件
	void installBreakHandlers();	// 设置信号捕捉
	virtual bool loadConfig();		// 从本地配置文件Config.ini读取数据

public:
	// 获取服务器id等信息 inline
	inline void		setServerType(uint8 uType)		{ m_uServerType = uType; }
	inline uint8	getServerType()const			{ return m_uServerType; }
	inline void		setServerID(uint16 uServerID)	{ if (!m_uServerID)m_uServerID = uServerID; }
	inline uint16	getServerID()const				{ return m_uServerID; }
	inline CMutex*	getRootLock()					{ return CTimer::GetSingleton().get_timer_locket(); }
	
	// virtual
	virtual void setServicesTitle(const char* pTitle, ...);	//设置服务器标题
	virtual void showToConsole(const char* pFormat, ...);	//显示信息
	virtual bool initialize(uint16 uServerID);	// 初始化server
	virtual bool canStart();					// 能否启动server
	virtual bool startServices();				// 启动server	
	virtual void maintenances();				// 主线程空循环，这里不处理逻辑
	virtual void timerProcess();				// timer工作线程，逻辑在这里处理
	virtual void stopServices();				// 关闭server
	virtual void denyServices();				// 拒绝server
	virtual bool canExitServices();				// 能否退出
	virtual bool loadData();					// load游戏数据

	// static
	static void	messageBoxOK(const char* pCaption, const char* pText, ...);

public:/*Ctrl+C退出事件*/
	static CCondEvent	m_clExitEvent;

private:
	volatile bool m_bExitServices;	//退出变量(收到退出消息)
	CCondEvent	m_clMaintainEvent;	//保持服务事件

protected:
	char		m_szEventExist[256];	// 全局事件名 windows系统有效
	uint64		m_uPingTime;			// ping时间
	bool		m_bInitFlag;			// server是否已经初始化完成
	uint32		m_uLoadFlag;			// 读取配置完成标示
	uint64		m_tmStartTime;			// 启动时间
	char		m_szTitle[256];			// 标题
	uint8		m_uServerType;			// 服务器类型
	uint16		m_uServerID;			// 服务器id

#ifdef WIN32
	static HANDLE m_hServerEvent;	// 全局事件句柄，windows下作唯一进程标示
#endif
};