/******************************************************************** 
创建时间:        2015/07/01 10:26
文件名称:        memoryFunctions.h
文件作者:        Domi
功能说明:        内存操作相关函数  
其他说明:        做跨平台封装
*********************************************************************/

#pragma once
#include <iostream>
#include <stdio.h>
#include "common/basic/basicTypes.h"


#ifdef WIN32	//WIN32
//内存拷贝
extern int32	dMemcpy(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount);
//内存移动
extern int32	dMemmove(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount);
#else	// linux
//内存拷贝
extern void*	dMemcpy(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount);
//内存移动
extern void*	dMemmove(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount);
#endif
