/******************************************************************** 
创建时间:        2015/06/28 14:03
文件名称:        tcp_context.cpp
文件作者:        Domi
*********************************************************************/

#include <functional>
#include "tcp_context.h"
#include "tcp_server.h"
#include "common/log/log.h"
#include "common/basic/memoryFunctions.h"

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
	_connected = false;

	memset(m_inbuf, 0, MaxBuffLen);
	memset(m_outbuf, 0, MaxBuffLen);
}

// private method
void CTcpContext::initContext(CTcpServer* _network, CTcpThread*_thread, evutil_socket_t fd, uint32 id)
{
	m_clTcpServer = _network;
	m_clOwnerThread = _thread;
	m_fd = fd;
	m_ContextId = id;

	_connected = true;
}

bool CTcpContext::send(const char* pBuffer,int32 nSize)
{
	if (!m_bufev){
		disconnect();
		return false;
	}

	struct evbuffer* output = bufferevent_get_output(m_bufev);
	uint32 len = evbuffer_get_length(output);
	if (evbuffer_get_length(output)>=MaxBuffLenLimit){	// 积压太多消息
		disconnect();
		return false;
	}

	uint32 msgLen = sizeof(PacketHead) + nSize;
	PacketHead* head = (PacketHead*)(m_outbuf);
	head->uPacketSize = msgLen;
	head->uHeadFlag = 10;
	head->uVersion = 77;

	dMemcpy(m_outbuf + sizeof(PacketHead), sizeof(m_outbuf), pBuffer, nSize);

	if (bufferevent_write(m_bufev, m_outbuf, msgLen) == -1){
		disconnect();
		return false;
	}

	return true;
}

void CTcpContext::disconnect()
{
	if (m_clTcpServer){
		m_clTcpServer->CloseContext(this);
	}
}

bool CTcpContext::processPacket()
{
	if (!_connected)	// 没有链接完成
		return false;

	if (!m_clTcpServer)
		return false;

	return m_clTcpServer->OnProcessPacket(this);
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
