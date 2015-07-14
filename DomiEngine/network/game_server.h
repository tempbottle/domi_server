/******************************************************************** 
����ʱ��:        2015/07/06 16:47
�ļ�����:        game_server.h
�ļ�����:        Domi
����˵��:        tcp server ������  
����˵��:         
*********************************************************************/

#include "network/tcp_server.h"
#include "network/tcp_client.h"
#include "network/tcp_context.h"
#include "common/pool/map_pool.h"

class GameServer : public CTcpServer
{
public:
	GameServer() {}
	~GameServer() {}

private:
	typedef CMapPool<CTcpContext*, CTcpClient>	POOL_CLIENT;

private:
	virtual CMutex*				ConnectLock()							{ return &(m_mapClient.getLock()); }				// m_mapClient���߳���
	virtual CTcpConnection*		ConnectFind(CTcpContext* pContext)		{ return m_mapClient.find(pContext); }				// ����һ���ͻ�������
	virtual void				ConnectDel(CTcpContext* pContext)		{ if (pContext) m_mapClient.release(pContext); }	// ɾ��һ���ͻ�������
	virtual CTcpConnection*		ConnectNew(CTcpContext* pContext);

public:
	inline CTcpClient* getClient(CTcpContext*_context, uint32 _key)
	{
		CCritLocker clLock(m_mapClient.getLock());
		CTcpClient* pClient = m_mapClient.find(_context);
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
