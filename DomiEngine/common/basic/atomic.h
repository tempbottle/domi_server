/******************************************************************** 
����ʱ��:        2015/07/09 16:07
�ļ�����:        atomic.h
�ļ�����:        Domi
����˵��:        ԭ�Ӳ�����װ  
����˵��:         
*********************************************************************/

#pragma once

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#include<pthread.h>
#include<semaphore.h>
#endif

template<class T>
class CAtomicInteger
{
private:
	volatile T value;
public:
	CAtomicInteger()
		:value(0)
	{
	}

	~CAtomicInteger() {}

public:
	// ԭ�Ӳ���
	T AtomicAdd(T t);
	T AtomicInc();
	T AtomicDec();

	inline T IncrementAndGet()	{ return AtomicInc(); }
	inline T GetValue()			{ return value; }
};

//-------------------------------------------------------------------
// ԭ�Ӳ����ӣ����tΪ��ֵ������ԭ�Ӳ�����
template < class T >
T CAtomicInteger<T>::AtomicAdd( T t)
{
#ifdef WIN32
	return (T)InterlockedExchangeAdd((T*)&value, (T)t);
#else
	//	return __gnu_cxx::__exchange_and_add(pt, t);
	return __sync_add_and_fetch(&value, t);
	//return *++pt;
#endif
}

// ԭ�Ӳ��� ++
template < class T >
T CAtomicInteger<T>::AtomicInc()
{
#ifdef WIN32
	return (T)InterlockedIncrement((T*)&value);
#else
	//return __gnu_cxx::__exchange_and_add(pt, 1);
	return __sync_add_and_fetch(&value, 1);
#endif
}

// ԭ�Ӳ��� --
template < class T >
T CAtomicInteger<T>::AtomicDec()
{
#ifdef WIN32
	return (T)InterlockedDecrement((T*)&value);
#else
	//return __gnu_cxx::__exchange_and_add(pt, -1);
	return __sync_add_and_fetch(&value, -1);
#endif
}