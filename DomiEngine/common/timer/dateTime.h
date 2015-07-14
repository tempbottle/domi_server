
#pragma once
#include "system/basic/basicTypes.h"
#include "system/basic/basicFunctions.h"
#include "system/basic/stringFunctions.h"
#include "system/basic/timeFunction.h"
#include "system/timer/dateTime.h"
#include "system/stl/stl_vector.h"
/*************************************************************/
#pragma pack(push,1)
//-------------------------------------------------------------
//------------------------------ 仅仅是为了兼容编辑器格式
struct _stDateTime_Old
{
//	uint8	uMark;			//标示
	int16	iYearBegin;		//年[标准年]
	int16	iYearEnd;		//年[标准年]
	int8	iMonthBegin;	//月[1,12]
	int8	iMonthEnd;		//月[1,12]
	uint8	uWeek;			//周[0,6]位
	uint32	uDay;			//日[1,31]位
	int8	iDayBegin;		//日[1,31]
	int8	iDayEnd;
	int8	iHourBegin;		//时[0,23]
	int8	iHourEnd;
	int8	iMinuteBegin;	//分[0,59]
	int8	iMinuteEnd;

	_stDateTime_Old()				{	initDateTime();	}
	_stDateTime_Old&getDateTime	()	{	return *this;	}
	inline void	initDateTime()
	{
//		uMark		= 0;
		iYearBegin	=-1;
		iYearEnd	=-1;
		iMonthBegin	=-1;
		iMonthEnd	=-1;
		uWeek		= 0;
		uDay		= 0;
		iDayBegin	=-1;
		iDayEnd		=-1;
		iHourBegin	=-1;
		iHourEnd	=-1;
		iMinuteBegin=-1;
		iMinuteEnd	=-1;
	}
};

//-------------------------------------------------------------
//------------------------------ 日期时间[字段小于0不检测]
struct _stDateTime
{//[*/年/年-年][*/月/月-月][[*/日-日]/[w周,w周]/[w周-w周]/[w周-w周,w周,w周]/[d日,d日]/[d日-d日]/[d日-d日,d日,d日]][时间-时间]
	enum
	{
		Week_Part 	,		//周内连续时间，大写W

		Mark_Max
	};

	uint8	uMark;			//标示
	int16	iYearBegin;		//年[标准年]
	int16	iYearEnd;		//年[标准年]
	int8	iMonthBegin;	//月[1,12]
	int8	iMonthEnd;		//月[1,12]
	uint8	uWeek;			//周[0,6]位
	uint32	uDay;			//日[1,31]位
	int8	iDayBegin;		//日[1,31]
	int8	iDayEnd;
	int8	iHourBegin;		//时[0,23]
	int8	iHourEnd;
	int8	iMinuteBegin;	//分[0,59]
	int8	iMinuteEnd;

	_stDateTime()				{	initDateTime();	}
	_stDateTime&getDateTime	()	{	return *this;	}
	inline void	initDateTime()
	{
		uMark		= 0;
		iYearBegin	=-1;
		iYearEnd	=-1;
		iMonthBegin	=-1;
		iMonthEnd	=-1;
		uWeek		= 0;
		uDay		= 0;
		iDayBegin	=-1;
		iDayEnd		=-1;
		iHourBegin	=-1;
		iHourEnd	=-1;
		iMinuteBegin=-1;
		iMinuteEnd	=-1;
	}

	inline bool isInitialTime() const
	{
		return (
			uMark			== 0	&&
			iYearBegin		== -1	&& 
			iYearEnd		== -1	&&
			iMonthBegin		== -1	&&
			iMonthEnd		== -1	&&
			uWeek			== 0	&&
			uDay			== 0	&&
			iDayBegin		== -1	&&
			iDayEnd			== -1	&&
			iHourBegin		== -1	&&
			iHourEnd		== -1	&&
			iMinuteBegin	== -1	&&
			iMinuteEnd		== -1	
			);
	}

	inline bool	isWeek		(const int&week) const
	{
		if(uWeek == 0)
			return true;

		return _CHECK_BIT(uWeek,_BIT32(week));
	}

	// 是否连续的周
	inline bool checkWeekSeries(uint8& uBeginWDay, uint8& uEndWDay) const
	{
		uBeginWDay	= 0;
		uEndWDay	= 0;

		if(uWeek == 0)
			return false;

		//周日颠倒一下，符合中国人习惯
		uint8 uTempWeek = uWeek;
		if(_CHECK_BIT(uTempWeek, _BIT32(0)))
			_SET_BIT32(uTempWeek, 7, true);

		//需要连续的日期才能处理
		bool bBegin		= false;
		bool bEnd		= false;

		for(uint8 i = 1; i < 8; i++)
		{
			if(_CHECK_BIT(uTempWeek, _BIT32(i)))
			{
				if(!bBegin)
				{
					bBegin		= true;
					uBeginWDay	= i;
					continue;
				}

				if(bEnd)
				{
					return false;
				}
			}
			else
			{
				if(bBegin && !bEnd)
				{
					bEnd		= true;
					uEndWDay	= i - 1;
				}
			}
		}

		return _CHECK_BIT(uMark, _BIT32(Week_Part));
	}

	// 得到周的开始时间
	uint64 getWeekBeginTime(const uint64& uTime, uint64& uBeginTime) const
	{
		uBeginTime = uTime;

		if(uWeek > 0)
		{
			uint8	uBeginWDay	= 0;
			uint8	uEndWDay	= 0;
			uint64	uCurrTime	= uTime;

			if(checkWeekSeries(uBeginWDay, uEndWDay))
			{
				tm tmTime;
				getLocalTime(&tmTime, uCurrTime);

				if(isWeek(tmTime.tm_wday))
				{
					//周日颠倒一下，符合中国人习惯
					if(tmTime.tm_wday == 0)
						tmTime.tm_wday = 7;

					//找出时间段的开始时间
					if(tmTime.tm_wday > uBeginWDay)
						uCurrTime -= (tmTime.tm_wday - uBeginWDay) * 86400;

					//得到开始时间
					getLocalTime(&tmTime, uCurrTime);

					//开始时间
					if(iYearBegin > 1900)
						tmTime.tm_year = iYearBegin - 1900;
					//月
					if(iMonthBegin > 0)
						tmTime.tm_mon = iMonthBegin - 1;
					//日
					if(iDayBegin > 0)
						tmTime.tm_mday = iDayBegin;
					//时
					if(iHourBegin >= 0)
						tmTime.tm_hour = iHourBegin;
					else
						tmTime.tm_hour = 0;
					//分
					if(iMinuteBegin >= 0)
						tmTime.tm_min = iMinuteBegin;
					else
						tmTime.tm_min = 0;

					uBeginTime = mktime(&tmTime);
					return true;
				}
			}
		}

		return false;
	}

	// 得到周的结束时间
	uint64 getWeekEndTime(const uint64& uTime, uint64& uEndTime) const
	{
		uEndTime = uTime;

		if(uWeek > 0)
		{
			uint8	uBeginWDay	= 0;
			uint8	uEndWDay	= 0;
			uint64	uCurrTime	= uTime;

			if(checkWeekSeries(uBeginWDay, uEndWDay))
			{
				tm tmTime;
				getLocalTime(&tmTime, uCurrTime);

				if(isWeek(tmTime.tm_wday))
				{
					//周日颠倒一下，符合中国人习惯
					if(tmTime.tm_wday == 0)
						tmTime.tm_wday = 7;

					//找出时间段的结束时间
					if(tmTime.tm_wday < uEndWDay)
						uCurrTime += (uEndWDay - tmTime.tm_wday) * 86400;

					//得到结束时间
					getLocalTime(&tmTime, uCurrTime);

					//结束时间
					if(iYearEnd > 1900)
						tmTime.tm_year = iYearEnd - 1900;
					//月
					if(iMonthEnd > 0)
						tmTime.tm_mon = iMonthEnd - 1;
					//日
					if(iDayEnd > 0)
						tmTime.tm_mday = iDayEnd;
					//时
					if(iHourEnd >= 0)
						tmTime.tm_hour = iHourEnd;
					else
						tmTime.tm_hour = 0;
					//分
					if(iMinuteEnd >= 0)
						tmTime.tm_min = iMinuteEnd;
					else
						tmTime.tm_min = 0;

					uEndTime = mktime(&tmTime);
					return true;
				}
			}
		}

		return false;
	}

    inline bool isDay		(const int&_Day) const
    {
        if(uDay == 0)
            return true;

        return _CHECK_BIT(uDay,_BIT32(_Day));
    }

	//--- 获得开始时间
	inline uint64	getBeginTime(const uint64& uTime) const
	{
		if(isInitialTime())
			return uTime;

		uint64 uWeekBeginTime = 0;
		if(getWeekBeginTime(uTime, uWeekBeginTime))
			return uWeekBeginTime;

        tm	tmTime;
		getLocalTime(&tmTime,&uTime);
        tmTime.tm_sec = 0;

		//周判断
		if (!isWeek(tmTime.tm_wday))
			return uTime;

 		//日判断
 		if(!isDay(tmTime.tm_mday))
 			return uTime;

		//开始时间
		if(iYearBegin > 1900)
			tmTime.tm_year = iYearBegin - 1900;
		//月
		if(iMonthBegin > 0)
			tmTime.tm_mon = iMonthBegin - 1;
		//日
		if(iDayBegin > 0)
			tmTime.tm_mday = iDayBegin;
		//时
		if(iHourBegin >= 0)
			tmTime.tm_hour = iHourBegin;
		else
			tmTime.tm_hour = 0;
		//分
		if(iMinuteBegin >= 0)
			tmTime.tm_min = iMinuteBegin;
		else
			tmTime.tm_min = 0;

		//开始时间是否到达
		return mktime(&tmTime);
	}

    //--- 获得结束时间
    inline uint64	getEndTime(const uint64& uTime) const
    {
		if ( isInitialTime() )
			return -1;

		uint64 uWeekEndTime = 0;
		if(getWeekEndTime(uTime, uWeekEndTime))
			return uWeekEndTime;

        tm	tmTime;
        getLocalTime(&tmTime,&uTime);

        //周判断
        if (!isWeek(tmTime.tm_wday))
            return uTime;

 		//日判断
 		if(!isDay(tmTime.tm_mday))
 			return uTime;

        //开始时间
        if(iYearEnd > 1900)
            tmTime.tm_year = iYearEnd - 1900;
        //月
        if(iMonthEnd > 0)
            tmTime.tm_mon = iMonthEnd - 1;
        //日
        if(iDayEnd > 0)
            tmTime.tm_mday = iDayEnd;
        //时
        if(iHourEnd >= 0)
            tmTime.tm_hour = iHourEnd;
        else
            tmTime.tm_hour = 0;
        //分
        if(iMinuteEnd >= 0)
            tmTime.tm_min = iMinuteEnd;
        else
            tmTime.tm_min = 0;

        //开始时间是否到达
        return mktime(&tmTime);
    }

	//--- 获得开始时间(只能获得标准时间类型)
	inline uint64	getBeginTime() const
	{
		if ( isInitialTime() )
			return -1;

		uint64 uWeekBeginTime = 0;
		if(getWeekBeginTime(getTime(), uWeekBeginTime))
			return uWeekBeginTime;

		if(iYearBegin <= 1900 || iMonthBegin <= 0|| iDayBegin <= 0 || iHourBegin < 0 || iMinuteBegin < 0)
			return 0;

		static tm	tmTime;
		memset(&tmTime,0,sizeof(tmTime));
		tmTime.tm_year	= iYearBegin - 1900;
		tmTime.tm_mon	= iMonthBegin - 1;
		tmTime.tm_mday	= iDayBegin;
		tmTime.tm_hour	= iHourBegin;
		tmTime.tm_min	= iMinuteBegin;

		return mktime(&tmTime);
	}

	//--- 获得结束时间(只能获得标准时间类型)
	inline uint64	getEndTime() const
	{
		if ( isInitialTime() )
			return -1;

		uint64 uWeekEndTime = 0;
		if(getWeekEndTime(getTime(), uWeekEndTime))
			return uWeekEndTime;

		if(iYearBegin <= 0 || iMonthBegin <= 0|| iDayEnd <= 0 || iHourEnd < 0 || iMinuteEnd < 0)
			return 0;

		static tm	tmTime;
		memset(&tmTime,0,sizeof(tmTime));
		tmTime.tm_year	= (iYearEnd > 1900) ? iYearEnd - 1900: (iYearBegin - 1900);
		tmTime.tm_mon	= (iMonthEnd > 0) ? (iMonthEnd - 1) : (iMonthBegin - 1);
		tmTime.tm_mday	= iDayEnd;
		tmTime.tm_hour	= iHourEnd;
		tmTime.tm_min	= iMinuteEnd;

		return mktime(&tmTime);
	}

	//--- 是否到达时间
	inline bool	isInTime	(const uint64& uTime, bool bCalcSecond = false) const
	{
        if ( isInitialTime() )
            return true;

		uint64 uWeekBeginTime = 0;
		if(getWeekBeginTime(uTime, uWeekBeginTime))
		{
			if(uTime < uWeekBeginTime)
				return false;

			uint64 uWeekEndTime = 0;
			if(getWeekEndTime(uTime, uWeekEndTime))
			{
				if(uTime < uWeekEndTime)
					return true;
			}

			return false;
		}

		tm		tmTime;
		getLocalTime(&tmTime, &uTime);

		uint64 uTmpTime = uTime;
		if(bCalcSecond)
			tmTime.tm_sec = 0;

		//周判断
		if (!isWeek(tmTime.tm_wday))
			return false;

 		//日判断
 		if(!isDay(tmTime.tm_mday))
 			return false;

		//开始时间
		if(iYearBegin > 1900)
			tmTime.tm_year = iYearBegin - 1900;
		//月
		if(iMonthBegin > 0)
			tmTime.tm_mon = iMonthBegin - 1;
		//日
		if(iDayBegin > 0)
			tmTime.tm_mday = iDayBegin;
		//时
		if(iHourBegin >= 0)
			tmTime.tm_hour = iHourBegin;
		//分
		if(iMinuteBegin >= 0)
			tmTime.tm_min = iMinuteBegin;

		//开始时间是否到达
		uint64 utime = mktime(&tmTime);
		if(utime > uTmpTime)
			return false;

		getLocalTime(&tmTime,&utime);

		//结束时间
		//年
		if(iYearEnd > 1900)
			tmTime.tm_year = iYearEnd - 1900;
		//月
		if(iMonthEnd > 0)
			tmTime.tm_mon = iMonthEnd - 1;
		else if(iMonthBegin > 0)
			tmTime.tm_mon = iMonthBegin - 1;
		//日
		if(iDayEnd > 0)
			tmTime.tm_mday = iDayEnd;
		//时
		if(iHourEnd >= 0)
			tmTime.tm_hour = iHourEnd;
		//分
		if(iMinuteEnd >= 0)
			tmTime.tm_min = iMinuteEnd;
		//结束时间是否到达
		utime = mktime(&tmTime);
		if(utime < uTmpTime)
			return false;

		return true;
	}

	//--- 从字符串读取数据
	bool	readFromString(std::string&strString)
	{//[*/年/年-年][*/月/月-月][[*/日-日]/[w周,w周]/[w周-w周]/[w周-w周,w周,w周]/[d日,d日]/[d日-d日]/[d日-d日,d日,d日]][时间-时间]
		initDateTime();
		if(strString.empty())
			return false;

		if(strString[0] != '[')
			return false;

		std::string strTemp;
		//年
		{
			truncateString(strString,'[');	//剔除年前[
			getSubString(strString,strTemp,']');
			if(strTemp.empty())
				return false;
			if(!haveFromString(strTemp,'*'))
			{
				iYearBegin = (int16)readInt32FromString(strTemp,'-');
				if(!strTemp.empty())
					iYearEnd = (int16)readInt32FromString(strTemp,']');
			}
		}//end年
		
		//月
		{
			truncateString(strString,'[');	//剔除月前[
			getSubString(strString,strTemp,']');
			if(strTemp.empty())
				return false;
			if(!haveFromString(strTemp,'*'))
			{
				iMonthBegin = (int8)readInt32FromString(strTemp,'-');
				if(!strTemp.empty())
					iMonthEnd = (int8)readInt32FromString(strTemp,']');
			}
		}//end月

		//日/周
		{
			truncateString(strString,'[');	//剔除日/周前[
			getSubString(strString,strTemp,']');
			if ( strTemp.empty() )
				return false;

			if ( !haveFromString( strTemp, '*' ) )
			{
				if(haveFromString(strTemp, 'W'))
				{
					_SET_BIT32(uMark, Week_Part, true);
					replaceString(strTemp, "W", "w");
				}

				if ( haveFromString( strTemp, 'w' ) )
				{//周
                    strarray&& weeks = splitStr( strTemp.c_str(), ',' );
                    for ( strarray::size_type widx = 0; widx < weeks.size(); ++widx )
                    {
                        std::string strWeek = weeks[ widx ];
                        if ( haveFromString( strTemp, '-' ) )
                        {
                            uint8 weekBegin = 0xff;
                            uint8 weekEnd = 0xff;
                            std::string strWeekBegin;
                            getSubString( strTemp, strWeekBegin, '-' );
                            if ( !strWeekBegin.empty() )
                            {
                                truncateString( strWeekBegin, 'w' ); // 剔除周前w
                                weekBegin = ( uint8 )dAtoi( strWeekBegin.c_str() );
                            }

                            if ( !strTemp.empty() )
                            {
                                truncateString( strTemp, 'w' ); // 剔除周前w
                                weekEnd = ( uint8 )dAtoi( strTemp.c_str() );
                            }

                            if ( weekBegin < 7 && weekEnd < 7 )
                            {
                                for ( uint8 i = weekBegin; i <= weekEnd; ++i )
                                    _SET_BIT32( uWeek, i, true );
                            }
                        }
                        else
                        {
                            truncateString( strWeek, 'w' );	// 剔除周前w
                            uint8 week = ( uint8 )dAtoi( strWeek.c_str() );
                            if ( week < 7 )
                                _SET_BIT32( uWeek, week, true );
                        }
                    }
				}
 				else if( haveFromString( strTemp, 'd' ) )
 				{//日（位）
                    strarray&& days = splitStr( strTemp.c_str(), ',' );
                    for ( strarray::size_type didx = 0; didx < days.size(); ++didx )
                    {
                        std::string strDay = days[ didx ];
                        if ( haveFromString( strTemp, '-' ) )
                        {
                            uint8 dayBegin = 0xff;
                            uint8 dayEnd = 0xff;
                            std::string strDayBegin;
                            getSubString( strTemp, strDayBegin, '-' );
                            if ( !strDayBegin.empty() )
                            {
                                truncateString( strDayBegin, 'd' ); // 剔除日前d
                                dayBegin = ( uint8 )dAtoi( strDayBegin.c_str() );
                            }

                            if ( !strTemp.empty() )
                            {
                                truncateString( strTemp, 'd' ); // 剔除日前d
                                dayEnd = ( uint8 )dAtoi( strTemp.c_str() );
                            }

                            if ( dayBegin >= 1 && dayBegin <= 31 && dayEnd >= 1 && dayEnd <= 31 )
                            {
                                for ( uint8 i = dayBegin; i <= dayEnd; ++i )
                                    _SET_BIT32( uDay, i, true );
                            }
                        }
                        else
                        {
                            truncateString( strDay, 'd' );	// 剔除日前d
                            uint8 day = ( uint8 )dAtoi( strDay.c_str() );
                            if ( day >= 1 && day <= 31 )
                                _SET_BIT32( uDay, day, true );
                        }
                    }
 				}
				else
				{//日
					std::string strDay;
					getSubString( strTemp, strDay, '-' );
					if ( strDay.empty() )
						return false;

					iDayBegin = ( int8 )readInt32FromString( strDay, '-' );
					getSubString( strTemp, strDay );
					if ( !strDay.empty() )
						iDayEnd	= ( int8 )readInt32FromString( strDay );
				}
			}
		}//end日/周

		//时间段
		{
			truncateString(strString,'[');	//时间前[
			getSubString(strString,strTemp,':');
			if(strTemp.empty())
				return false;

			iHourBegin	=(int8)readInt32FromString(strTemp);
			getSubString(strString,strTemp,'-');
			if(strTemp.empty())
				return false;
			iMinuteBegin=(int8)readInt32FromString(strTemp);

			if(!strString.empty())
			{
				getSubString(strString,strTemp,':');
				if(!strTemp.empty())
				{
					iHourEnd	=(int8)readInt32FromString(strTemp);
					getSubString(strString,strTemp);
					if(!strTemp.empty())
						iMinuteEnd=(int8)readInt32FromString(strTemp);
				}
			}
		}//end时间段
		return true;
	}
};



inline bool isInOpenTime(stl_vector<_stDateTime>& vecDateTime, uint64 uTime)
{
	if(vecDateTime.empty())
		return true;
	for (uint32 i = 0; i < vecDateTime.size(); ++i)
	{
		_stDateTime& stOpenTime = vecDateTime[i];
		if(stOpenTime.isInTime(uTime))
		{
			return true;
		}
	}
	return false;
}

inline bool getOpenTime(stl_vector<_stDateTime>& vecDateTime, uint32 uDurationTime,  uint64 uCurTime, uint64* pBeginTime = nullptr, uint64* pEndTime = nullptr)
{
	bool bRet = false;
	uint64 uTimeBegin = 0;
	uint64 uTimeEnd = 0;
	if(vecDateTime.empty())
	{
		bRet = true;
		uTimeBegin = uCurTime;
		uTimeEnd = uCurTime + uDurationTime;
	}
	for (uint32 i = 0; i < vecDateTime.size(); ++i)
	{
		_stDateTime& stOpenTime = vecDateTime[i];
		if(stOpenTime.isInTime(uCurTime))
		{
			bRet = true;
			uTimeBegin = stOpenTime.getBeginTime(uCurTime);
			uTimeEnd = stOpenTime.getEndTime(uCurTime);
		}
	}
	if(pBeginTime != nullptr)
	{
		*pBeginTime = uTimeBegin;
	}
	if(pEndTime != nullptr)
	{
		*pEndTime = uTimeEnd;
	}
	return bRet;
}

//获取比当前时间大的最小开始时间
inline uint64 getMinBeginTime(stl_vector<_stDateTime>& vecDateTime, uint64 uCurrTime)
{
	uint64 uTime = 0;
	uint64 uTemp = 0;

	uint32 uSize = vecDateTime.size();
	for(uint32 i = 0; i < uSize; ++i)
	{
		_stDateTime& stTime = vecDateTime[i];
		uTemp = stTime.getBeginTime(uCurrTime);

		if(uTemp < uCurrTime && stTime.iDayBegin == -1)
			uTemp = stTime.getBeginTime(uCurrTime+86400);

		if(uTemp < uCurrTime)
			continue;

		if(uTime == 0 || uTemp < uTime)
			uTime = uTemp;
	}

	return uTime;
}

//是否在指定时间内
inline bool isInTime(const stl_vector<_stDateTime>& vecDateTime, uint64 uCurrTime)
{
	uint32 uSize = vecDateTime.size();
	if(uSize == 0)
		return true;

	for(uint32 i = 0; i < uSize; ++i)
	{
		if(vecDateTime[i].isInTime(uCurrTime))
			return true;
	}

	return false;
}

/*************************************************************/
#pragma pack(pop)
