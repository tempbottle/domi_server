/******************************************************************** 
创建时间:        2015/06/28 22:35
文件名称:        tcp_server.h
文件作者:        Domi
功能说明:        tcpServer  
其他说明:         
*********************************************************************/

#pragma once
#include <vector>
#include "common/thread/csLocker.h"
#include "network/libevent.h"
#include "network/tcp_thread.h"
#include "tcp_context.h"
#include "tcp_connection.h"
#include "message_queue.h"
#include "common/pool/map_pool.h"

class CTcpServer
{
protected:
	typedef std::vector<CTcpThread*> VECTOR_THREAD;
	typedef CMapPool<uint32, CTcpContext> MAP_CONTEXT;

public:
	CTcpServer();
	virtual ~CTcpServer();

private:
	static int32 _qsort_thread	(const void *p1, const void *p2);	//对线程使用次数进行排序

public:
	inline void SetWorkThreadNum(uint32 num) { m_unWorkThreadNum = num; }

	void SetTimeout(uint64 _timeout_connect,uint64 _timeout_read,uint64 _timeout_write);
	bool StartServer();
	bool StopServer();
	void CloseContext(CTcpContext *pContext);

	// 回调
	virtual void OnConnect(CTcpContext* pContext);				// 接收新连接回调
	virtual void OnDisConnect(CTcpContext* pContext);			// 断开连接回调
	virtual void onTimeoutWrite(CTcpContext* pContext);			// 写超时回调
	virtual void onTimeoutRead(CTcpContext* pContext);			// 读超时回调
	virtual bool OnProcessPacket(CTcpContext* pContext);		// 解包

	// 虚函数子类实现
	virtual CTcpConnection*	ConnectNew(CTcpContext* pContext) = 0;
	virtual CMutex*			ConnectLock() = 0;
	virtual CTcpConnection*	ConnectFind(CTcpContext* pContext) = 0;
	virtual void			ConnectDel(CTcpContext* pContext) = 0;
public:
	// 接受新连接回调函数
	static void DoAccept(evconnlistener *listener, evutil_socket_t fd,sockaddr *sa, int socklen, void *user_data);
	static void DoRead(struct bufferevent *bev, void *ctx);
	static void DoEvent(struct bufferevent *bev, short error, void *ctx);
	static void DoAcceptError(evconnlistener *listener,void* ctx);	// listener的监听错误回调
	static THREAD_RETURN	_listener_thread_(void* pParam);
	static THREAD_RETURN	_working_thread_(void* pParam);

public:
	// libevnt
	event_base*			m_pListenerBase;	//监听反应堆
	event				m_evAccept;			//接受事件
	evutil_socket_t 	m_clListenerSocket;	//监听socket
	evconnlistener*		m_pListener;		//监听对象

	// thread
	CThread				m_clListenerThread;	//监听线程
	VECTOR_THREAD		m_vecWorkThread;	//工作线程池
	uint32				m_unWorkThreadNum;	//工作线程数量
	uint32				nCurrentWorker;		//线程负载均衡
	CCondEvent			m_clExitEvent;		//退出循环事件,线程同步条件变量

	// 
	MAP_CONTEXT			m_mapContext;		//客户端上下文集合
	uint16				m_send_buffer_max;	//最大发送缓冲区数量
	timeval				m_timeout;
	uint64				m_timeout_connect;	//链接超时(秒)
	uint64				m_timeout_read;		//读取超时时间(秒)
	uint64				m_timeout_write;	//写入超时时间(秒)

	std::map<CTcpContext*, CTcpConnection*> m_mapContent;	// 客户端连接
	CMessageQueue		m_clMessageQueue;	// 消息队列
	CMutex				m_Mutex;			// 互斥锁,用来锁context集合

	uint32 m_nextContextId;
};