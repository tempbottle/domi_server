/*--------------------------------------------------------------
*
Linux�ڱ��ʱʹ�������¼���ʱ�����ͣ�
1.typedef long time_t;      //<time.h>����õ�
2.struct tim //<sys/time.h>����С��λ��΢��
{
time_t      tv_sec;
suseconds_t tv_usec;
}
3.struct timespec //<time.h>����С��λ������
{
time_t tv_sec;
long   tv_nsec;
}
4.struct tm//<time.h>����������
{
int tm_sec;
int tm_min;
int tm_hour;
int tm_mday;
int tm_mon;
int tm_year;
int tm_wday;
int tm_yda;
int tm_isdst;
#ifdef _BSD_SOURCE
long tm_gmtoff;
xonst char *tm_zone;
#endif
}

*------------------------------------------------------------*/
#include "common/basic/timeFunction.h"
#include "common/basic/memoryFunctions.h"
#include "common/basic/stringFunctions.h"
#include <memory.h> 
#include <string.h>
#ifdef WIN32
#include <windows.h >
#else//WIN32
#include <unistd.h> 
#include <sys/times.h> 
#endif//WIN32
/*************************************************************/
static uint64	g_uSystemTick64 = 0;
static uint64	g_uSystemTime = 0;
static uint64	s_start_time = ::time( nullptr );
static uint64	s_init_time = ::time( nullptr );
//-------------------------------------------------------------
//------------------------------ ���ϵͳ��������
uint32	getTickCount	()
{
#ifdef WIN32
	//ֻ��ȷ��55ms
	return ::GetTickCount();
	//ֻ��ȷ��10ms
	//timeGetTime();
#else//WIN32
	tms tm;
	return ::times(&tm);
#endif // WIN32
}
//-------------------------------------------------------------
//------------------------------ ���ϵͳ��������
uint64	getTickCount64	()
{
#ifdef WIN32
	return ::GetTickCount64();
#else//WIN32
	/*struct timespec
	{
		time_t tv_sec; // seconds[��]
		long tv_nsec; // nanoseconds[����]
	};
	int clock_gettime(clockid_t clk_id, struct timespect *tp);
	//@clk_id:
	CLOCK_REALTIME:ϵͳʵʱʱ��,��ϵͳʵʱʱ��ı���ı�
	CLOCK_MONOTONIC:��ϵͳ������һ����ʼ��ʱ,����ϵͳʱ�䱻�û��ı��Ӱ��
	CLOCK_PROCESS_CPUTIME_ID:�����̵���ǰ����ϵͳCPU���ѵ�ʱ��
	CLOCK_THREAD_CPUTIME_ID:���̵߳���ǰ����ϵͳCPU���ѵ�ʱ��
	*/

	timespec _spec;
	clock_gettime(CLOCK_MONOTONIC,&_spec);
	uint64 uTick = _spec.tv_sec * 1000 + _spec.tv_nsec / 1000 / 1000;

	/*timeval _val;
	::gettimeofday(&_val,NULL);
	uint64 uTick = _val.tv_sec * 1000 + _val.tv_usec / 1000;*/
	return uTick;
#endif // WIN32
}
//-------------------------------------------------------------
//------------------------------ ���ϵͳ��������(64λ)����Ҫ�ֶ����¡�
uint64	getSystemTick64	()
{
	if(!g_uSystemTick64)
		setSystemTick64();

	return g_uSystemTick64;
}
//-------------------------------------------------------------
//------------------------------ ����ϵͳ��������
uint64	setSystemTick64	()
{
	g_uSystemTick64 = getTickCount64();

	return g_uSystemTick64;
}
//-------------------------------------------------------------
//------------------------------ ���ϵͳ��ʱ��
uint64	getSecond		()
{
	return ::time(nullptr);
}

//-------------------------------------------------------------
//------------------------------ ���ϵͳ����ʱ��
uint64	getMillisecond	()
{
#ifdef WIN32
	//ϵͳ�����Ժ�ĺ��뼶ʱ��
	//clock();
	//ֻ��ȷ��55ms
	return ::GetTickCount64();

#else//WIN32
	/*struct timespec
	{
		time_t tv_sec; // seconds[��]
		long tv_nsec; // nanoseconds[����]
	};
	int clock_gettime(clockid_t clk_id, struct timespect *tp);
	//@clk_id:
	CLOCK_REALTIME:ϵͳʵʱʱ��,��ϵͳʵʱʱ��ı���ı�
	CLOCK_MONOTONIC:��ϵͳ������һ����ʼ��ʱ,����ϵͳʱ�䱻�û��ı��Ӱ��
	CLOCK_PROCESS_CPUTIME_ID:�����̵���ǰ����ϵͳCPU���ѵ�ʱ��
	CLOCK_THREAD_CPUTIME_ID:���̵߳���ǰ����ϵͳCPU���ѵ�ʱ��
	*/

	timespec _spec;
	clock_gettime(CLOCK_MONOTONIC,&_spec);
	uint64 uTick = _spec.tv_sec * 1000 + _spec.tv_nsec / 1000 / 1000;

	/*timeval _val;
	::gettimeofday(&_val,NULL);
	uint64 uTick = _val.tv_sec * 1000 + _val.tv_usec / 1000;*/
	return uTick;
#endif // WIN32
}
//-------------------------------------------------------------
//------------------------------ ���ϵͳ΢��ʱ��
uint64	getMicroseconds	()
{
#ifdef WIN32
	/*LARGE_INTEGER stFrequency;
	QueryPerformanceFrequency(&stFrequency); */
	LARGE_INTEGER stCount;
	QueryPerformanceCounter(&stCount); 

	return uint64(stCount.QuadPart);
#else//WIN32
	/*struct timespec
	{
		time_t tv_sec; // seconds[��]
		long tv_nsec; // nanoseconds[����]
	};
	int clock_gettime(clockid_t clk_id, struct timespect *tp);
	//@clk_id:
	CLOCK_REALTIME:ϵͳʵʱʱ��,��ϵͳʵʱʱ��ı���ı�
	CLOCK_MONOTONIC:��ϵͳ������һ����ʼ��ʱ,����ϵͳʱ�䱻�û��ı��Ӱ��
	CLOCK_PROCESS_CPUTIME_ID:�����̵���ǰ����ϵͳCPU���ѵ�ʱ��
	CLOCK_THREAD_CPUTIME_ID:���̵߳���ǰ����ϵͳCPU���ѵ�ʱ��
	*/

	timespec _spec;
	clock_gettime(CLOCK_MONOTONIC,&_spec);
	uint64 uTick = _spec.tv_sec * 1000 * 1000 + _spec.tv_nsec / 1000;

	/*timeval _val;
	::gettimeofday(&_val,NULL);
	uint64 uTick = _val.tv_sec * 1000 * 1000 + _val.tv_usec;*/
	return uTick;
#endif // WIN32
}
//-------------------------------------------------------------
//------------------------------ ���ʱ��ʱ��(����)
uint64	getClockTime	()
{
#ifdef WIN32
	uint64 uTick = uint64(getTime()) * 1000;
#else//WIN32
	timespec _spec;
	clock_gettime(CLOCK_REALTIME,&_spec);

	uint64 uTick = _spec.tv_sec * 1000 + _spec.tv_nsec / 1000 / 1000;
#endif // WIN32
	return uTick;
}
//-------------------------------------------------------------
//------------------------------ ���ϵͳʱ��
uint64	getTime			()
{
    return s_start_time + ( time( nullptr ) - s_init_time );
}

uint32	getTime32		()
{
	return (uint32)getTime();
}

//-------------------------------------------------------------
//------------------------------ ����ϵͳ��ʼʱ�䣬���ڵ���
void	setStartTime    ( uint64 time )
{
    s_start_time = time;
    s_init_time = ::time( nullptr );
}
//-------------------------------------------------------------
//------------------------------ 
uint64	setSysTime		()
{
	static uint64 g_uTime = 0;
	if(!g_uTime)
	{
		/*
		tm tm_temp;
		tm_temp.tm_year	= 2010-1900;//��1900+
		tm_temp.tm_mon	= 0;		//��[0,11]
		tm_temp.tm_mday	= 1;		//��[1,31]
		tm_temp.tm_hour	= 0;		//ʱ[0,23]
		tm_temp.tm_min	= 0;		//��[0,59]
		tm_temp.tm_sec	= 0;		//��[0,59]
		g_uTime	= mktime(&tm_temp);
		*/
	}

	g_uSystemTime = getTime();

	if(g_uSystemTime > g_uTime)
		g_uSystemTime -=g_uTime;

	//����������ƶ�һ����ʼʱ��,������ʱ��Ƭ

	return g_uSystemTime;
}
//-------------------------------------------------------------
//------------------------------ ���ϵͳʱ��
uint64	getSysTime		()
{
	if(!g_uSystemTime)
		setSysTime();

	return g_uSystemTime;
}
//-------------------------------------------------------------
//------------------------------ ��ý��տ�ʼʱ��
uint64	getDayBeginTime	()
{
	static tm	_tm;
	memset(&_tm,0,sizeof(_tm));

	getLocalTime(&_tm,getTime());
	_tm.tm_hour= 0;
	_tm.tm_min = 0;
	_tm.tm_sec = 0;

	return mktime(&_tm);
}
//-------------------------------------------------------------
//------------------------------ ��ñ���ʱ��
int32	getLocalTime	(struct tm* _Tm,const uint64* _Time)
{
	if(!_Tm || !_Time)
		return -1;

	time_t _time = *_Time;
#ifdef WIN32
	return ::localtime_s(_Tm,&_time);
#else//WIN32
	if(_Tm)
		memset(_Tm,0xff,sizeof(struct tm));
	struct tm* tmp = ::localtime(&_time);
	if(tmp && _Tm)
		dMemcpy(_Tm,sizeof(struct tm),tmp,sizeof(struct tm));
	return 0;
#endif // WIN32
}
//-------------------------------------------------------------
//------------------------------ ��ñ���ʱ��
int32	getLocalTime	(struct tm* _Tm,uint64 _Time)
{
	if(!_Tm || !_Time)
		return -1;

	return getLocalTime(_Tm,&_Time);
}

//-------------------------------------------------------------
//------------------------------ ���ʱ���������
int32	diffTimeDay		(uint32 _early,uint32 _late)
{
	if(_early == 0 || _late == 0)
		return 0;

	static tm	tm_early;
	static tm	tm_late;
	memset(&tm_early,0,sizeof(tm_early));
	memset(&tm_late,0,sizeof(tm_late));

	getLocalTime(&tm_early,_early);
	getLocalTime(&tm_late,_late);

	if(tm_early.tm_year > tm_late.tm_year)
		return 0;

	//ͬ��ͬ��
	if(tm_early.tm_year == tm_late.tm_year && tm_early.tm_yday == tm_late.tm_yday)
		return 0;

	//ͬ���ж�
	if(tm_early.tm_year == tm_late.tm_year)
	{
		if(tm_early.tm_yday >= tm_late.tm_yday)
			return 0;

		return (tm_late.tm_yday - tm_early.tm_yday);
	}

	int32 iDay = 0;
	//��ͬ��ʱ
	if(tm_early.tm_year != tm_late.tm_year)
	{
		tm tm_temp = tm_early;

		//��ȡ12��31��ʱ��
		tm_temp.tm_mon	= 11;
		tm_temp.tm_mday = 31;
		tm_temp.tm_yday = 0;
		uint64 _temp	= mktime(&tm_temp);

		getLocalTime(&tm_temp,_temp);
		iDay = tm_temp.tm_yday - tm_early.tm_yday;

		iDay+=1;//����+1

		//������������
		for (int32 i = tm_early.tm_year + 1;i < tm_late.tm_year;i++)
		{
			tm_temp.tm_year++;
			tm_temp.tm_yday = 0;

			_temp	= mktime(&tm_temp);
			getLocalTime(&tm_temp,_temp);

			iDay += tm_temp.tm_yday;
			iDay+=1;//����+1
		}
	}

	return (iDay + tm_late.tm_yday);
}
//-------------------------------------------------------------
//------------------------------ ���ʱ���������
int32	diffTimeWeek	(uint64 _early,uint64 _late)
{
	if(_early == 0 || _late == 0)
		return 0;

	static tm	tm_early;
	static tm	tm_late;
	memset(&tm_early,0,sizeof(tm_early));
	memset(&tm_late,0,sizeof(tm_late));

	getLocalTime(&tm_early,_early);
	getLocalTime(&tm_late,_late);

	if(tm_early.tm_year > tm_late.tm_year)
		return 0;

	//ͬ��ͬ��
	if(tm_early.tm_year == tm_late.tm_year && tm_early.tm_yday == tm_late.tm_yday)
		return 0;

	//�����������ڵ�ÿһ��������������
	if(tm_early.tm_wday == 0)
		tm_early.tm_wday = 7;
	if(tm_late.tm_wday == 0)
		tm_late.tm_wday = 7;

	if(tm_early.tm_wday != 7)
		tm_early.tm_mday += (7 - tm_early.tm_wday);
	if(tm_late.tm_wday != 7)
		tm_late.tm_mday += (7 - tm_late.tm_wday);

	int32 iDay = diffTimeDay((uint32)mktime(&tm_early),(uint32)mktime(&tm_late));

	int32 iWeek = 0;
	if(iDay > 0)
		iWeek = iDay / 7;//�϶�����7�ı�����Ϊ��������

	return iWeek;
}
//-------------------------------------------------------------
//------------------------------ ���ʱ���������
int32	diffTimeMonth	(uint64 _early,uint64 _late)
{
	if(_early == 0 || _late == 0)
		return 0;

	static tm	tm_early;
	static tm	tm_late;
	memset(&tm_early,0,sizeof(tm_early));
	memset(&tm_late,0,sizeof(tm_late));

	getLocalTime(&tm_early,_early);
	getLocalTime(&tm_late,_late);

	if(tm_early.tm_year > tm_late.tm_year)
		return 0;

	//ͬ��ͬ��
	if(tm_early.tm_year == tm_late.tm_year && tm_early.tm_mon == tm_late.tm_mon)
		return 0;

	//ͬ���ж�
	if(tm_early.tm_year == tm_late.tm_year)
		return (tm_late.tm_mon - tm_early.tm_mon);

	int32 iMon = 0;
	//��ͬ��ʱ
	if(tm_early.tm_year != tm_late.tm_year)
	{
		//�����������
		iMon = (tm_late.tm_year - tm_early.tm_year) * 12;
		//�ټ����������
		iMon += tm_late.tm_mon;
		if(iMon >= tm_early.tm_mon)
			iMon -= tm_early.tm_mon;
		else
			iMon = 0;
	}

	return iMon;
}

//-------------------------------------------------------------
//------------------------------ ʱ���ʽ��(YYYY-MM-DD HH:MM:SS)
pc_str	time_format		(uint64 _time)
{
	tm	tmTime;
	memset(&tmTime,0,sizeof(tmTime));
	if(getLocalTime(&tmTime,_time) != 0)
		return "";

	static char szDate[32] = {0};
	dSprintf(szDate,sizeof(szDate),"%.4d-%.2d-%.2d %.2d:%.2d:%.2d",tmTime.tm_year + 1900,tmTime.tm_mon + 1,tmTime.tm_mday,tmTime.tm_hour,tmTime.tm_min,tmTime.tm_sec);

	return szDate;
}

//-------------------------------------------------------------
//------------------------------ ˯��
void	dSleep	(uint32 millisecond)
{
#ifdef WIN32
	::Sleep(millisecond);
#else//WIN32
	usleep( millisecond * 1000);
#endif // WIN32
}
//-------------------------------------------------------------
//------------------------------ 
void	getLocalTime(SYSTEMTIME&systime)
{
#ifdef WIN32
	::GetLocalTime(&systime);
#else//WIN32
	dMemset(&systime,0,sizeof(systime));
	time_t _time = getTime();
	struct tm* tmp = ::localtime(&_time);
	if(tmp)
	{
		timespec _spec;
		clock_gettime(CLOCK_REALTIME,&_spec);

		systime.wYear			= tmp->tm_year + 1900;
		systime.wMonth			= tmp->tm_mon + 1;
		systime.wDayOfWeek		= tmp->tm_wday;
		systime.wDay			= tmp->tm_mday;
		systime.wHour			= tmp->tm_hour;
		systime.wMinute			= tmp->tm_min;
		systime.wSecond			= tmp->tm_sec;
		systime.wMilliseconds	= _spec.tv_nsec / 1000 / 1000;
	}
#endif // WIN32
}

//-------------------------------------------------------------
//------------------------------ �����Ƿ���
bool isDayExpired(uint64 _early, uint64 _late, uint16 _limit)
{
	int32 nDiffDay = (int32)diffTimeDay(int32(_early), _late);

	return nDiffDay >= _limit;
}

bool isDayExpired(uint16 _diff, uint16 _limit)
{
	return _diff >= _limit;
}