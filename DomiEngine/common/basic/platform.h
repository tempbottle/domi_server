
#pragma once
#include "common/basic/basicTypes.h"
/*************************************************************/
//#############################################################
//############################## error
//#############################################################
//--- ��ô�����
extern long			get_error		();
//--- ��ô����ַ���
extern const char*	get_error_str	(long _error);
//--- ��ô����ַ���
/* linux�µ���ʱ�޴��������ӡ��� */
extern const char*	get_herror_str	();
//--- ��ô����ַ���
extern const char*	get_herror_str	(long _error);

//#############################################################
//############################## �ļ�Ŀ¼���
//#############################################################
//--- ��ѯĿ¼�Ƿ����
extern bool	findDirectory	(pc_str _dir);
//--- ����Ŀ¼
extern bool	createDirectory	(pc_str _dir,char _break = '/');