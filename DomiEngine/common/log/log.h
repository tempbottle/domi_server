/******************************************************************** 
����ʱ��:        2015/06/28 14:36
�ļ�����:        log.h
�ļ�����:        Domi
����˵��:        log��־  
����˵��:        �����ļ���д����־
*********************************************************************/

#pragma once
#include <iostream>
#include <ctime>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "common/basic/basicTypes.h"
#include "common/basic/timeFunction.h"
#include "common/log/fileStream.h"
#include "common/thread/csLocker.h"
#include "common/log/consoleColor.h"

class CLog
{
private:
	enum _e_LogType
	{
		LT_Log,		// log
		LT_Warn,	// ����
		LT_Error,	// ����
		LT_Max
	};

private:
	static bool				m_bInitiate;			// �Ƿ��Ѿ���ʼ��
	static SYSTEMTIME		m_gSysTime[LT_Max];		// ����ʱ��
	static char				m_szDirectory[256];		// ·��
	static char				m_szBuffer[2048];		// ����
	static CMutex			m_csLock;				// ������
	static CFileStream		m_clgFile[LT_Max];		// Ŀ���ļ� 
    static char             m_logFilePrev[ 256 ];	// log�ļ�ǰ׺

private:
	static void	createAFile	(uint8 ucType);
	static void	getDate		(char* pDate,const uint32& uSize);
	static void	setDirectory(const char* pDir);
	static void	_print		(const char* pFormat,va_list&argptr);

public:
	static bool	initialize(const char* root = nullptr, const char* prev = nullptr);		// ��ʼ��
	static void	shutdown();	// �ر�

	static void	print(const char* pFormat,...);
	static void info(const char* pFormat, ...);
	static void	warn(const char* pFormat,...);
	static void	error(const char* pFormat,...);
	static void	lastError(const char* pFormat,...);
};
