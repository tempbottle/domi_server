/******************************************************************** 
创建时间:        2015/06/28 13:50
文件名称:        tcp_context.h
文件作者:        Domi
功能说明:        客户端上下文  
其他说明:         
*********************************************************************/

#pragma once

#ifdef WIN32
#include <winsock2.h>
#include <mswsock.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment (lib,"wsock32.lib")
#else
#include <netinet/in.h>//包含了<sys/socket.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>//系统类型定义
#include <arpa/inet.h>
#include <fcntl.h>//阻塞模式头文件
#include <netdb.h>
#include <unistd.h>//close函数
#endif
#include <errno.h>	//包含此文件就会设置errno变量【int geterror()】
#include "tcp_thread.h"
#include "libevent.h"

#define MaxBuffLen 1024*10	// 4k

//////////////////////////////////////////////////////////////////////////
// 协议包头
struct PacketHead
{
	uint16	uHeadFlag;		// 包头标示
	uint16	uPacketSize;	// 包大小
	uint16	uVersion;		// 版本信息

	PacketHead() {	memset( this, 0, sizeof( *this ) );	}
};

//////////////////////////////////////////////////////////////////////////
class CTcpServer;
class CTcpThread;
class CTcpContext		//状态有【链接中|已链接|断开中】
{
public:
	friend class CTcpServer;

public:
	CTcpContext();
	virtual ~CTcpContext(){}

private:
	void initialize();	// 初始化
	void initContext(CTcpServer* _network, CTcpThread*_thread, evutil_socket_t fd, uint32 id = 0);

public:
	bool	send(const char* pBuffer,int32 nSize);
	void	disconnect();
	ulong	remote_address();		// 远程地址
	const char*	remote_ip();		// 远程地址的ip

public:
	inline int getPendingLen() { return m_inbufLen - m_readBegin; }	// 剩余未处理的字节数
	inline int getFreeLen()	{ return MaxBuffLen - m_inbufLen; }		// 可以apend的长度

public:
	CTcpServer*	m_clTcpServer;		// 所属的server
	CTcpThread*	m_clOwnerThread;	// 所属的线程
	CMutex	m_csLock;

	evutil_socket_t m_fd;			// socket fd
	struct bufferevent* m_bufev;	// bufferevent 
	uint32 m_ContextId;				// 连接索引

	char m_inbuf[MaxBuffLen];		// buffer缓存，这里的缓存用来的分包
	int m_inbufLen;					// inbuff 结束位置
	int m_readBegin;				// 下一次开始读取的位置

	// volatile 关键字使用 
	volatile bool	_connected;		//是否已完成链接
	volatile bool	_disconnect;	//断开中
	volatile uint64	_connect_tick;	//链接时间
	volatile uint64	_read_tick;		//读取时间
	volatile uint64	_write_tick;	//写入时间
};
