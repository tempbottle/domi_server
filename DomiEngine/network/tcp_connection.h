/******************************************************************** 
创建时间:        2015/06/30 20:02
文件名称:        tcp_content.h
文件作者:        Domi
功能说明:        tcp content 基类 
其他说明:         
*********************************************************************/

#pragma once
#include "network/libevent.h"
#include "common/thread/csLocker.h"
#include "tcp_context.h"

class CTcpConnection
{
public:
	CTcpConnection();
	~CTcpConnection() {}

public:
	virtual void initialize();
	virtual void disconnect();
	virtual void onKickOut();
	virtual pc_str	remote_ip();
	virtual ulong	remote_address();

	// 回调
	virtual void onConnect();		// 连接上的回调
	virtual void onDisconnect();	// 断开连接

public:
	// 内联函数
	//inline void	bindPlayer(){ }
	inline void	bindContext(CTcpContext* pContext)	{ m_pContext = pContext; }	// 绑定context
	inline CMutex* getApplyMutex()					{ return &m_apply_mutex; }
	inline uint32  getApplyKey()					{ return m_apply_key;    }
	inline void	setApplyLock(bool _lock)			{ if (_lock)m_apply_mutex.lock(); else m_apply_mutex.unlock();}

private:
	uint32		m_apply_key;	//引用key,确保独立引用
	CMutex		m_apply_mutex;	//确保线程安全

	CTcpContext* m_pContext;	// 客户端上下文
};