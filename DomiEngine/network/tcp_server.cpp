/******************************************************************** 
创建时间:        2015/06/22 20:27
文件名称:        tcp_server.cpp
文件作者:        Domi
*********************************************************************/

#include "tcp_server.h"
#include <functional>
#include "common/log/log.h"

CTcpServer::CTcpServer()
	: m_unWorkThreadNum(3)
	, m_nextContextId(0)
{
#ifdef WIN32
	WSADATA WSAData;
	WSAStartup(0x0201, &WSAData);
#endif

	m_pListenerBase = nullptr;
	nCurrentWorker = 0;
}

CTcpServer::~CTcpServer()
{
#ifdef WIN32
	WSACleanup();
#endif
}

void CTcpServer::SetTimeout(uint64 _timeout_connect,uint64 _timeout_read,uint64 _timeout_write)
{
	m_timeout_connect = _timeout_connect;
	m_timeout_read = _timeout_read;
	m_timeout_write = _timeout_write;
}

bool CTcpServer::StartServer()
{
	CLog::info("tcp server 开启……");
	m_pListenerBase = new_event_base();
	if (m_pListenerBase==NULL)	// event_base 创建失败
	{
		return false;
	}

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(17777);
	m_pListener = evconnlistener_new_bind(m_pListenerBase, DoAccept, (void*)this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&sin, sizeof(sin));
	if (m_pListener==NULL)
	{
		return false;
	}
	evconnlistener_set_error_cb(m_pListener, DoAcceptError);

	// 监听线程
	m_clListenerThread.create(&CTcpServer::_listener_thread_,this);

	CTcpThread* pWorkThread = nullptr;
	// 初始化work线程
	for (uint32 i=0;i<m_unWorkThreadNum;++i)
	{
		pWorkThread = new CTcpThread();	// new 加括号，表示显式调用构造函数，不带括号表示隐式调用构造函数
		if (pWorkThread)
		{
			pWorkThread->m_clTcpServer = this;
			pWorkThread->m_base = new_event_base();
			pWorkThread->m_exit = &m_clExitEvent;

			m_vecWorkThread.push_back(pWorkThread);
			pWorkThread->create(&CTcpServer::_working_thread_,pWorkThread);
		}
	}

	return true;
}

bool CTcpServer::StopServer()
{
	printf("开始关闭server！\n");
	// 延时2s关闭
	struct timeval delay = { 2, 0 };
	event_base_loopexit(this->m_pListenerBase, &delay);

	// 工作base 延时2s
	for (uint32 i = 0; i < m_vecWorkThread.size(); ++i)
	{
		CTcpThread* pWork = m_vecWorkThread[i];
		if (pWork)
			event_base_loopexit(pWork->m_base, &delay);
	}
	
	// 关闭listener线程
	m_clListenerThread.wait_exit();	// 等待线程退出
	evconnlistener_free(m_pListener);
	event_base_free(m_pListenerBase);
	CLog::warn("[accept]线程关闭完成……");

	// 关闭work线程
	for (uint32 i=0;i<m_vecWorkThread.size();++i)
	{
		CTcpThread* pWork = m_vecWorkThread[i];
		if (!pWork)
			continue;

		pWork->wait_exit();
		event_base_free(pWork->m_base);
		delete pWork;
	}

	CLog::warn("[work]线程关闭完成……");

	return true;
}

void CTcpServer::CloseContext(CTcpContext *pContext)
{
	// 锁住context，防止tcpserver的DoRead线程进行buffer读取
	CCritLocker lock(pContext->m_csLock);
	printf("info: 线程id = %d！\n", CTcpThread::getCurrentThreadID());
	bufferevent_lock(pContext->m_bufev);
	bufferevent_disable(pContext->m_bufev, EV_READ | EV_WRITE);
	evutil_closesocket(pContext->m_fd);
	bufferevent_free(pContext->m_bufev);
	bufferevent_unlock(pContext->m_bufev);
	pContext->initialize();

	CCritLocker contextLocker(m_mapContext.getLock());
	m_mapContext.release(pContext->m_ContextId);
	this->OnDisConnect(pContext);
	printf("info: CTcpServer::CloseContext！\n");
}

// 接收新连接的回调
void CTcpServer::OnConnect(CTcpContext* pContext)
{
	CTcpConnection* pConn = ConnectNew(pContext);
	if (pConn == nullptr)
	{
		pContext->disconnect();
		return;
	}
	
	pConn->onConnect();
}

// 断开连接回调
void CTcpServer::OnDisConnect(CTcpContext* pContext)
{
	// 锁住连接map，找到对应的client
	CCritLocker connLock(ConnectLock());
	CTcpConnection* pConn = ConnectFind(pContext);
	if (pConn)
	{
		// 确保client连接不在timer线程中处理逻辑
		CCritLocker clApplyLock(pConn->getApplyMutex());
		pConn->onDisconnect();
		pConn->initialize();
	}
	ConnectDel(pContext);	// 回收客户端连接
}

// 写超时回调
void CTcpServer::onTimeoutWrite(CTcpContext* pContext)
{

}

// 读超时回调
void CTcpServer::onTimeoutRead(CTcpContext* pContext)
{
}

// 解包
bool CTcpServer::OnProcessPacket(CTcpContext* pContext)
{
	// 检查连接是否已经完好
	// TODO

	while (pContext->m_inbuf && pContext->getPendingLen() > sizeof(PacketHead))
	{
		//一个协议包的请求头还没读完，则继续循环读或者等待下一个libevent时间进行循环读
		PacketHead* pHead = (PacketHead*)(pContext->m_inbuf + pContext->m_readBegin);

		// 消息包头不合法
		if (pHead->uPacketSize > MaxBuffLen || pHead->uPacketSize < sizeof(PacketHead))
		{
			//请求包不合法
			pContext->disconnect();
			return false;
		}

		// 剩余数据不够一个包，继续收
		if (pHead->uPacketSize > pContext->getPendingLen())
		{
			printf("剩余数据不够一个包，继续收！\n");
			return false;
		}

		if (pContext->m_readBegin > pContext->m_inbufLen)
		{
			// 出问题了，已读的字节数居然大于总数，直接断掉连接
			printf("出问题了，已读的字节数居然大于总数，直接断掉连接！\n");
			this->CloseContext(pContext);
			return false;
		}

		// 压入message
		CCritLocker connMapLock(ConnectLock());
		CTcpConnection* pConn = ConnectFind(pContext);
		if (pConn)
			this->m_clMessageQueue.push(pConn->getApplyKey(), pContext, pContext->m_inbuf + pContext->m_readBegin, pHead->uPacketSize);
		
		//处理下一个协议包
		pContext->m_readBegin += pHead->uPacketSize;
	}

	// 不足一个包的数据，移动到开始位置
	int pending = pContext->getPendingLen();
	if (pending > 0)	// 有需要移动的字节
		memmove(pContext->m_inbuf, pContext->m_inbuf + pContext->m_readBegin, pending);

	pContext->m_inbufLen = pending;
	pContext->m_readBegin = 0;

	return true;
}

//-------------------------------------------------------------------
// static method
// 接受新连接回调函数
void CTcpServer::DoAccept(evconnlistener *listener, evutil_socket_t fd, sockaddr *sa, int socklen, void *user_data)
{
	CLog::info("tcp accept 线程id = [%d] ", CTcpThread::getCurrentThreadID());
	if (fd <=0 )
	{
		printf("无效的socket！fd = [ %d] \n", fd);
		return;
	}
	
	CTcpServer* pServer = (CTcpServer *)user_data;
	int nCurrent = (pServer->nCurrentWorker++) % pServer->m_unWorkThreadNum;	//工作线程负载均衡
	CTcpThread* pWorkThread = pServer->m_vecWorkThread[nCurrent];
	
	if (!pWorkThread)
	{
		printf("创建工作线程错误！\n");
		return;
	}

	CCritLocker contextLocker(pServer->m_mapContext.getLock());
	CTcpContext* pContext = pServer->m_mapContext.allocate(pServer->m_nextContextId);
	if (!pContext)
	{
		printf("context 分配错误！\n");
		return;
	}

	// 创建一个bufferevent，绑定socket，并托管给event_base
	// 注意，这里是托管给分配的工作线程的base，而不是监听线程的base
	pContext->m_bufev=bufferevent_socket_new(pWorkThread->m_base,fd, BEV_OPT_CLOSE_ON_FREE|LEV_OPT_THREADSAFE);
	if (pContext->m_bufev == NULL)
	{
		printf("创建eventbuffer失败！\n");
		return;
	}

	//int ret = bufferevent_setfd(pContext->bufev, fd); //注意：这里如果是iocp会设置不成功，必须在 bufferevent_socket_new 就设置fd
	//evutil_socket_t testfd = bufferevent_getfd(pContext->m_bufev);

	// 设置bufev 三个回调函数
	bufferevent_setcb(pContext->m_bufev, &CTcpServer::DoRead, NULL, DoEvent, pContext);

	// 设置 bufferevent的水位，需要在看看文档 和源码
	//bufferevent_setwatermark(pContext->bufev, EV_READ, 0, 10240);

	// 写入超时
	struct timeval delayWriteTimeout;
	delayWriteTimeout.tv_sec = 2;//pServer->m_timeout_write;
	delayWriteTimeout.tv_usec = 0;

	// 读取超时
	struct timeval delayReadTimeout;
	delayReadTimeout.tv_sec = 2; //pServer->m_timeout_read;
	delayReadTimeout.tv_usec = 0;

	// 设置超时时间
	//bufferevent_set_timeouts(pContext->m_bufev, /*&delayReadTimeout*/NULL, &delayWriteTimeout);
	evutil_make_socket_nonblocking(fd);
	bufferevent_enable(pContext->m_bufev, EV_READ | EV_WRITE);	// 调用event_add将读写事件加入到事件监听队列中
	pContext->initContext(pServer, pWorkThread, fd, pServer->m_nextContextId);
	++pServer->m_nextContextId;

	pServer->OnConnect(pContext);
}

void CTcpServer::DoRead(struct bufferevent* bev, void *ctx)
{
	//CLog::info("tcp work 线程id = [%d] ", CTcpThread::getCurrentThreadID());
	struct evbuffer* input=bufferevent_get_input(bev);
	if (evbuffer_get_length(input)) 
	{
		CTcpContext* pContext = reinterpret_cast<CTcpContext*>(ctx);
		CCritLocker lock(pContext->m_csLock);
		int buffLen = evbuffer_get_length(input);	// evbuff 内的总字节数
		while (buffLen>0)
		{
			//printf("buffLen = %d\n", buffLen);
			int freeLen = pContext->getFreeLen();
			if (freeLen<=0)
			{
				// 没有多余的空间,解包先
				if (!pContext->m_clTcpServer->OnProcessPacket(pContext))
				{
					printf("OnProcessPacket 错误！！\n");
					return;
				}
			}
			else
			{
				//bufferevent_read(bev, pContext->m_inbuf + pContext->m_readBegin, freeLen);
				int ret = evbuffer_remove(input, pContext->m_inbuf + pContext->m_readBegin, freeLen);
				if (ret == -1)
				{
					// 读取错误
					printf("evbuffer_remove 错误！！\n");
					return;
				}

				buffLen -= ret;
				pContext->m_inbufLen += ret;
			}
		}
		
		// 已经取出了所有
		if (pContext->m_inbufLen>0)
			pContext->m_clTcpServer->OnProcessPacket(pContext);
	}
}

void CTcpServer::DoEvent(struct bufferevent *bev, short error, void *ctx)
{
	CLog::info("tcp event 线程id = [%d] ", CTcpThread::getCurrentThreadID());
	if (error&EVBUFFER_TIMEOUT)
	{
		printf("EVBUFFER_TIMEOUT! \n");
	}
	else if (error&EVBUFFER_EOF)
	{
		printf("EVBUFFER_EOF! \n");
	}
	else if (error&EVBUFFER_ERROR)
	{
		printf("EVBUFFER_ERROR! \n");
	}

	CTcpContext* pContext = reinterpret_cast<CTcpContext*>(ctx);
	if (pContext)
	{
		pContext->disconnect();
	}
}

void CTcpServer::DoAcceptError(evconnlistener *listener, void* ctx)
{
	printf("监听错误回调! \n");
}

// 监听线程
THREAD_RETURN CTcpServer::_listener_thread_(void* pParam)
{
	CTcpServer* pTcpServer=reinterpret_cast<CTcpServer*>(pParam);
	if (pTcpServer==NULL)
	{
		return -1;
	}

	CLog::info("[accept]线程开始启动，线程id = [%d]……", CTcpThread::getCurrentThreadID());
	event_base_dispatch(pTcpServer->m_pListenerBase);
	CLog::info("[accept]线程关闭……");
	return ::GetCurrentThreadId();
}

// 工作线程
THREAD_RETURN CTcpServer::_working_thread_(void* pParam)
{
	CTcpThread* pWorkThread=reinterpret_cast<CTcpThread*>(pParam);
	if (pWorkThread==NULL)
	{
		return -1;
	}

	if (pWorkThread->m_base == NULL)
	{
		return -1;
	}

	CLog::info("[work]线程开始启动，线程id = [%d] ……", CTcpThread::getCurrentThreadID());
	event_base_loop(pWorkThread->m_base,EVLOOP_NO_EXIT_ON_EMPTY);
	CLog::info("[work]线程关闭……");
	return ::GetCurrentThreadId();
}