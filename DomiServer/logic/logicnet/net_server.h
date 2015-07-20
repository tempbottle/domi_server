/******************************************************************** 
����ʱ��:        2015/07/06 16:47
�ļ�����:        game_server.h
�ļ�����:        Domi
����˵��:        tcp server ������  
����˵��:        ���տͻ��������ϵ�tcp���ӣ�Ȼ�󽻸�Ӧ�ò㴦��
*********************************************************************/

#include "network/tcp_server.h"
#include "network/tcp_context.h"
#include "common/pool/map_pool.h"
#include "logic/logicnet/net_client.h"

class CNetServer : public CTcpServer
{
public:
	CNetServer() {}
	~CNetServer() {}

private:
	typedef CMapPool<CTcpContext*, CNetClient>	POOL_CLIENT;

private:
	virtual CMutex*				ConnectLock()							{ return &(m_mapClient.getLock()); }				// m_mapClient���߳���
	virtual CTcpConnection*		ConnectFind(CTcpContext* pContext)		{ return m_mapClient.find(pContext); }				// ����һ���ͻ�������
	virtual void				ConnectDel(CTcpContext* pContext)		{ if (pContext) m_mapClient.release(pContext); }	// ɾ��һ���ͻ�������
	virtual CTcpConnection*		ConnectNew(CTcpContext* pContext);

public:
	inline CNetClient* getClient(CTcpContext*_context, uint32 _key)
	{
		CCritLocker clLock(m_mapClient.getLock());
		CNetClient* pClient = m_mapClient.find(_context);
		if (pClient)
		{
			//pClient->setApplyLock(true);
			return pClient;
		}

		return nullptr;
	}

private:
	POOL_CLIENT	m_mapClient;
};
