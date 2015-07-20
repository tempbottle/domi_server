/******************************************************************** 
创建时间:        2015/06/22 20:27
文件名称:        tcp_server.cpp
文件作者:        Domi
*********************************************************************/

#include "tcp_server.h"
#include <functional>
#include "common/log/log.h"

CTcpServer::CTcpServer()
	: m_unWorkThreadNum(2)
	, m_nextContextId(0)
	, m_port(0)
{
#ifdef WIN32
	WSADATA WSAData;
	WSAStartup(0x0201, &WSAData);
#endif

	m_pListenerBase = nullptr;
	m_nCurrentWorker = 0;
}

CTcpServer::~CTcpServer()
{
#ifdef WIN32
	WSACleanup();
#endif

	m_nextContextId = 0;
}

//////////////////////////////////////////////////////////////////////////
bool CTcpServer::Initialize(uint32 threadNum, uint16 port)
{
	if (!threadNum){
		CLog::error("[TcpServer],work线程数错误,threadNum=%d……", threadNum);
		return false;
	}

	if (!port || port<10000){
		CLog::error("[TcpServer],监听端口错误,port=%d……", port);
		return false;
	}

	m_pListenerBase = new_event_base();
	if (m_pListenerBase == NULL){// event_base 创建失败
		CLog::error("[TcpServer],listen eventbase 创建失败……");
		return false;
	}

	this->SetWorkThreadNum(threadNum);
	this->SetListenPort(port);
	return true;
}

bool CTcpServer::StarLister()
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(m_port);
	m_pListener = evconnlistener_new_bind(m_pListenerBase, DoAccept, (void*)this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&sin, sizeof(sin));
	if (m_pListener == NULL){
		CLog::error("[TcpServer],tcp监听创建失败……");
		return false;
	}

	// 设置lister错误回调
	evconnlistener_set_error_cb(m_pListener, DoAcceptError);

	// 监听线程
	if (!m_clListenerThread.create(&CTcpServer::_listener_thread_, this)){
		CLog::error("[TcpServer],tcp监听线程创建失败……");
		return false;
	}

	return true;
}

bool CTcpServer::StartServer()
{
	if (!StarLister())
		return false;

	CTcpThread* pWorkThread = nullptr;
	
	for (uint32 i = 0; i<m_unWorkThreadNum; ++i){	// 初始化work线程
		pWorkThread = new CTcpThread();	// new 加括号，表示显式调用构造函数，不带括号表示隐式调用构造函数
		if (pWorkThread && pWorkThread->initialize(this)){
			m_vecWorkThread.push_back(pWorkThread);
			pWorkThread->create(&CTcpServer::_working_thread_,pWorkThread);
		}
		else{
			CLog::error("[TcpServer],第[%d]个工作线程创建失败……",i+1);
		}
	}

	return true;
}

bool CTcpServer::StopServer()
{
	// 延时2s关闭监听线程
	timeval delay = { 2, 0 };
	event_base_loopexit(this->m_pListenerBase, &delay);

	// 延时2s关闭工作线程
	for (uint32 i = 0; i < m_vecWorkThread.size(); ++i){
		CTcpThread* pWork = m_vecWorkThread[i];
		if (pWork)
			event_base_loopexit(pWork->m_base, &delay);
	}
	
	// 关闭listener线程
	m_clListenerThread.wait_exit();	// 等待线程退出
	evconnlistener_free(m_pListener);
	event_base_free(m_pListenerBase);

	// 关闭work线程
	for (uint32 i=0;i<m_vecWorkThread.size();++i){
		CTcpThread* pWork = m_vecWorkThread[i];
		if (pWork){
			pWork->wait_exit();
			delete pWork;
		}
	}
	m_vecWorkThread.clear();

	return true;
}

void CTcpServer::CloseContext(CTcpContext *pContext)
{
	bufferevent_lock(pContext->m_bufev);
	bufferevent_disable(pContext->m_bufev, EV_READ | EV_WRITE);
	evutil_closesocket(pContext->m_fd);	// 断开bufferevent后，需要关闭套接字，否则会有问题
	bufferevent_free(pContext->m_bufev);
	bufferevent_unlock(pContext->m_bufev);

	pContext->initialize();	// 复位

	// 处理client连接断线逻辑
	CCritLocker contextLocker(GetContextMapLock());
	m_mapContext.release(pContext->m_ContextId);
	this->OnDisConnect(pContext);
}

//////////////////////////////////////////////////////////////////////////
void CTcpServer::OnConnect(CTcpContext* pContext)
{
	CTcpConnection* pConn = ConnectNew(pContext);
	if (pConn == nullptr){
		pContext->disconnect();
		return;
	}
	
	pConn->onConnect();
}

void CTcpServer::OnDisConnect(CTcpContext* pContext)
{
	// 锁住连接map，找到对应的client
	CCritLocker connLock(ConnectLock());
	CTcpConnection* pConn = ConnectFind(pContext);
	if (pConn){
		// 确保client连接不在timer线程中处理逻辑
		CCritLocker clApplyLock(pConn->getApplyMutex());
		pConn->onDisconnect();
		//pConn->initialize();
	}
	ConnectDel(pContext);	// 回收客户端连接
}

void CTcpServer::OnTimeoutWrite(CTcpContext* pContext)
{

}

void CTcpServer::OnTimeoutRead(CTcpContext* pContext)
{
}

bool CTcpServer::OnProcessPacket(CTcpContext* pContext)
{
	while (pContext->m_inbuf && pContext->getPendingLen() > sizeof(PacketHead))
	{
		//一个协议包的请求头还没读完，则继续循环读或者等待下一个libevent时间进行循环读
		PacketHead* pHead = (PacketHead*)(pContext->m_inbuf + pContext->m_readBegin);
		if (pHead->uPacketSize > MaxBuffLen || pHead->uPacketSize < sizeof(PacketHead)){ // 消息包头不合法
			pContext->disconnect();
			return false;
		}

		int len = pContext->getPendingLen();
		if (pHead->uPacketSize > pContext->getPendingLen()){ // 剩余数据不够一个包，继续收
			printf("剩余数据不够一个包，继续收！\n");
			break;
		}

		if (pContext->m_readBegin > pContext->m_inbufLen){ // 出问题了，已读的字节数居然大于总数，直接断掉连接
			this->CloseContext(pContext);
			return false;
		}

		// 压入message
		CCritLocker connMapLock(ConnectLock());
		CTcpConnection* pConn = ConnectFind(pContext);
		if (pConn)
			this->m_clMessageQueue.push(pConn->getApplyKey(), pContext, pContext->m_inbuf + pContext->m_readBegin + sizeof(PacketHead), pHead->uPacketSize);
		
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

//////////////////////////////////////////////////////////////////////////
// static method
void CTcpServer::DoAccept(evconnlistener *listener, evutil_socket_t fd, sockaddr *sa, int socklen, void *user_data)
{
	//CLog::info("tcp accept 线程id = [%d] ", CTcpThread::getCurrentThreadID());
	if (fd <=0 ){
		CLog::error("[TcpServer],无效的socket fd = [%d]……", fd);
		return;
	}
	
	CTcpServer* pServer = (CTcpServer *)user_data;
	int nCurrent = (pServer->GetCurWorker()++) % pServer->GetWorkThreadNum();	//工作线程负载均衡
	CTcpThread* pWorkThread = pServer->m_vecWorkThread[nCurrent];
	
	if (!pWorkThread){
		CLog::error("[TcpServer],创建工作线程错误……");
		return;
	}

	CCritLocker contextLocker(pServer->GetContextMapLock());
	CTcpContext* pContext = pServer->m_mapContext.allocate(pServer->m_nextContextId);
	if (!pContext){
		CLog::error("[TcpServer],context 分配错误……");
		return;
	}

	// 创建一个bufferevent，绑定socket，并托管给event_base
	// 注意，这里是托管给分配的工作线程的base，而不是监听线程的base
	pContext->m_bufev=bufferevent_socket_new(pWorkThread->m_base,fd, BEV_OPT_CLOSE_ON_FREE|LEV_OPT_THREADSAFE);
	if (pContext->m_bufev == NULL){
		CLog::error("[TcpServer],创建eventbuffer失败……");
		return;
	}

	//int ret = bufferevent_setfd(pContext->bufev, fd); //注意：这里如果是iocp会设置不成功，必须在 bufferevent_socket_new 就设置fd
	//evutil_socket_t testfd = bufferevent_getfd(pContext->m_bufev);

	// 设置bufev 三个回调函数
	bufferevent_setcb(pContext->m_bufev, &CTcpServer::DoRead, NULL, &CTcpServer::DoEvent, pContext);

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
	pContext->initContext(pServer, pWorkThread, fd, ++pServer->m_nextContextId);

	pServer->OnConnect(pContext);
}

void CTcpServer::DoRead(struct bufferevent* bev, void *ctx)
{
	struct evbuffer* input=bufferevent_get_input(bev);
	if (evbuffer_get_length(input)) 
	{
		CTcpContext* pContext = reinterpret_cast<CTcpContext*>(ctx);
		if (!pContext){
			CLog::error("[TcpServer],context不存在……");
			return;
		}

		CTcpServer* pTcpServer = pContext->GetOwnedTcpServer();
		if (!pTcpServer){
			CLog::error("[TcpServer],tcpServer不存在……");
			return;
		}

		int buffLen = evbuffer_get_length(input);	// evbuff 内的总字节数
		while (buffLen>0)
		{
			int freeLen = pContext->getFreeLen();
			if (freeLen<=0){ // 没有多余的空间,解包先
				if (!pContext->processPacket()){
					CLog::error("[TcpServer],解包错误，关闭链接……");
					pTcpServer->CloseContext(pContext);
					return;
				}
			}
			else{
				//bufferevent_read(bev, pContext->m_inbuf + pContext->m_readBegin, freeLen);
				int ret = evbuffer_remove(input, pContext->m_inbuf + pContext->m_inbufLen, freeLen);
				if (ret == -1){ // 读取错误
					CLog::error("[TcpServer],buffer 读取失败，关闭链接……");
					pTcpServer->CloseContext(pContext);
					return;
				}

				buffLen -= ret;
				pContext->m_inbufLen += ret;
			}
		}
		
		// 已经取出了所有
		if (pContext->m_inbufLen>0)
			pContext->processPacket();
	}
}

void CTcpServer::DoEvent(struct bufferevent *bev, short error, void *ctx)
{
	CTcpContext* pContext = reinterpret_cast<CTcpContext*>(ctx);
	if (!pContext)
		return;

	if (error&EVBUFFER_TIMEOUT)
	{
		
	}
	else if (error&EVBUFFER_EOF)
	{
		//printf("EVBUFFER_EOF! \n");
	}
	else if (error&EVBUFFER_ERROR)
	{
		//printf("EVBUFFER_ERROR! \n");
	}

	if (pContext)
		pContext->disconnect();
}

void CTcpServer::DoAcceptError(evconnlistener *listener, void* ctx)
{
	printf("监听错误回调! \n");
}

THREAD_RETURN CTcpServer::_listener_thread_(void* pParam)
{
	CTcpServer* pTcpServer=reinterpret_cast<CTcpServer*>(pParam);
	if (pTcpServer==NULL)
		return -1;

	CLog::info("[accept]线程开始启动，线程id = [%d]……", CTcpThread::getCurrentThreadID());
	event_base_dispatch(pTcpServer->m_pListenerBase);
	CLog::info("[accept]线程关闭……");
	return ::GetCurrentThreadId();
}

THREAD_RETURN CTcpServer::_working_thread_(void* pParam)
{
	CTcpThread* pWorkThread=reinterpret_cast<CTcpThread*>(pParam);
	if (pWorkThread==NULL)
		return -1;

	if (pWorkThread->m_base == NULL)
		return -1;

	CLog::info("[work]线程开始启动，线程id = [%d] ……", CTcpThread::getCurrentThreadID());
	//event_base_dispatch(pWorkThread->m_base);
	event_base_loop(pWorkThread->m_base,EVLOOP_NO_EXIT_ON_EMPTY);
	CLog::info("[work]线程关闭……");
	return ::GetCurrentThreadId();
}