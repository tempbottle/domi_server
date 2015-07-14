/******************************************************************** 
����ʱ��:        2015/07/07 22:29
�ļ�����:        consoleColor.cpp
�ļ�����:        Domi
*********************************************************************/

/*----------------- consoleColor.cpp
*windows:
*ǰ��ɫ:FOREGROUND_BLUE����ɫ����FOREGROUND_GREEN����ɫ����FOREGROUND_RED����ɫ����FOREGROUND_INTENSITY��������
*����ɫ:BACKGROUND_BLUE����ɫ����BACKGROUND_GREEN����ɫ����BACKGROUND_RED����ɫ����BACKGROUND_INTENSITY��������
*
*
*--------------------------------------------------------------
*linux:\033[��ɫ;��ɫ;m
*��ʾ��ʽ:0��Ĭ��ֵ����1����������22���Ǵ��壩��4���»��ߣ���24�����»��ߣ���5����˸����25������˸����7�����ԣ���27���Ƿ��ԣ�
*ǰ��ɫ:30����ɫ����31����ɫ����32����ɫ���� 33����ɫ����34����ɫ����35����죩��36����ɫ����37����ɫ��
*����ɫ:40����ɫ����41����ɫ����42����ɫ���� 43����ɫ����44����ɫ����45����죩��46����ɫ����47����ɫ��
*------------------------------------------------------------*/

#include "common/log/consoleColor.h"
#include "common/basic/stringFunctions.h"

#ifdef WIN32
HANDLE	CConsoleColor::g_hConsole	= INVALID_HANDLE_VALUE;
WORD	CConsoleColor::g_fore_color[color_max]=
{
	FOREGROUND_RED						,//��
	FOREGROUND_GREEN					,//��
	FOREGROUND_RED | FOREGROUND_GREEN	,//��
	FOREGROUND_BLUE						,//��
	FOREGROUND_INTENSITY				,//����
};
WORD	CConsoleColor::g_back_color[color_max]=
{
	BACKGROUND_RED						,//��
	BACKGROUND_GREEN					,//��
	BACKGROUND_RED | BACKGROUND_GREEN	,//��
	BACKGROUND_BLUE						,//��
	BACKGROUND_INTENSITY				,//����
};
#else // WIN32
const char*	CConsoleColor::g_fore_color[CConsoleColor::color_max] =
{
	"31"	,//��
	"32"	,//��
	"33"	,//��
	"34"	,//��
	"1"		,//����
};
const char*	CConsoleColor::g_back_color[CConsoleColor::color_max] =
{
	"41"	,//��
	"42"	,//��
	"43"	,//��
	"44"	,//��
	"1"		,//����
};
#endif // WIN32

// ��ʼ��
bool CConsoleColor::initialize()
{
#ifdef WIN32
	if(g_hConsole == INVALID_HANDLE_VALUE || g_hConsole == nullptr)
	{
		g_hConsole= ::GetStdHandle(STD_OUTPUT_HANDLE);
		if(g_hConsole == nullptr || g_hConsole == INVALID_HANDLE_VALUE)
			return false;
	}
#endif // WIN32
	return true;
}

// �ر� 
void CConsoleColor::shutdown()
{
#ifdef WIN32
	if(g_hConsole != nullptr && g_hConsole != INVALID_HANDLE_VALUE)
		::CloseHandle(g_hConsole);

	g_hConsole = nullptr;
#endif // WIN32
}

CConsoleColor::CConsoleColor(uint8 _fore,uint8 _bank)
{
	initialize();
#ifdef WIN32
	m_wColorSave	= 0;
	if(g_hConsole != nullptr && g_hConsole != INVALID_HANDLE_VALUE)
	{
		CONSOLE_SCREEN_BUFFER_INFO stInfo;
		::GetConsoleScreenBufferInfo(g_hConsole, &stInfo);
		m_wColorSave = stInfo.wAttributes;
	}
#endif // WIN32

	setColor(_fore,_bank);
}
 
CConsoleColor::~CConsoleColor()
{
	//���
#ifdef WIN32
	if(g_hConsole != nullptr && g_hConsole != INVALID_HANDLE_VALUE)
		::SetConsoleTextAttribute(g_hConsole,m_wColorSave);
#else // WIN32
	dPrintf("\033[0m");
#endif // WIN32
}
 
void CConsoleColor::setColor(uint8 _fore,uint8 _bank)
{
#ifdef WIN32
	if(g_hConsole != nullptr && g_hConsole != INVALID_HANDLE_VALUE)
	{
		WORD wColor = 0;
		for (int i = 0;i < color_max;i++)
		{
			if(_fore && _CHECK_BIT(_fore,_BIT32(i)))
				wColor |= g_fore_color[i];

			if(!_fore && _CHECK_BIT(m_wColorSave,g_fore_color[i]))
				wColor |= g_fore_color[i];

			if(_bank && _CHECK_BIT(_bank,_BIT32(i)))
				wColor |= g_back_color[i];

			if(!_bank && _CHECK_BIT(m_wColorSave,g_back_color[i]))
				wColor |= g_back_color[i];
		}

		::SetConsoleTextAttribute(g_hConsole,wColor);
	}
#else // WIN32
	bool bFirst = true;
	for (int i = 0;i < color_max;i++)
	{
		if(_CHECK_BIT(_fore,_BIT32(i)))
		{
			if(bFirst)
			{
				bFirst	= false;
				dPrintf("\033[%s",g_fore_color[i]);
			}
			else
			{
				dPrintf(";%s",g_fore_color[i]);
			}
		}

		if(_CHECK_BIT(_bank,_BIT32(i)))
		{
			if(bFirst)
			{
				bFirst	= false;
				dPrintf("\033[%s",g_fore_color[i]);
			}
			else
			{
				dPrintf(";%s",g_fore_color[i]);
			}
		}
	}
	if(!bFirst)
		dPrintf("m");
#endif // WIN32
}