/******************************************************************** 
����ʱ��:        2015/06/28 22:35
�ļ�����:        tcp_server.h
�ļ�����:        Domi
����˵��:        tcpServer  
����˵��:         
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
	static int32 _qsort_thread	(const void *p1, const void *p2);	//���߳�ʹ�ô�����������

public:
	inline void SetWorkThreadNum(uint32 num) { m_unWorkThreadNum = num; }

	void SetTimeout(uint64 _timeout_connect,uint64 _timeout_read,uint64 _timeout_write);
	bool StartServer();
	bool StopServer();
	void CloseContext(CTcpContext *pContext);

	// �ص�
	virtual void OnConnect(CTcpContext* pContext);				// ���������ӻص�
	virtual void OnDisConnect(CTcpContext* pContext);			// �Ͽ����ӻص�
	virtual void onTimeoutWrite(CTcpContext* pContext);			// д��ʱ�ص�
	virtual void onTimeoutRead(CTcpContext* pContext);			// ����ʱ�ص�
	virtual bool OnProcessPacket(CTcpContext* pContext);		// ���

	// �麯������ʵ��
	virtual CTcpConnection*	ConnectNew(CTcpContext* pContext) = 0;
	virtual CMutex*			ConnectLock() = 0;
	virtual CTcpConnection*	ConnectFind(CTcpContext* pContext) = 0;
	virtual void			ConnectDel(CTcpContext* pContext) = 0;
public:
	// ���������ӻص�����
	static void DoAccept(evconnlistener *listener, evutil_socket_t fd,sockaddr *sa, int socklen, void *user_data);
	static void DoRead(struct bufferevent *bev, void *ctx);
	static void DoEvent(struct bufferevent *bev, short error, void *ctx);
	static void DoAcceptError(evconnlistener *listener,void* ctx);	// listener�ļ�������ص�
	static THREAD_RETURN	_listener_thread_(void* pParam);
	static THREAD_RETURN	_working_thread_(void* pParam);

public:
	// libevnt
	event_base*			m_pListenerBase;	//������Ӧ��
	event				m_evAccept;			//�����¼�
	evutil_socket_t 	m_clListenerSocket;	//����socket
	evconnlistener*		m_pListener;		//��������

	// thread
	CThread				m_clListenerThread;	//�����߳�
	VECTOR_THREAD		m_vecWorkThread;	//�����̳߳�
	uint32				m_unWorkThreadNum;	//�����߳�����
	uint32				nCurrentWorker;		//�̸߳��ؾ���
	CCondEvent			m_clExitEvent;		//�˳�ѭ���¼�,�߳�ͬ����������

	// 
	MAP_CONTEXT			m_mapContext;		//�ͻ��������ļ���
	uint16				m_send_buffer_max;	//����ͻ���������
	timeval				m_timeout;
	uint64				m_timeout_connect;	//���ӳ�ʱ(��)
	uint64				m_timeout_read;		//��ȡ��ʱʱ��(��)
	uint64				m_timeout_write;	//д�볬ʱʱ��(��)

	std::map<CTcpContext*, CTcpConnection*> m_mapContent;	// �ͻ�������
	CMessageQueue		m_clMessageQueue;	// ��Ϣ����
	CMutex				m_Mutex;			// ������,������context����

	uint32 m_nextContextId;
};