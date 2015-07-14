/******************************************************************** 
����ʱ��:        2015/06/28 14:03
�ļ�����:        tcp_context.cpp
�ļ�����:        Domi
*********************************************************************/

#include "tcp_context.h"
#include "tcp_server.h"

CTcpContext::CTcpContext()
{
	initialize();
}

// ��ʼ��
void CTcpContext::initialize()
{
	m_fd = 0;
	m_bufev = nullptr;
	m_inbufLen = 0;
	m_readBegin = 0;
	m_clOwnerThread = nullptr;
	m_clTcpServer = nullptr;

	memset(m_inbuf, 0, MaxBuffLen);
}

// private method
void CTcpContext::initContext(CTcpServer* _network, CTcpThread*_thread, evutil_socket_t fd, uint32 id)
{
	m_clTcpServer = _network;
	m_clOwnerThread = _thread;
	m_fd = fd;
	m_ContextId = id;
}

bool CTcpContext::send(const char* pBuffer,int32 nSize)
{
	return true;
}

void CTcpContext::disconnect()
{
	printf("context �Ͽ�����! \n");
	if (m_clTcpServer)
	{
		m_clTcpServer->CloseContext(this);
	}	
}

// Զ�̵�ַ
ulong CTcpContext::remote_address()
{
	return 0;
}

// Զ�̵�ַ��ip
const char*	CTcpContext::remote_ip()
{
	return "test";
}