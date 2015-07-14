/******************************************************************** 
����ʱ��:        2015/07/01 10:14
�ļ�����:        net_message.h
�ļ�����:        Domi
����˵��:        ��װ��message  
����˵��:        1�ֽڶ���
*********************************************************************/
#pragma once
#include "common/basic/basicTypes.h"
#include "network/tcp_context.h"
#include "network/net_message.pb.h"
using namespace NetMessage;

/*
Э������ʹ��google protobuf
message NetMessage{
	uint16 uProtocol = 1;	// Э���
	uint8 type = 2;			// ��Ϣ�����ͣ�1=���������ȥ�İ�����2 = ���ذ��������İ���
	optional bytes content = 3;	// ��Ϣ�Լ�
}
*/

#pragma pack(push,1)
struct _stNetMessage 
{
	uint32		 _apply_key;						//����key
	CTcpContext* _context;							//Ӧ��������
	uint16		 _size;								//���ݴ�С
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
