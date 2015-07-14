/******************************************************************** 
����ʱ��:        2015/07/01 10:26
�ļ�����:        memoryFunctions.h
�ļ�����:        Domi
����˵��:        �ڴ������غ���  
����˵��:        ����ƽ̨��װ
*********************************************************************/

#pragma once
#include <iostream>
#include <stdio.h>
#include "common/basic/basicTypes.h"


#ifdef WIN32	//WIN32
//�ڴ濽��
extern int32	dMemcpy(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount);
//�ڴ��ƶ�
extern int32	dMemmove(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount);
#else	// linux
//�ڴ濽��
extern void*	dMemcpy(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount);
//�ڴ��ƶ�
extern void*	dMemmove(void* _Dst,size_t _DstSize,const void* _Src,size_t _MaxCount);
#endif
