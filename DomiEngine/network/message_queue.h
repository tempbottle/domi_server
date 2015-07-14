/******************************************************************** 
����ʱ��:        2015/07/01 9:56
�ļ�����:        CMessageQueue.h
�ļ�����:        Domi
����˵��:        ��Ϣ����  
����˵��:        ����˫�����л���
*********************************************************************/

#pragma once
#include "common/pool/queue_pool.h"
#include "network/tcp_connection.h"
#include "network/net_message.h"

class CMessageQueue
{
public:
	enum
	{
		message_pop	,//ȡ������
		message_push,//�������
		message_max
	};

	typedef CQueuePool<_stNetMessage> msg_queue;

public:
	CMessageQueue()
	{
		reset();
		queue_max(60000);	// ÿ�����������6w����Ϣ
	}
	virtual ~CMessageQueue(){}

public:
	inline void			queue_max(uint32 _max)	{	m_queue_max = _max;				}
	inline msg_queue*	get_pop	()				{	return m_message[message_pop];	}
	inline msg_queue*	get_push()				{	return m_message[message_push];	}

public:
	inline void	reset();					// ��λ(����,������߳�����д)
	inline void	swap_queue();				// ������Ϣ����(����,������߳�����д)
	inline _stNetMessage* front();			// ȡ����Ϣͷ(���̲߳���Ҫ����)
	inline void	pop();						// ȡ����Ϣ(���̲߳���Ҫ����)
	inline bool	push(uint32 _apply_key, CTcpContext* pContext,char*_buffer, uint16 _size);	// д����Ϣ(���߳���Ҫ����)

protected:
	uint32		m_queue_max;				//�����������
	msg_queue	m_instance[message_max];	//��Ϣʵ��
	msg_queue*	m_message[message_max];		//��Ϣָ��
};

//-----------------------------------------------------------------------
// ������Ϣ����
inline void	CMessageQueue::reset()
{
	try
	{
		CCritLocker clLock_pop(m_instance[message_pop].get_mutex());
		CCritLocker clLock_push(m_instance[message_push].get_mutex());

		for (int i = 0;i < message_max;i++)
		{
			m_message[i] = &m_instance[i];
			m_instance[i].clear();
		}
	}
	catch (...)
	{
	}
}

// ������Ϣ����
inline void	CMessageQueue::swap_queue()
{
	try
	{
		//�л�ʱ��Ҫ������������
		CCritLocker clLock_pop(m_instance[message_pop].get_mutex());
		CCritLocker clLock_push(m_instance[message_push].get_mutex());

		msg_queue* _temp			= m_message[message_pop	];
		m_message[message_pop	]	= m_message[message_push];
		m_message[message_push	]	= _temp;
	}
	catch (...)
	{
	}
}

inline _stNetMessage* CMessageQueue::front()
{
	try
	{
		if(m_message[message_pop])
			return m_message[message_pop]->front();
	}
	catch (...)
	{
	}

	return nullptr;
}

inline void	CMessageQueue::pop()
{
	try
	{
		if(m_message[message_pop])
			return m_message[message_pop]->pop();
	}
	catch (...)
	{
	}
}

inline bool	CMessageQueue::push(uint32 _apply_key,CTcpContext* pContext,char* _buffer,uint16 _size)
{
	try
	{
		if(!m_message[message_push])
			return false;

		//������
		CCritLocker clLock(m_message[message_push]->get_mutex());
		if(m_queue_max && m_message[message_push]->size() > m_queue_max)
			return false;

		_stNetMessage* pMsg = m_message[message_push]->push();
		if(!pMsg)
			return false;

		pMsg->reset(_apply_key, pContext, _buffer, _size);

		return true;
	}
	catch (...)
	{
	}
	return false;
}