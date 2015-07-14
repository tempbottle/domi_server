/******************************************************************** 
创建时间:        2015/07/01 10:14
文件名称:        net_message.h
文件作者:        Domi
功能说明:        封装的message  
其他说明:        1字节对齐
*********************************************************************/
#pragma once
#include "common/basic/basicTypes.h"
#include "network/tcp_context.h"
#include "network/net_message.pb.h"
using namespace NetMessage;

/*
协议数据使用google protobuf
message NetMessage{
	uint16 uProtocol = 1;	// 协议号
	uint8 type = 2;			// 消息包类型，1=请求包（出去的包），2 = 返回包（进来的包）
	optional bytes content = 3;	// 消息自己
}
*/

#pragma pack(push,1)
struct _stNetMessage 
{
	uint32		 _apply_key;						//引用key
	CTcpContext* _context;							//应用上下文
	uint16		 _size;								//数据大小
	MessagePack  m_proto;							//protobuf

	_stNetMessage()
		:_apply_key(0)
		,_context(nullptr)
		, _size(0)
	{
		//printf("MessagePack = %d \n", sizeof(MessagePack));
		//m_proto.Clear();
	}

	inline void	reset(uint32 apply_key, CTcpContext* context,char* buffer, uint16 size)
	{
		_context = context;
		_apply_key = apply_key;
        
		//m_proto.SerializeToArray(buffer, size);
		//printf("%s\n", buffer + 6);
		//printf("context = %d\n", context->m_ContextId);
	}
};
/*************************************************************/
#pragma pack(pop)
