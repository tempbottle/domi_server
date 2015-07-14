/******************************************************************** 
����ʱ��:        2015/06/30 20:02
�ļ�����:        tcp_content.h
�ļ�����:        Domi
����˵��:        tcp content ���� 
����˵��:         
*********************************************************************/

#pragma once
#include "network/libevent.h"
#include "common/thread/csLocker.h"
#include "tcp_context.h"

class CTcpConnection
{
public:
	CTcpConnection();
	~CTcpConnection() {}

public:
	virtual void initialize();
	virtual void disconnect();
	virtual void onKickOut();
	virtual pc_str	remote_ip();
	virtual ulong	remote_address();

	// �ص�
	virtual void onConnect();		// �����ϵĻص�
	virtual void onDisconnect();	// �Ͽ�����

public:
	// ��������
	//inline void	bindPlayer(){ }
	inline void	bindContext(CTcpContext* pContext)	{ m_pContext = pContext; }	// ��context
	inline CMutex* getApplyMutex()					{ return &m_apply_mutex; }
	inline uint32  getApplyKey()					{ return m_apply_key;    }
	inline void	setApplyLock(bool _lock)			{ if (_lock)m_apply_mutex.lock(); else m_apply_mutex.unlock();}

private:
	uint32		m_apply_key;	//����key,ȷ����������
	CMutex		m_apply_mutex;	//ȷ���̰߳�ȫ

	CTcpContext* m_pContext;	// �ͻ���������
};