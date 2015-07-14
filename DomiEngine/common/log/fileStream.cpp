/******************************************************************** 
����ʱ��:        2015/07/07 22:09
�ļ�����:        fileStream.cpp
�ļ�����:        Domi
*********************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include "common/log/fileStream.h"

// �ļ���������
const char	CFileStream::m_gszAccess[Access_Max][5] =
{
	"rb",	// ���ļ�
	"wb",	// д�ļ�
	"rb+",	// ��д�ļ�
	"ab+",	// ׷��
};

CFileStream::CFileStream()
{
	m_pFile	= nullptr;
}

CFileStream::~CFileStream()
{
	close();
}

// ���ļ�
bool CFileStream::open(const char*pszFileName,_enAccess eAccess)
{
	if(eAccess >= Access_Max)
		eAccess = _Read;

	return open(pszFileName,m_gszAccess[eAccess]);
}

// ���ļ�
bool CFileStream::open(const char*pszFileName,const char* pszMode)
{
	if(!pszFileName)
		return false;

	close();

#ifdef WIN32
	::fopen_s(&m_pFile,pszFileName,pszMode ? pszMode : "rb");
#else
	m_pFile = ::fopen(pszFileName,pszMode ? pszMode : "rb");
#endif

	return (m_pFile != nullptr);
}

//�ر�
void CFileStream::close()
{	
	if(m_pFile)
		::fclose(m_pFile);

	m_pFile = nullptr;
}

//���»�����
void CFileStream::flush()
{
	//ȷ��д�뵽�ļ�-��һ���Ǵ��̵��Ǳ𴦿��Զ�ȡ
	if(m_pFile)
		fflush(m_pFile);
}

// ����ļ�����
uint32	CFileStream::getFileLength()
{
	if(!m_pFile)
		return 0;

	int32 nTell = ftell(m_pFile);
	if(seek(0,SEEK_END) != 0)
	{
		seek(nTell,SEEK_SET);
		return 0;
	}

	uint32 uLength = position();
	seek(nTell,SEEK_SET);

	return uLength;
}

//��õ�ǰ�ļ�ָ��λ��
uint32	CFileStream::position()
{
	if(!m_pFile)
		return 0;

	return ::ftell(m_pFile);
}

//����ļ�
int	CFileStream::clear()
{
	if(!m_pFile)
		return 0;

	return ::fflush(m_pFile);
}

//�Ƿ��ѵ��ļ�β
bool CFileStream::eof()
{
	if(!m_pFile)
		return true;

	return (::feof(m_pFile) != 0);
}

/*
**ƫ��
**|SEEK_SET | SEEK_CUR		| SEEK_END	|
**|�ļ���ͷ | ��ǰ��д��λ��| �ļ�β	|
*/
int	CFileStream::seek(int32 _n8Offset, int _Origin)
{
	if(!m_pFile)
		return 0;

	return ::fseek(m_pFile,_n8Offset,_Origin);
}


//��ȡ
bool CFileStream::_read(uint32 uBytes,void* outBuffer)
{
	if(!m_pFile || !uBytes || !outBuffer)
		return 0;

	return (::fread(outBuffer,1,uBytes,m_pFile) == uBytes);
}

//д��

bool CFileStream::_write(uint32 uBytes,const void*inBuffer)
{
	if(!m_pFile || !uBytes || !inBuffer)
		return 0;

	return (::fwrite(inBuffer,1,uBytes,m_pFile) == uBytes);
}

// ��ȡ��uMaxBytes���ֽ�
uint32 CFileStream::fread(uint32 uMaxBytes,void* outBuffer)
{
	if(!m_pFile || !uMaxBytes || !outBuffer)
		return 0;

	return ::fread(outBuffer,1,uMaxBytes,m_pFile);
}

//
uint32	CFileStream::fwrite(uint32 uMaxBytes,const void*inBuffer)
{
	if(!m_pFile || !uMaxBytes || !inBuffer)
		return 0;

	return ::fwrite(inBuffer,1,uMaxBytes,m_pFile);
}

uint32	CFileStream::fprintf(const char* pszFormat,...)
{
	if(!m_pFile || !pszFormat)
		return 0;

	va_list argptr;
	va_start(argptr, pszFormat);
#ifdef WIN32
	uint32 uLen = ::vfprintf_s(m_pFile,pszFormat,argptr);
#else//WIN32
	uint32 uLen = ::vfprintf(m_pFile,pszFormat,argptr);
#endif//WIN32
	va_end(argptr);

	return uLen;
}
