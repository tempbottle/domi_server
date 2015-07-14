#pragma once
#include "common/basic/basicTypes.h"
#include <map>

/*************************************************************/
extern int32	dAtoi			(const char*pString);
extern uint32	dAtoui			(const char*pString);
extern long		dAtol			(const char*pString);
extern double	dAtof			(const char*pString);
extern int64	dAtoll			(const char*pString);
extern uint64	dAtoull			(const char*pString);
//------------------------------------------------------
//------------------------------- Value Function...
//--- ���uint32�����а�������λ
extern uint8	getNumberBit	(uint32 uNumber);

//--- ����ۼ���  positive
extern uint32	getAddValue		(uint32 uFrom,uint32 uValue,uint32 nMax);

//--- ������
extern uint32	addValue		(uint32 uValue,int nValue,uint32 nMax = uint32(-1));

//--- ����intֵ
extern bool		setInt			(int32&iOld,int32 iNew,int32 iMax = 0x7FFFFFFF,int32 iMin = 0/*0x80000000*/);
extern bool		setInt8			(int8&iOld,int8 iNew,int8 iMax = 0x7F,int8 iMin=0);
//--- ����intֵ
extern bool		updateInt		(int32&iValue,int32 iUpdate,int32 iMax = 0x7FFFFFFF,int32 iMin = 0);

//--- ����int32ֵ
extern bool		updateUint32	(uint32&uValue,int32 iUpdate,uint32 uMax = 0xFFFFFFFF);
extern bool		canUpdateUint32	(uint32 uValue,int32 iUpdate,uint32 uMax = 0xFFFFFFFF);
extern bool		canUpdateUint32	(uint32 uValue,uint32 uUpdate,bool bAdd,uint32 uMax = 0xFFFFFFFF);
extern bool		updateUint32	(uint32&uValue,uint32&uUpdate,bool bAdd,uint32 uMax = 0xFFFFFFFF);
extern bool		updateUint32_	(uint32&uValue,uint32 uUpdate,bool bAdd,uint32 uMax = 0xFFFFFFFF);

//--- ����int64ֵ
extern bool		updateUint64	(uint64&uValue,int64 iUpdate,uint64 uMax = 0xFFFFFFFFFFFFFFFF);
extern bool		canUpdateUint64	(uint64 uValue,int64 iUpdate,uint64 uMax = 0xFFFFFFFFFFFFFFFF);
extern bool		canUpdateUint64	(uint64 uValue,uint64 uUpdate,bool bAdd,uint64 uMax = 0xFFFFFFFFFFFFFFFF);
extern bool		updateUint64	(uint64&uValue,uint64&uUpdate,bool bAdd,uint64 uMax = 0xFFFFFFFFFFFFFFFF);
extern bool		updateUint64_	(uint64&uValue,uint64 uUpdate,bool bAdd,uint64 uMax = 0xFFFFFFFFFFFFFFFF);

extern bool		canUpdateInt64	(int64 iValue,int64 iUpdate,int64 iMax = 0x7FFFFFFFFFFFFFFF,int64 iMin = 0x8000000000000000);
extern bool		updateInt64		(int64 &iValue,int64 iUpdate,int64 iMax = 0x7FFFFFFFFFFFFFFF,int64 iMin = 0x8000000000000000);

//--- ��ȡ����ֵ
extern uint32	getUint32Proportion	(uint32 uValue,float32 fRatio,bool bRoundedUp = true);

/*--- ����
@_molecular		=����
@_denominator	=��ĸ
@_floor			=����ȡ��
*/
extern uint32	dDivisionUint32	(uint64 _molecular,uint32 _denominator,bool _floor = true);
extern uint64	dDivisionUint64	(uint64 _molecular,uint64 _denominator,bool _floor = true);

/*--- �˷�
*/
extern bool		canMultiplicationUint16(uint16 _multiplier, uint16 _multiplicand, uint16 uMax = 0xFFFF);
extern bool		canMultiplicationUint32(uint32 _multiplier, uint32 _multiplicand, uint32 uMax = 0xFFFFFFFF);
extern void		multiplicationUint32(uint32& _multiplier, float _multiplicand, uint32 uMax = 0xFFFFFFFF);

//-------------------------------------------------------------
//------------------------------ ָ������(��ָ���ں�)
template<class T>
inline void	_qsort_pointer(T *_list, const uint32&_count)
{
	if(!_list || !_count)
		return;

	for (uint32 i = 0;i < _count;++i)
	{
		if(_list[i])
			continue;

		bool _empty = true;
		for (uint32 j = i + 1;j < _count;++j)
		{
			if(!_list[j])
				continue;

			_empty	= false;
			_list[i]= _list[j];
			_list[j]= nullptr;
			break;
		}
		if(_empty)
			break;
	}
}

//-------------------------------------------------------------
//------------------------------ ����ָ��ֵ
template<class _T>
inline void	setValue(_T*_pointer,_T _value)
{
	if(_pointer)
		*_pointer = _value;
}

//-------------------------------------------------------------
//------------------------------ ���32λ
inline bool checkBit32(uint32 uMark, uint8 uBit)
{
	return _CHECK_BIT(uMark, _BIT32(uBit));
}

//-------------------------------------------------------------
//------------------------------ ����32λ
inline uint32 setBit32(uint32 uMark, uint8 uBit, bool bSet)
{
	_SET_BIT32(uMark, uBit, bSet);
	return uMark;
}