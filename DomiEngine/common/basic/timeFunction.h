#pragma once
#include "common/basic/basicTypes.h"
#include <time.h>
#ifdef WIN32	//WIN32
#include <winsock2.h>//δ�������綨���ͻ
#include <windows.h>
#else	//linux
#include <sys/time.h>
#endif // WIN32
/*************************************************************/
//--- ���ϵͳ��������
extern uint32	getTickCount	();
//--- ���ϵͳ��������(64λ)
extern uint64	getTickCount64	();
//--- ���ϵͳ��������(64λ)����Ҫ�ֶ����¡�
extern uint64	getSystemTick64	();
//--- ����ϵͳ��������
extern uint64	setSystemTick64	();

//--- ���ϵͳ��ʱ��
extern uint64	getSecond		();
//--- ���ϵͳ����ʱ��
extern uint64	getMillisecond	();
//--- ���ϵͳ΢��ʱ��
extern uint64	getMicroseconds	();
//--- ���ʱ��ʱ��(����)
extern uint64	getClockTime	();

//--- ����ϵͳ��ʼʱ�䣬���ڵ���
extern void	    setStartTime    ( uint64 time );
//--- ���ϵͳʱ��
extern uint64	getTime			();
extern uint32	getTime32		();
//--- ���ϵͳʱ��
extern uint64	setSysTime		();
extern uint64	getSysTime		();
//--- ��ý��տ�ʼʱ��
extern uint64	getDayBeginTime	();

//--- ��ñ���ʱ��
extern int32	getLocalTime	(struct tm* _Tm,const uint64* _Time);
extern int32	getLocalTime	(struct tm* _Tm,uint64 _Time);

//--- ���ʱ���������
extern int32	diffTimeDay		(uint32 _early,uint32 _late);
//--- ���ʱ���������
extern int32	diffTimeWeek	(uint64 _early,uint64 _late);
//--- ���ʱ���������
extern int32	diffTimeMonth	(uint64 _early,uint64 _late);
//--- ʱ���ʽ��(YYYY-MM-DD HH:MM:SS)
extern pc_str	time_format		(uint64 _time);
//--- ˯��
extern void		dSleep			(uint32 millisecond);
//--- �����Ƿ���
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
