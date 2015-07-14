/******************************************************************** 
创建时间:        2015/07/07 22:27
文件名称:        consoleColor.h
文件作者:        Domi
功能说明:        控制台文本颜色  
其他说明:         
*********************************************************************/

#pragma once
#include "common/basic/basicTypes.h"
#ifdef WIN32	// WIN32
#include <windows.h>
#endif 

class CConsoleColor
{
public:
	enum _emColor
	{
		color_read		,//红
		color_green		,//绿
		color_yellow	,//黄
		color_blue		,//蓝
		color_intensity	,//高亮
		color_max
	};
private:
#ifdef WIN32
	static HANDLE		g_hConsole;
	static WORD			g_fore_color[color_max];
	static WORD			g_back_color[color_max];
#else // WIN32
	static const char*	g_fore_color[color_max];
	static const char*	g_back_color[color_max];
#endif // WIN32

private:
#ifdef WIN32
	WORD	m_wColorSave;
#endif // WIN32

public:
	CConsoleColor(uint8 _fore,uint8 _bank = 0);
	virtual~CConsoleColor();

public:
	static bool	initialize();
	static void	shutdown();

	void setColor(uint8 _fore,uint8 _bank = 0);
};