#pragma once
#include "common/basic/basicTypes.h"
#include <time.h>
#ifdef WIN32	//WIN32
#include <winsock2.h>//未放置网络定义冲突
#include <windows.h>
#else	//linux
#include <sys/time.h>
#endif // WIN32
/*************************************************************/
//--- 获得系统启动毫秒
extern uint32	getTickCount	();
//--- 获得系统启动毫秒(64位)
extern uint64	getTickCount64	();
//--- 获得系统启动毫秒(64位)【需要手动更新】
extern uint64	getSystemTick64	();
//--- 设置系统启动毫秒
extern uint64	setSystemTick64	();

//--- 获得系统秒时间
extern uint64	getSecond		();
//--- 获得系统毫秒时间
extern uint64	getMillisecond	();
//--- 获得系统微秒时间
extern uint64	getMicroseconds	();
//--- 获得时钟时间(毫秒)
extern uint64	getClockTime	();

//--- 设置系统起始时间，用于调试
extern void	    setStartTime    ( uint64 time );
//--- 获得系统时间
extern uint64	getTime			();
extern uint32	getTime32		();
//--- 获得系统时间
extern uint64	setSysTime		();
extern uint64	getSysTime		();
//--- 获得今日开始时间
extern uint64	getDayBeginTime	();

//--- 获得本地时间
extern int32	getLocalTime	(struct tm* _Tm,const uint64* _Time);
extern int32	getLocalTime	(struct tm* _Tm,uint64 _Time);

//--- 获得时间相差天数
extern int32	diffTimeDay		(uint32 _early,uint32 _late);
//--- 获得时间相差周数
extern int32	diffTimeWeek	(uint64 _early,uint64 _late);
//--- 获得时间相差月数
extern int32	diffTimeMonth	(uint64 _early,uint64 _late);
//--- 时间格式化(YYYY-MM-DD HH:MM:SS)
extern pc_str	time_format		(uint64 _time);
//--- 睡眠
extern void		dSleep			(uint32 millisecond);
//--- 天数是否到期
extern bool		isDayExpired	(uint64 _early, uint64 _late, uint16 _limit);
extern bool		isDayExpired	(uint16 _diff, uint16 _limit);

#ifndef WIN32
struct SYSTEMTIME
{
	uint16 wYear;
	uint16 wMonth;
	uint16 wDayOfWeek;
	uint16 wDay;
	uint16 wHour;
	uint16 wMinute;
	uint16 wSecond;
	uint16 wMilliseconds;
};
#endif // WIN32
extern void	getLocalTime(SYSTEMTIME&systime);
