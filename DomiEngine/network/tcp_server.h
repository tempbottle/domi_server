/******************************************************************** 
����ʱ��:        2015/06/28 22:35
�ļ�����:        tcp_server.h
�ļ�����:        Domi
����˵��:        tcpServer  
����˵��:         
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
	bool Initialize(uint32 threadNum, uint16 port);			// tcpserver��ʼ��
	bool StarLister();										// ���������߳�
	bool StartServer();										// ������������
	bool StopServer();										// �رշ���
	void CloseContext(CTcpContext *pContext);				// �ر�ĳ������

	// [virtual]�ص�
	virtual void OnConnect(CTcpContext* pContext);			// ���������ӻص�
	virtual void OnDisConnect(CTcpContext* pContext);		// �Ͽ����ӻص�
	virtual void OnTimeoutWrite(CTcpContext* pContext);		// д��ʱ�ص�
	virtual void OnTimeoutRead(CTcpContext* pContext);		// ����ʱ�ص�
	virtual bool OnProcessPacket(CTcpContext* pContext);	// ���

	// �麯������ʵ��
	virtual CTcpConnection*	ConnectNew(CTcpContext* pContext) = 0;
	virtual CMutex*			ConnectLock() = 0;
	virtual CTcpConnection*	ConnectFind(CTcpContext* pContext) = 0;
	virtual void			ConnectDel(CTcpContext* pContext) = 0;

	// [static]���������ӻص�����
	static void DoAccept(evconnlistener *listener, evutil_socket_t fd,sockaddr *sa, int socklen, void *user_data);
	static void DoRead(struct bufferevent *bev, void *ctx);
	static void DoEvent(struct bufferevent *bev, short error, void *ctx);
	static void DoAcceptError(evconnlistener *listener,void* ctx);	// listener�ļ�������ص�
	static THREAD_RETURN _listener_thread_(void* pParam);
	static THREAD_RETURN _working_thread_(void* pParam);

private:
	// libevnt
	event_base*			m_pListenerBase;	//������Ӧ��
	event				m_evAccept;			//�����¼�
	evutil_socket_t		m_clListenerSocket;	//����socket
	evconnlistener*		m_pListener;		//��������

	// thread
	uint32				m_unWorkThreadNum;	//�����߳�����
	CThread				m_clListenerThread;	//�����߳�
	VECTOR_THREAD		m_vecWorkThread;	//�����̳߳�
	uint32				m_nCurrentWorker;	//�̸߳��ؾ���

	// context
	uint16				m_port;				//�����˿�
	MAP_CONTEXT			m_mapContext;		//�ͻ��������ļ���
	uint16				m_send_buffer_max;	//����ͻ���������
	timeval				m_timeout;
	
public:
	uint32				m_nextContextId;
	CMessageQueue		m_clMessageQueue;	// ��Ϣ����
};