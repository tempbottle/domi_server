/******************************************************************** 
创建时间:        2015/07/01 10:28
文件名称:        memoryFunctions.cpp
文件作者:        Domi
*********************************************************************/

#include "common/basic/memoryFunctions.h"
#include <string.h>
#include <memory>

#ifdef WIN32 //windows
int32 dMemcpy(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount)
{
	return ::memcpy_s(_Dst,_DstSize,_Src,_MaxCount);
}

int32 dMemmove(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount)
{
	return ::memmove_s(_Dst,_DstSize,_Src,_MaxCount);
}
#else	//linux	
void* dMemcpy(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount)
{
	if(!_Dst || !_DstSize || !_Src || !_MaxCount)
		return _Dst;

	if(_MaxCount > _DstSize)
		_MaxCount = _DstSize;

	return ::memcpy((char*)_Dst,(char*)_Src,_MaxCount);
}
void* dMemmove(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount)
{
	if(!_Dst || !_DstSize || !_Src || !_MaxCount)
		return _Dst;

	if(_MaxCount > _DstSize)
		_MaxCount = _DstSize;

	return ::memmove((char*)_Dst,(char*)_Src,_MaxCount);
}
#endif
