/******************************************************************** 
创建时间:        2015/06/28 22:35
文件名称:        tcp_server.h
文件作者:        Domi
功能说明:        tcpServer  
其他说明:         
*********************************************************************/

#pragma once
#include <vector>
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
	inline void SetWorkThreadNum(uint32 num)	{ m_unWorkThreadNum = num; }
	inline uint32& GetWorkThreadNum()			{ return m_unWorkThreadNum; }
	inline uint32& GetCurWorker()				{ return m_nCurrentWorker; }
	inline void SetListenPort(uint16 port)		{ m_port = port; }
	inline CMutex& GetContextMapLock()			{ return m_mapContext.getLock(); }

public:
	bool Initialize(uint32 threadNum, uint16 port);			// tcpserver初始化
	bool StarLister();										// 启动监听线程
	bool StartServer();										// 启动整个服务
	bool StopServer();										// 关闭服务
	void CloseContext(CTcpContext *pContext);				// 关闭某个连接

	// [virtual]回调
	virtual void OnConnect(CTcpContext* pContext);			// 接收新连接回调
	virtual void OnDisConnect(CTcpContext* pContext);		// 断开连接回调
	virtual void OnTimeoutWrite(CTcpContext* pContext);		// 写超时回调
	virtual void OnTimeoutRead(CTcpContext* pContext);		// 读超时回调
	virtual bool OnProcessPacket(CTcpContext* pContext);	// 解包

	// 虚函数子类实现
	virtual CTcpConnection*	ConnectNew(CTcpContext* pContext) = 0;
	virtual CMutex*			ConnectLock() = 0;
	virtual CTcpConnection*	ConnectFind(CTcpContext* pContext) = 0;
	virtual void			ConnectDel(CTcpContext* pContext) = 0;

	// [static]接受新连接回调函数
	static void DoAccept(evconnlistener *listener, evutil_socket_t fd,sockaddr *sa, int socklen, void *user_data);
	static void DoRead(struct bufferevent *bev, void *ctx);
	static void DoEvent(struct bufferevent *bev, short error, void *ctx);
	static void DoAcceptError(evconnlistener *listener,void* ctx);	// listener的监听错误回调
	static THREAD_RETURN _listener_thread_(void* pParam);
	static THREAD_RETURN _working_thread_(void* pParam);

private:
	// libevnt
	event_base*			m_pListenerBase;	//监听反应堆
	event				m_evAccept;			//接受事件
	evutil_socket_t		m_clListenerSocket;	//监听socket
	evconnlistener*		m_pListener;		//监听对象

	// thread
	uint32				m_unWorkThreadNum;	//工作线程数量
	CThread				m_clListenerThread;	//监听线程
	VECTOR_THREAD		m_vecWorkThread;	//工作线程池
	uint32				m_nCurrentWorker;	//线程负载均衡

	// context
	uint16				m_port;				//监听端口
	MAP_CONTEXT			m_mapContext;		//客户端上下文集合
	uint16				m_send_buffer_max;	//最大发送缓冲区数量
	timeval				m_timeout;
	
public:
	uint32				m_nextContextId;
	CMessageQueue		m_clMessageQueue;	// 消息队列
};