/******************************************************************** 
����ʱ��:        2015/06/22 14:50
�ļ�����:        singleton.h
�ļ�����:        Domi
����˵��:        ����ģ��  
����˵��:         
*********************************************************************/

#pragma once
#include <iostream>
using namespace std;
//#include "system/basic/basicTypes.h"

// ��һ��ʵ��

template <typename T>
class Singleton
{
protected:
	Singleton(){}
	virtual~Singleton(){}

public:
	inline static T* GetSingletonPtr()	
	{
		static T instance;
		return &instance;;	
	}

	inline static T& GetSingleton()
	{
		return *(Singleton::GetSingletonPtr());
	}
};


/*
// �ڶ���ʵ��
template <typename T>
class Singleton
{
private:
	static T* m_pInstance;

	// GC
	//����Ψһ��������������������ɾ��CSingleton��ʵ�� 
	template<class M>
	class CGarbo 
	{
	public:
		CGarbo() { }
		~CGarbo()
		{
			if (ptr)
				delete ptr;
		}

		M* ptr;
	};

protected:
	Singleton(){}
	virtual ~Singleton(){}

public:
	inline static T* GetSingletonPtr()
	{
		if (m_pInstance == nullptr)  //�ж��Ƿ��һ�ε���  
		{
			static CGarbo<T> garbo;
			garbo.ptr = new T();
			m_pInstance = garbo.ptr;
		}

		return m_pInstance;
	}

	inline static T& GetSingleton()
	{
		return *(Singleton::GetSingletonPtr());
	}
};

//template<class T>
//typename Singleton<T>::CGarbo Singleton<T>::Garbo;

template<class T> 
T* Singleton<T>::m_pInstance = nullptr;
*/