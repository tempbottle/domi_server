/******************************************************************** 
创建时间:        2015/06/28 14:03
文件名称:        tcp_context.cpp
文件作者:        Domi
*********************************************************************/

#include "tcp_context.h"
#include "tcp_server.h"

CTcpContext::CTcpContext()
{
	initialize();
}

// 初始化
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
	printf("context 断开连接! \n");
	if (m_clTcpServer)
	{
		m_clTcpServer->CloseContext(this);
	}	
}

// 远程地址
ulong CTcpContext::remote_address()
{
	return 0;
}

// 远程地址的ip
const char*	CTcpContext::remote_ip()
{
	return "test";
}
