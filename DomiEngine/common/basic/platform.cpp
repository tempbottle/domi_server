
#include "common/basic/platform.h"
#include "common/basic/stringFunctions.h"
#ifdef WIN32
#include <windows.h>
#else//WIN32
#include <sys/types.h>	//ϵͳ���Ͷ���
#include <errno.h>		//�������ļ��ͻ�����errno����
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>
#endif//WIN32
/*************************************************************/
//-------------------------------------------------------------
//------------------------------ ��ô�����
long		get_error		()
{
	long _error = 0;
#ifdef WIN32
	_error = ::GetLastError();
#else//WIN32
	_error = errno;
#endif//WIN32

	return _error;
}
//-------------------------------------------------------------
 //------------------------------ ��ô����ַ���
const char*	get_error_str	(long _error)
{
	const char* _error_str = nullptr;
#ifdef WIN32
	static char szMessage[512] = {0};
	_error_str = szMessage;
	long _size = ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
										nullptr,
										_error,
										MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),/*Ĭ������*/ 
										szMessage,
										sizeof(szMessage),
										nullptr );

	szMessage[512 - 1] = 0;
	for (long i = 0;i < _size;i++)
	{
		if (szMessage[i] < 0 || szMessage[i] > 31)
			continue;

		szMessage[i] = 0;
		break;
	}
#else//WIN32
	_error_str = strerror(_error);
#endif//WIN32

	return _error_str;
}

//-------------------------------------------------------------
//------------------------------ ��ô����ַ���
const char*	get_herror_str	()
{
	const char* _error_str = nullptr;
#ifdef WIN32
	_error_str = get_error_str(get_error());
#else//WIN32
	char* _str = nullptr;
	herror(_str);

	_error_str = _str;
#endif//WIN32

	return _error_str;
}
//-------------------------------------------------------------
//------------------------------ ��ô����ַ���
const char*	get_herror_str	(long _error)
{
	const char* _error_str = nullptr;
#ifdef WIN32
	_error_str = get_error_str(_error);
#else//WIN32
	_error_str = hstrerror(_error);
#endif//WIN32

	return _error_str;
}
//#############################################################
//############################## �ļ�Ŀ¼���
//#############################################################
/*linux
//------------------------------ 
stat��ȡ���ļ�״̬��  
��غ���  fstat��lstat��chmod��chown��readlink��utime
��ͷ�ļ�  #include<sys/stat.h>
#include<unistd.h>
���庯��  int stat(const char * file_name,struct stat *buf);
����˵��  stat()����������file_name��ָ���ļ�״̬�����Ƶ�����buf��ָ�Ľṹ�С�
������struct stat�ڸ�������˵��
//------------------------------ 

//------------------------------ 
*����Ƿ���Զ�/дĳһ�Ѵ��ڵ��ļ�
��ͷ�ļ�  #include<unistd.h>
���庯��  int access(const char * pathname,int mode);
@mode:R_OK������W_OK��д��X_OK��ִ�С�F_OK�����ڡ�
���أ�����������˵�Ȩ�޶�ͨ���˼���򷵻�0ֵ����ʾ�ɹ���ֻҪ��һȨ�ޱ���ֹ�򷵻�-1
�������:
EACCESS ����pathname ��ָ�����ļ���������Ҫ����Ե�Ȩ�ޡ�
EROFS ������д��Ȩ�޵��ļ�������ֻ���ļ�ϵͳ�ڡ�
EFAULT ����pathnameָ�볬���ɴ�ȡ�ڴ�ռ䡣
EINVAL ����mode ����ȷ��
ENAMETOOLONG ����pathname̫����
ENOTDIR ����pathnameΪһĿ¼��
ENOMEM �����ڴ治��
ELOOP ����pathname�й�������������⡣
EIO I/O ��ȡ����
//------------------------------ 
*����ĸ˳������Ŀ¼�ṹ
��ͷ�ļ�  #include<dirent.h>
���庯��  int alphasort(const struct dirent **a,const struct dirent **b);
����˵��  alphasort()Ϊscandir()������qsort()����ʱ����qsort()��Ϊ�жϵĺ�������ϸ˵����ο�scandir()��qsort()��
����ֵ  �ο�qsort()��
����:
//* ��ȡ/Ŀ¼�����е�Ŀ¼�ṹ��������ĸ˳������
main()
{
	struct dirent **namelist;
	int i,total;
	total = scandir(��/��,&namelist ,0,alphasort);
	if(total <0)
		perror(��scandir��);
	else
	{
		for(i=0;i<total;i++)
			printf(��%s\n��,namelist[i]->d_name);
		printf(��total = %d\n��,total);
	}
}
//------------------------------ 

//------------------------------ 
*chdir���ı䵱ǰ�Ĺ�����Ŀ¼��  
��غ���  getcwd��chroot
��ͷ�ļ�  #include<unistd.h>
���庯��  int chdir(const char * path);
����˵��  chdir()��������ǰ�Ĺ���Ŀ¼�ı���Բ���path��ָ��Ŀ¼��
����ֵ  ִ�гɹ��򷵻�0��ʧ�ܷ���-1��errnoΪ������롣
//------------------------------ 
*fchdir���ı䵱ǰ�Ĺ���Ŀ¼��  
��غ���  getcwd��chroot
��ͷ�ļ�  #include<unistd.h>
���庯��  int fchdir(int fd);
����˵��  fchdir()��������ǰ�Ĺ���Ŀ¼�ı���Բ���fd ��ָ���ļ������ʡ�
����ִֵ  �гɹ��򷵻�0��ʧ�ܷ���-1��errnoΪ������롣
//------------------------------ 
*getcwd��ȡ�õ�ǰ�Ĺ���Ŀ¼��  
��غ���  get_current_dir_name��getwd��chdir
��ͷ�ļ�  #include<unistd.h>
���庯��  char * getcwd(char * buf,size_t size);
����˵��  getcwd()�Ὣ��ǰ�Ĺ���Ŀ¼����·�����Ƶ�����buf��ָ���ڴ�ռ䣬����sizeΪbuf�Ŀռ��С��
�ڵ��ô˺���ʱ��buf��ָ���ڴ�ռ�Ҫ�㹻��������Ŀ¼����·�����ַ������ȳ�������size��С�����ֵNULL��
errno��ֵ��ΪERANGE����������bufΪNULL��getcwd()��������size�Ĵ�С�Զ������ڴ�(ʹ��malloc())��
�������sizeҲΪ0����getcwd()��������Ŀ¼����·�����ַ����̶������������õ��ڴ��С�����̿�����ʹ������ַ���������free()���ͷŴ˿ռ䡣
����ֵ  ִ�гɹ��򽫽�����Ƶ�����buf��ָ���ڴ�ռ䣬���Ƿ����Զ����õ��ַ���ָ�롣ʧ�ܷ���NULL������������errno��
//------------------------------ 

//------------------------------ 
*chmod���ı��ļ���Ȩ�ޣ�  
��غ���  fchmod��stat��open��chown
��ͷ�ļ�  #include<sys/types.h>
#include<sys/stat.h>
���庯��  int chmod(const char * path,mode_t mode);
����˵��  chmod()��������mode Ȩ�������Ĳ���path ָ���ļ���Ȩ�ޡ�

//------------------------------ 
closedir���ر�Ŀ¼��  
��غ���  opendir
��ͷ�ļ�  #include<sys/types.h>
#include<dirent.h>
���庯��  int closedir(DIR *dir);
//------------------------------ 
opendir����Ŀ¼��  
��غ���  open��readdir��closedir��rewinddir��seekdir��telldir��scandir
��ͷ�ļ�  #include<sys/types.h>
#include<dirent.h>
���庯��  DIR * opendir(const char * name);
����˵��  opendir()�����򿪲���nameָ����Ŀ¼��������DIR*��̬��Ŀ¼������open()���ƣ���������Ŀ¼�Ķ�ȡ��������Ҫʹ�ô˷���ֵ��
����ֵ  �ɹ��򷵻�DIR* ��̬��Ŀ¼������ʧ���򷵻�NULL��
//------------------------------ 
readdir����ȡĿ¼��  
��غ���  open��opendir��closedir��rewinddir��seekdir��telldir��scandir
��ͷ�ļ�  #include<sys/types.h>
#include<dirent.h>
���庯��  struct dirent * readdir(DIR * dir);
����˵��  readdir()���ز���dirĿ¼�����¸�Ŀ¼����㡣
�ṹdirent��������
struct dirent
{
	ino_t d_ino;
	ff_t d_off;
	signed short int d_reclen;
	unsigned char d_type;
	har d_name[256;
	};
d_ino ��Ŀ¼������inode
d_off Ŀ¼�ļ���ͷ����Ŀ¼������λ��
d_reclen _name�ĳ��ȣ�������NULL�ַ�
d_type d_name ��ָ���ļ�����
d_name �ļ���
����ֵ  �ɹ��򷵻��¸�Ŀ¼����㡣�д��������ȡ��Ŀ¼�ļ�β�򷵻�NULL��
//------------------------------ 
rewinddir�������ȡĿ¼��λ��Ϊ��ͷλ�ã�  
��غ���  open��opendir��closedir��telldir��seekdir��readdir��scandir
��ͷ�ļ�  #include<sys/types.h>
#include<dirent.h>
���庯��  void rewinddir(DIR *dir);
����˵��  rewinddir()�������ò���dir Ŀ¼��Ŀǰ�Ķ�ȡλ��Ϊԭ����ͷ�Ķ�ȡλ�á�
//------------------------------ 
seekdir�������»ض�ȡĿ¼��λ�ã�  
��غ���  open��opendir��closedir��rewinddir��telldir��readdir��scandir
��ͷ�ļ�  #include<dirent.h>
���庯��  void seekdir(DIR * dir,off_t offset);
����˵��  seekdir()�������ò���dirĿ¼��Ŀǰ�Ķ�ȡλ�ã��ڵ���readdir()ʱ��Ӵ���λ�ÿ�ʼ��ȡ������offset �������Ŀ¼�ļ���ͷ��ƫ������
//------------------------------ 
telldir��ȡ��Ŀ¼���Ķ�ȡλ�ã�  
��غ���  open��opendir��closedir��rewinddir��seekdir��readdir��scandir
��ͷ�ļ�  #include<dirent.h>
���庯��  off_t telldir(DIR *dir);
����˵��  telldir()���ز���dirĿ¼��Ŀǰ�Ķ�ȡλ�á��˷���ֵ�������Ŀ¼�ļ���ͷ��ƫ��������ֵ�����¸���ȡλ�ã��д�����ʱ����-1��
�������  EBADF����dirΪ��Ч��Ŀ¼����
//------------------------------ 

//------------------------------ 
utime���޸��ļ��Ĵ�ȡʱ��͸���ʱ�䣩  
��غ���  utimes��stat
��ͷ�ļ�  #include<sys/types.h>
#include<utime.h>
���庯��  int utime(const char * filename,struct utimbuf * buf);
����˵��  utime()�����޸Ĳ���filename�ļ�������inode��ȡʱ�䡣
�ṹutimbuf��������
struct utimbuf
{
	time_t actime;
	time_t modtime;
};
//------------------------------ 

//------------------------------ 
remove��ɾ���ļ���  
��غ���  link��rename��unlink
��ͷ�ļ�  #include<stdio.h>
���庯��  int remove(const char * pathname);

//------------------------------ 
rename�������ļ����ƻ�λ�ã�  
��غ���  link��unlink��symlink
��ͷ�ļ�  #include<stdio.h>
���庯��  int rename(const char * oldpath,const char * newpath);
����˵��  rename()�Ὣ����oldpath ��ָ�����ļ����Ƹ�Ϊ����newpath��ָ���ļ����ơ���newpath��ָ�����ļ��Ѵ��ڣ���ᱻɾ����
����ֵ  ִ�гɹ��򷵻�0��ʧ�ܷ���-1������ԭ�����errno

//------------------------------ 

*/
//-------------------------------------------------------------
//------------------------------ ��ѯĿ¼�Ƿ����
bool	findDirectory(pc_str _dir)
{
	if(!_dir || !*_dir)
		return false;

#ifdef WIN32
	WIN32_FIND_DATAA FileData; 
	HANDLE hSearch;
	hSearch = ::FindFirstFile(_dir, &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE)
	{ 
		::FindClose(hSearch);
		return false;
	}

	if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		::FindClose(hSearch);
		return true;
	}

	while(::FindNextFile(hSearch, &FileData))
	{ 
		if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			::FindClose(hSearch);
			return true;
		}
	} 

	::FindClose(hSearch);

#else // WIN32
	struct stat _stat;
	int _res = stat(_dir,&_stat);
	if(_res == 0 && S_ISDIR(_stat.st_mode))
		return true;
#endif // WIN32

	return false;
}

//-------------------------------------------------------------
//------------------------------ ����Ŀ¼
bool	createDirectory	(pc_str _dir,char _break)
{
	if(!_dir || !*_dir)
		return false;

#ifdef WIN32
	std::string strDir(_dir);
	std::string strMkDir;
	while(!strDir.empty())
	{
		std::string strTemp;
		getSubString(strDir,strTemp,_break);
		if(strTemp.empty())
			continue;

		if(!strMkDir.empty())
			strMkDir += _break;

		strMkDir += strTemp;
		if(!findDirectory(strMkDir.c_str()))
		{
			if(::CreateDirectory(strMkDir.c_str(),nullptr)!=TRUE)
				return false; 
		}
	}

#else // WIN32
	std::string strDir(_dir);
	std::string strMkDir;
	while(!strDir.empty())
	{
		std::string strTemp;
		getSubString(strDir,strTemp,_break);
		if(strTemp.empty())
			continue;

		if(!strMkDir.empty())
			strMkDir += _break;

		strMkDir += strTemp;
		if(access(strMkDir.c_str(),0)!=0)
		{
			if(mkdir(strMkDir.c_str(),0755)==-1)
				return false; 
		}
	}
#endif // WIN32
	return true;
}