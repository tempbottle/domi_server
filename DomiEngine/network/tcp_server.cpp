/******************************************************************** 
����ʱ��:        2015/06/22 20:27
�ļ�����:        tcp_server.cpp
�ļ�����:        Domi
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
		CLog::error("[TcpServer],work�߳�������,threadNum=%d����", threadNum);
		return false;
	}

	if (!port || port<10000){
		CLog::error("[TcpServer],�����˿ڴ���,port=%d����", port);
		return false;
	}

	m_pListenerBase = new_event_base();
	if (m_pListenerBase == NULL){// event_base ����ʧ��
		CLog::error("[TcpServer],listen eventbase ����ʧ�ܡ���");
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
		CLog::error("[TcpServer],tcp��������ʧ�ܡ���");
		return false;
	}

	// ����lister����ص�
	evconnlistener_set_error_cb(m_pListener, DoAcceptError);

	// �����߳�
	if (!m_clListenerThread.create(&CTcpServer::_listener_thread_, this)){
		CLog::error("[TcpServer],tcp�����̴߳���ʧ�ܡ���");
		return false;
	}

	return true;
}

bool CTcpServer::StartServer()
{
	if (!StarLister())
		return false;

	CTcpThread* pWorkThread = nullptr;
	
	for (uint32 i = 0; i<m_unWorkThreadNum; ++i){	// ��ʼ��work�߳�
		pWorkThread = new CTcpThread();	// new �����ţ���ʾ��ʽ���ù��캯�����������ű�ʾ��ʽ���ù��캯��
		if (pWorkThread && pWorkThread->initialize(this)){
			m_vecWorkThread.push_back(pWorkThread);
			pWorkThread->create(&CTcpServer::_working_thread_,pWorkThread);
		}
		else{
			CLog::error("[TcpServer],��[%d]�������̴߳���ʧ�ܡ���",i+1);
		}
	}

	return true;
}

bool CTcpServer::StopServer()
{
	// ��ʱ2s�رռ����߳�
	timeval delay = { 2, 0 };
	event_base_loopexit(this->m_pListenerBase, &delay);

	// ��ʱ2s�رչ����߳�
	for (uint32 i = 0; i < m_vecWorkThread.size(); ++i){
		CTcpThread* pWork = m_vecWorkThread[i];
		if (pWork)
			event_base_loopexit(pWork->m_base, &delay);
	}
	
	// �ر�listener�߳�
	m_clListenerThread.wait_exit();	// �ȴ��߳��˳�
	evconnlistener_free(m_pListener);
	event_base_free(m_pListenerBase);

	// �ر�work�߳�
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
	evutil_closesocket(pContext->m_fd);	// �Ͽ�bufferevent����Ҫ�ر��׽��֣������������
	bufferevent_free(pContext->m_bufev);
	bufferevent_unlock(pContext->m_bufev);

	pContext->initialize();	// ��λ

	// ����client���Ӷ����߼�
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
	// ��ס����map���ҵ���Ӧ��client
	CCritLocker connLock(ConnectLock());
	CTcpConnection* pConn = ConnectFind(pContext);
	if (pConn){
		// ȷ��client���Ӳ���timer�߳��д����߼�
		CCritLocker clApplyLock(pConn->getApplyMutex());
		pConn->onDisconnect();
		//pConn->initialize();
	}
	ConnectDel(pContext);	// ���տͻ�������
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
		//һ��Э���������ͷ��û���꣬�����ѭ�������ߵȴ���һ��libeventʱ�����ѭ����
		PacketHead* pHead = (PacketHead*)(pContext->m_inbuf + pContext->m_readBegin);
		if (pHead->uPacketSize > MaxBuffLen || pHead->uPacketSize < sizeof(PacketHead)){ // ��Ϣ��ͷ���Ϸ�
			pContext->disconnect();
			return false;
		}

		int len = pContext->getPendingLen();
		if (pHead->uPacketSize > pContext->getPendingLen()){ // ʣ�����ݲ���һ������������
			printf("ʣ�����ݲ���һ�����������գ�\n");
			break;
		}

		if (pContext->m_readBegin > pContext->m_inbufLen){ // �������ˣ��Ѷ����ֽ�����Ȼ����������ֱ�Ӷϵ�����
			this->CloseContext(pContext);
			return false;
		}

		// ѹ��message
		CCritLocker connMapLock(ConnectLock());
		CTcpConnection* pConn = ConnectFind(pContext);
		if (pConn)
			this->m_clMessageQueue.push(pConn->getApplyKey(), pContext, pContext->m_inbuf + pContext->m_readBegin + sizeof(PacketHead), pHead->uPacketSize);
		
		//������һ��Э���
		pContext->m_readBegin += pHead->uPacketSize;
	}

	// ����һ���������ݣ��ƶ�����ʼλ��
	int pending = pContext->getPendingLen();
	if (pending > 0)	// ����Ҫ�ƶ����ֽ�
		memmove(pContext->m_inbuf, pContext->m_inbuf + pContext->m_readBegin, pending);

	pContext->m_inbufLen = pending;
	pContext->m_readBegin = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// static method
void CTcpServer::DoAccept(evconnlistener *listener, evutil_socket_t fd, sockaddr *sa, int socklen, void *user_data)
{
	//CLog::info("tcp accept �߳�id = [%d] ", CTcpThread::getCurrentThreadID());
	if (fd <=0 ){
		CLog::error("[TcpServer],��Ч��socket fd = [%d]����", fd);
		return;
	}
	
	CTcpServer* pServer = (CTcpServer *)user_data;
	int nCurrent = (pServer->GetCurWorker()++) % pServer->GetWorkThreadNum();	//�����̸߳��ؾ���
	CTcpThread* pWorkThread = pServer->m_vecWorkThread[nCurrent];
	
	if (!pWorkThread){
		CLog::error("[TcpServer],���������̴߳��󡭡�");
		return;
	}

	CCritLocker contextLocker(pServer->GetContextMapLock());
	CTcpContext* pContext = pServer->m_mapContext.allocate(pServer->m_nextContextId);
	if (!pContext){
		CLog::error("[TcpServer],context ������󡭡�");
		return;
	}

	// ����һ��bufferevent����socket�����йܸ�event_base
	// ע�⣬�������йܸ�����Ĺ����̵߳�base�������Ǽ����̵߳�base
	pContext->m_bufev=bufferevent_socket_new(pWorkThread->m_base,fd, BEV_OPT_CLOSE_ON_FREE|LEV_OPT_THREADSAFE);
	if (pContext->m_bufev == NULL){
		CLog::error("[TcpServer],����eventbufferʧ�ܡ���");
		return;
	}

	//int ret = bufferevent_setfd(pContext->bufev, fd); //ע�⣺���������iocp�����ò��ɹ��������� bufferevent_socket_new ������fd
	//evutil_socket_t testfd = bufferevent_getfd(pContext->m_bufev);

	// ����bufev �����ص�����
	bufferevent_setcb(pContext->m_bufev, &CTcpServer::DoRead, NULL, &CTcpServer::DoEvent, pContext);

	// ���� bufferevent��ˮλ����Ҫ�ڿ����ĵ� ��Դ��
	//bufferevent_setwatermark(pContext->bufev, EV_READ, 0, 10240);

	// д�볬ʱ
	struct timeval delayWriteTimeout;
	delayWriteTimeout.tv_sec = 2;//pServer->m_timeout_write;
	delayWriteTimeout.tv_usec = 0;

	// ��ȡ��ʱ
	struct timeval delayReadTimeout;
	delayReadTimeout.tv_sec = 2; //pServer->m_timeout_read;
	delayReadTimeout.tv_usec = 0;

	// ���ó�ʱʱ��
	//bufferevent_set_timeouts(pContext->m_bufev, /*&delayReadTimeout*/NULL, &delayWriteTimeout);
	evutil_make_socket_nonblocking(fd);
	bufferevent_enable(pContext->m_bufev, EV_READ | EV_WRITE);	// ����event_add����д�¼����뵽�¼�����������
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
			CLog::error("[TcpServer],context�����ڡ���");
			return;
		}

		CTcpServer* pTcpServer = pContext->GetOwnedTcpServer();
		if (!pTcpServer){
			CLog::error("[TcpServer],tcpServer�����ڡ���");
			return;
		}

		int buffLen = evbuffer_get_length(input);	// evbuff �ڵ����ֽ���
		while (buffLen>0)
		{
			int freeLen = pContext->getFreeLen();
			if (freeLen<=0){ // û�ж���Ŀռ�,�����
				if (!pContext->processPacket()){
					CLog::error("[TcpServer],������󣬹ر����ӡ���");
					pTcpServer->CloseContext(pContext);
					return;
				}
			}
			else{
				//bufferevent_read(bev, pContext->m_inbuf + pContext->m_readBegin, freeLen);
				int ret = evbuffer_remove(input, pContext->m_inbuf + pContext->m_inbufLen, freeLen);
				if (ret == -1){ // ��ȡ����
					CLog::error("[TcpServer],buffer ��ȡʧ�ܣ��ر����ӡ���");
					pTcpServer->CloseContext(pContext);
					return;
				}

				buffLen -= ret;
				pContext->m_inbufLen += ret;
			}
		}
		
		// �Ѿ�ȡ��������
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
	printf("��������ص�! \n");
}

THREAD_RETURN CTcpServer::_listener_thread_(void* pParam)
{
	CTcpServer* pTcpServer=reinterpret_cast<CTcpServer*>(pParam);
	if (pTcpServer==NULL)
		return -1;

	CLog::info("[accept]�߳̿�ʼ�������߳�id = [%d]����", CTcpThread::getCurrentThreadID());
	event_base_dispatch(pTcpServer->m_pListenerBase);
	CLog::info("[accept]�̹߳رա���");
	return ::GetCurrentThreadId();
}

THREAD_RETURN CTcpServer::_working_thread_(void* pParam)
{
	CTcpThread* pWorkThread=reinterpret_cast<CTcpThread*>(pParam);
	if (pWorkThread==NULL)
		return -1;

	if (pWorkThread->m_base == NULL)
		return -1;

	CLog::info("[work]�߳̿�ʼ�������߳�id = [%d] ����", CTcpThread::getCurrentThreadID());
	//event_base_dispatch(pWorkThread->m_base);
	event_base_loop(pWorkThread->m_base,EVLOOP_NO_EXIT_ON_EMPTY);
	CLog::info("[work]�̹߳رա���");
	return ::GetCurrentThreadId();
}