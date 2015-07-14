/******************************************************************** 
����ʱ��:        2015/06/22 20:27
�ļ�����:        tcp_server.cpp
�ļ�����:        Domi
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
	CLog::info("tcp server ��������");
	m_pListenerBase = new_event_base();
	if (m_pListenerBase==NULL)	// event_base ����ʧ��
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

	// �����߳�
	m_clListenerThread.create(&CTcpServer::_listener_thread_,this);

	CTcpThread* pWorkThread = nullptr;
	// ��ʼ��work�߳�
	for (uint32 i=0;i<m_unWorkThreadNum;++i)
	{
		pWorkThread = new CTcpThread();	// new �����ţ���ʾ��ʽ���ù��캯�����������ű�ʾ��ʽ���ù��캯��
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
	printf("��ʼ�ر�server��\n");
	// ��ʱ2s�ر�
	struct timeval delay = { 2, 0 };
	event_base_loopexit(this->m_pListenerBase, &delay);

	// ����base ��ʱ2s
	for (uint32 i = 0; i < m_vecWorkThread.size(); ++i)
	{
		CTcpThread* pWork = m_vecWorkThread[i];
		if (pWork)
			event_base_loopexit(pWork->m_base, &delay);
	}
	
	// �ر�listener�߳�
	m_clListenerThread.wait_exit();	// �ȴ��߳��˳�
	evconnlistener_free(m_pListener);
	event_base_free(m_pListenerBase);
	CLog::warn("[accept]�̹߳ر���ɡ���");

	// �ر�work�߳�
	for (uint32 i=0;i<m_vecWorkThread.size();++i)
	{
		CTcpThread* pWork = m_vecWorkThread[i];
		if (!pWork)
			continue;

		pWork->wait_exit();
		event_base_free(pWork->m_base);
		delete pWork;
	}

	CLog::warn("[work]�̹߳ر���ɡ���");

	return true;
}

void CTcpServer::CloseContext(CTcpContext *pContext)
{
	// ��סcontext����ֹtcpserver��DoRead�߳̽���buffer��ȡ
	CCritLocker lock(pContext->m_csLock);
	printf("info: �߳�id = %d��\n", CTcpThread::getCurrentThreadID());
	bufferevent_lock(pContext->m_bufev);
	bufferevent_disable(pContext->m_bufev, EV_READ | EV_WRITE);
	evutil_closesocket(pContext->m_fd);
	bufferevent_free(pContext->m_bufev);
	bufferevent_unlock(pContext->m_bufev);
	pContext->initialize();

	CCritLocker contextLocker(m_mapContext.getLock());
	m_mapContext.release(pContext->m_ContextId);
	this->OnDisConnect(pContext);
	printf("info: CTcpServer::CloseContext��\n");
}

// ���������ӵĻص�
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

// �Ͽ����ӻص�
void CTcpServer::OnDisConnect(CTcpContext* pContext)
{
	// ��ס����map���ҵ���Ӧ��client
	CCritLocker connLock(ConnectLock());
	CTcpConnection* pConn = ConnectFind(pContext);
	if (pConn)
	{
		// ȷ��client���Ӳ���timer�߳��д����߼�
		CCritLocker clApplyLock(pConn->getApplyMutex());
		pConn->onDisconnect();
		pConn->initialize();
	}
	ConnectDel(pContext);	// ���տͻ�������
}

// д��ʱ�ص�
void CTcpServer::onTimeoutWrite(CTcpContext* pContext)
{

}

// ����ʱ�ص�
void CTcpServer::onTimeoutRead(CTcpContext* pContext)
{
}

// ���
bool CTcpServer::OnProcessPacket(CTcpContext* pContext)
{
	// ��������Ƿ��Ѿ����
	// TODO

	while (pContext->m_inbuf && pContext->getPendingLen() > sizeof(PacketHead))
	{
		//һ��Э���������ͷ��û���꣬�����ѭ�������ߵȴ���һ��libeventʱ�����ѭ����
		PacketHead* pHead = (PacketHead*)(pContext->m_inbuf + pContext->m_readBegin);

		// ��Ϣ��ͷ���Ϸ�
		if (pHead->uPacketSize > MaxBuffLen || pHead->uPacketSize < sizeof(PacketHead))
		{
			//��������Ϸ�
			pContext->disconnect();
			return false;
		}

		// ʣ�����ݲ���һ������������
		if (pHead->uPacketSize > pContext->getPendingLen())
		{
			printf("ʣ�����ݲ���һ�����������գ�\n");
			return false;
		}

		if (pContext->m_readBegin > pContext->m_inbufLen)
		{
			// �������ˣ��Ѷ����ֽ�����Ȼ����������ֱ�Ӷϵ�����
			printf("�������ˣ��Ѷ����ֽ�����Ȼ����������ֱ�Ӷϵ����ӣ�\n");
			this->CloseContext(pContext);
			return false;
		}

		// ѹ��message
		CCritLocker connMapLock(ConnectLock());
		CTcpConnection* pConn = ConnectFind(pContext);
		if (pConn)
			this->m_clMessageQueue.push(pConn->getApplyKey(), pContext, pContext->m_inbuf + pContext->m_readBegin, pHead->uPacketSize);
		
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

//-------------------------------------------------------------------
// static method
// ���������ӻص�����
void CTcpServer::DoAccept(evconnlistener *listener, evutil_socket_t fd, sockaddr *sa, int socklen, void *user_data)
{
	CLog::info("tcp accept �߳�id = [%d] ", CTcpThread::getCurrentThreadID());
	if (fd <=0 )
	{
		printf("��Ч��socket��fd = [ %d] \n", fd);
		return;
	}
	
	CTcpServer* pServer = (CTcpServer *)user_data;
	int nCurrent = (pServer->nCurrentWorker++) % pServer->m_unWorkThreadNum;	//�����̸߳��ؾ���
	CTcpThread* pWorkThread = pServer->m_vecWorkThread[nCurrent];
	
	if (!pWorkThread)
	{
		printf("���������̴߳���\n");
		return;
	}

	CCritLocker contextLocker(pServer->m_mapContext.getLock());
	CTcpContext* pContext = pServer->m_mapContext.allocate(pServer->m_nextContextId);
	if (!pContext)
	{
		printf("context �������\n");
		return;
	}

	// ����һ��bufferevent����socket�����йܸ�event_base
	// ע�⣬�������йܸ�����Ĺ����̵߳�base�������Ǽ����̵߳�base
	pContext->m_bufev=bufferevent_socket_new(pWorkThread->m_base,fd, BEV_OPT_CLOSE_ON_FREE|LEV_OPT_THREADSAFE);
	if (pContext->m_bufev == NULL)
	{
		printf("����eventbufferʧ�ܣ�\n");
		return;
	}

	//int ret = bufferevent_setfd(pContext->bufev, fd); //ע�⣺���������iocp�����ò��ɹ��������� bufferevent_socket_new ������fd
	//evutil_socket_t testfd = bufferevent_getfd(pContext->m_bufev);

	// ����bufev �����ص�����
	bufferevent_setcb(pContext->m_bufev, &CTcpServer::DoRead, NULL, DoEvent, pContext);

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
	pContext->initContext(pServer, pWorkThread, fd, pServer->m_nextContextId);
	++pServer->m_nextContextId;

	pServer->OnConnect(pContext);
}

void CTcpServer::DoRead(struct bufferevent* bev, void *ctx)
{
	//CLog::info("tcp work �߳�id = [%d] ", CTcpThread::getCurrentThreadID());
	struct evbuffer* input=bufferevent_get_input(bev);
	if (evbuffer_get_length(input)) 
	{
		CTcpContext* pContext = reinterpret_cast<CTcpContext*>(ctx);
		CCritLocker lock(pContext->m_csLock);
		int buffLen = evbuffer_get_length(input);	// evbuff �ڵ����ֽ���
		while (buffLen>0)
		{
			//printf("buffLen = %d\n", buffLen);
			int freeLen = pContext->getFreeLen();
			if (freeLen<=0)
			{
				// û�ж���Ŀռ�,�����
				if (!pContext->m_clTcpServer->OnProcessPacket(pContext))
				{
					printf("OnProcessPacket ���󣡣�\n");
					return;
				}
			}
			else
			{
				//bufferevent_read(bev, pContext->m_inbuf + pContext->m_readBegin, freeLen);
				int ret = evbuffer_remove(input, pContext->m_inbuf + pContext->m_readBegin, freeLen);
				if (ret == -1)
				{
					// ��ȡ����
					printf("evbuffer_remove ���󣡣�\n");
					return;
				}

				buffLen -= ret;
				pContext->m_inbufLen += ret;
			}
		}
		
		// �Ѿ�ȡ��������
		if (pContext->m_inbufLen>0)
			pContext->m_clTcpServer->OnProcessPacket(pContext);
	}
}

void CTcpServer::DoEvent(struct bufferevent *bev, short error, void *ctx)
{
	CLog::info("tcp event �߳�id = [%d] ", CTcpThread::getCurrentThreadID());
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
	printf("��������ص�! \n");
}

// �����߳�
THREAD_RETURN CTcpServer::_listener_thread_(void* pParam)
{
	CTcpServer* pTcpServer=reinterpret_cast<CTcpServer*>(pParam);
	if (pTcpServer==NULL)
	{
		return -1;
	}

	CLog::info("[accept]�߳̿�ʼ�������߳�id = [%d]����", CTcpThread::getCurrentThreadID());
	event_base_dispatch(pTcpServer->m_pListenerBase);
	CLog::info("[accept]�̹߳رա���");
	return ::GetCurrentThreadId();
}

// �����߳�
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

	CLog::info("[work]�߳̿�ʼ�������߳�id = [%d] ����", CTcpThread::getCurrentThreadID());
	event_base_loop(pWorkThread->m_base,EVLOOP_NO_EXIT_ON_EMPTY);
	CLog::info("[work]�̹߳رա���");
	return ::GetCurrentThreadId();
}