/******************************************************************** 
����ʱ��:        2015/07/07 22:05
�ļ�����:        fileStream.h
�ļ�����:        Domi
����˵��:        �ļ���  
����˵��:         
*********************************************************************/

#pragma once
#include <iostream>
#include <memory.h>
#include "common/basic/basicTypes.h"

class CFileStream
{
public:
	enum _enAccess
	{
		_Read		,/*��*/ 
		_Write		,/*д*/ 
		_ReadWrite	,/*��д*/ 
		_Append		,/*׷��*/ 
		Access_Max
	};

protected:
	static const char	m_gszAccess[Access_Max][5];

protected:
	FILE*	m_pFile;

public:
	inline bool		isOpen	()	{	return (m_pFile != nullptr);	}
	inline FILE*	getFile	()	{	return m_pFile;				}

public:
	CFileStream();
	virtual~CFileStream();

public:
	bool open(const char*pszFileName,_enAccess eAccess);			// ���ļ�
	bool open(const char*pszFileName, const char* pszMode = "rb");	// ���ļ�
	void close();													// �ر��ļ�
	void flush();													// ���»�����

public:
	uint32	getFileLength();							// ����ļ�����
	int	clear();										// ����ļ�
	int	seek(int32 _nOffset, int _Origin = SEEK_SET);	// ƫ��(�ɹ�����0)

public:
	inline int seekBegin(int32 _nOffset)	{return seek(_nOffset,SEEK_SET);}	// ���ļ�ͷƫ��
	inline int seekEnd(int32 _nOffset)		{return seek(_nOffset,SEEK_END);}	// ���ļ�βƫ��
	inline int seekCurrent(int32 _nOffset)	{return seek(_nOffset,SEEK_CUR);}	// ���ļ���ǰλ��ƫ�� 

public:
	virtual uint32	size()							{return getFileLength();	}
	virtual bool	skip(int32 _Bytes)				{ return (seekCurrent(_Bytes) == 0); }
	virtual bool	setpos(uint32 newPosition)		{ return (seekBegin(newPosition) == 0); }
	virtual uint32	position();
	virtual bool	eof();

protected:
	virtual bool	_read			(uint32 uBytes,void* outBuffer);
	virtual bool	_write			(uint32 uBytes,const void*inBuffer);

public:
	virtual uint32	fread			(uint32 uMaxBytes,void* outBuffer);
	virtual uint32	fwrite			(uint32 uMaxBytes,const void*inBuffer);
	virtual uint32	fprintf			(const char* pszFormat,...);
};
