/******************************************************************** 
����ʱ��:        2015/07/01 12:02
�ļ�����:        CQueuePool.h
�ļ�����:        Domi
����˵��:        ���г�  
����˵��:        ��ѭ������
*********************************************************************/

#pragma once
#include <queue>
#include "common/thread/csLocker.h"

template<class _Ty>
class CQueuePool
{
protected:
	typedef std::queue<_Ty*> QUEUE;
	enum{ _block_max = 10000 };	//ÿ����������ߴ�

public:
	CQueuePool();
	virtual~CQueuePool();

protected:
	inline void	_build();
	inline _Ty*	_malloc();
	inline void	_free(_Ty*_val);

public:
	inline bool		empty			()const		{	return m_use_queue.empty();	}
	inline size_t	size			()const		{	return m_use_queue.size();	}
	inline void		set_block_size	(int _size)	{	m_block_size = _size;		}
	inline CMutex&	get_mutex		()			{	return m_mutex;				}

	inline void	destroy();
	inline void	clear();
	inline void	pop();
	inline _Ty*	push();
	inline _Ty*	front();

public:
	CMutex	m_mutex;		//�̰߳�ȫ������
	int		m_block_size;	//ÿ��������С(Ĭ��1,���10000)
	QUEUE	m_use_queue;	//ʹ�ö���
	QUEUE	m_free_queue;	//�ͷŶ���
};

//-----------------------------------------------------------------------
template<class _Ty>
CQueuePool<_Ty>::CQueuePool()
{
	m_block_size = 1;
}

template<class _Ty>
CQueuePool<_Ty>::~CQueuePool()
{
	destroy();
}

template<class _Ty>
inline void	CQueuePool<_Ty>::_build()
{
	//�����ߴ�
	if(m_block_size < 1)
		m_block_size = 1;

	if(m_block_size > _block_max)
		m_block_size = _block_max;

	try
	{
		//��������ڴ�
		_Ty*_val = nullptr;
		for (int i = 0;i < m_block_size;i++)
		{
			_val = new _Ty;
			if(_val)
				m_free_queue.push(_val);
		}
	}
	catch (...)
	{
		//printf("%s Crash!\n", "CQueuePool");
	}
}

// ����ռ�
// �����free�ռ䣬���free�ռ�ȡ������֮������new
template<class _Ty>
inline _Ty*	CQueuePool<_Ty>::_malloc()
{
	_Ty*_val = nullptr;
	try
	{
		if(m_free_queue.empty())
			_build();

		if(!m_free_queue.empty())
		{
			_val = m_free_queue.front();
			m_free_queue.pop();
		}
	}
	catch (...)
	{
	}

	return _val;
}

// �ͷſռ䣬ʹ֮��Ϊ����
template<class _Ty>
inline void	CQueuePool<_Ty>::_free(_Ty*_val)
{
	try
	{
		if(_val)
			m_free_queue.push(_val);
	}
	catch (...)
	{
	}
}

// ���ٶ��гأ�����Ѿ�ʹ�õĺͿ��е�
template<class _Ty>
inline void	CQueuePool<_Ty>::destroy()
{
	try
	{
		_Ty*_val = nullptr;
		//����ʹ�ö���
		while(!m_use_queue.empty())
		{
			_val = m_use_queue.front();
			m_use_queue.pop();
			if(_val)
				delete _val;
		}

		//�����ͷŶ���
		while(!m_free_queue.empty())
		{
			_val = m_free_queue.front();
			m_free_queue.pop();
			if(_val)
				delete _val;
		}
	}
	catch (...)
	{
	}
}

//����ʹ�ö���
template<class _Ty>
inline void	CQueuePool<_Ty>::clear()
{
	try
	{
		while(!m_use_queue.empty())
		{
			_free(m_use_queue.front());
			m_use_queue.pop();
		}
	}
	catch (...)
	{
	}
}

// ��ʹ�ö�����pop��һ��Ԫ��
// ע�⣺һ��Ҫ��pop֮ǰʹ����������ݣ���������Ұָ��
template<class _Ty>
inline void	CQueuePool<_Ty>::pop()
{
	try
	{
		if(!m_use_queue.empty())
		{
			_free(m_use_queue.front());
			m_use_queue.pop();
		}
	}
	catch (...)
	{
	}
}

// ѹ��һ��Ԫ��
template<class _Ty>
inline _Ty*	CQueuePool<_Ty>::push()
{
	_Ty*_val = nullptr;
	try
	{
		_val = _malloc();
		if(_val)
			m_use_queue.push(_val);
	}
	catch (...)
	{
	}
	return _val;
}
 
// ���ض���ͷ��Ԫ��
// һ���ȵ���front()��ȡ��Ԫ�ز����ĳЩ��������pop()
template<class _Ty>
inline _Ty*	CQueuePool<_Ty>::front()
{
	_Ty*_val = nullptr;
	try
	{
		if(!m_use_queue.empty())
			_val = m_use_queue.front();
	}
	catch (...)
	{
	}

	return _val;
}
