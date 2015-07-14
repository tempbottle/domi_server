/******************************************************************** 
����ʱ��:        2015/06/28 13:50
�ļ�����:        tcp_context.h
�ļ�����:        Domi
����˵��:        �ͻ���������  
����˵��:         
*********************************************************************/

#pragma once

#ifdef WIN32
#include <winsock2.h>
#include <mswsock.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment (lib,"wsock32.lib")
#else
#include <netinet/in.h>//������<sys/socket.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>//ϵͳ���Ͷ���
#include <arpa/inet.h>
#include <fcntl.h>//����ģʽͷ�ļ�
#include <netdb.h>
#include <unistd.h>//close����
#endif
#include <errno.h>	//�������ļ��ͻ�����errno������int geterror()��
#include "tcp_thread.h"
#include "libevent.h"

#define MaxBuffLen 1024*10	// 4k

//////////////////////////////////////////////////////////////////////////
// Э���ͷ
struct PacketHead
{
	uint16	uHeadFlag;		// ��ͷ��ʾ
	uint16	uPacketSize;	// ����С
	uint16	uVersion;		// �汾��Ϣ

	PacketHead() {	memset( this, 0, sizeof( *this ) );	}
};

//////////////////////////////////////////////////////////////////////////
class CTcpServer;
class CTcpThread;
class CTcpContext		//״̬�С�������|������|�Ͽ��С�
{
public:
	friend class CTcpServer;

public:
	CTcpContext();
	virtual ~CTcpContext(){}

private:
	void initialize();	// ��ʼ��
	void initContext(CTcpServer* _network, CTcpThread*_thread, evutil_socket_t fd, uint32 id = 0);

public:
	bool	send(const char* pBuffer,int32 nSize);
	void	disconnect();
	ulong	remote_address();		// Զ�̵�ַ
	const char*	remote_ip();		// Զ�̵�ַ��ip

public:
	inline int getPendingLen() { return m_inbufLen - m_readBegin; }	// ʣ��δ������ֽ���
	inline int getFreeLen()	{ return MaxBuffLen - m_inbufLen; }		// ����apend�ĳ���

public:
	CTcpServer*	m_clTcpServer;		// ������server
	CTcpThread*	m_clOwnerThread;	// �������߳�
	CMutex	m_csLock;

	evutil_socket_t m_fd;			// socket fd
	struct bufferevent* m_bufev;	// bufferevent 
	uint32 m_ContextId;				// ��������

	char m_inbuf[MaxBuffLen];		// buffer���棬����Ļ��������ķְ�
	int m_inbufLen;					// inbuff ����λ��
	int m_readBegin;				// ��һ�ο�ʼ��ȡ��λ��

	// volatile �ؼ���ʹ�� 
	volatile bool	_connected;		//�Ƿ����������
	volatile bool	_disconnect;	//�Ͽ���
	volatile uint64	_connect_tick;	//����ʱ��
	volatile uint64	_read_tick;		//��ȡʱ��
	volatile uint64	_write_tick;	//д��ʱ��
};
