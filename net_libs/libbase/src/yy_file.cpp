#include "yy_file.h"
#include "yy_exception.h"

#include <stdio.h>
#include <sys/stat.h>
#include <Windows.h>
#include <direct.h>
#include <string>
#include <io.h>
#include <errno.h>

#define MAXFILEPATHLEN 1024
#define MAXDIRLEN 1024



NS_YY_BEGIN

void FilePathCheck(const char* pFilePath)
{
	if(NULL == pFilePath || '\0' == pFilePath[0])
		THROWNEXCEPT("file path is invalid.");

	size_t len = strlen(pFilePath);
	if(len > MAXFILEPATHLEN)
		THROWNEXCEPT("file path len limit:%d, current file path len is:%d.", MAXFILEPATHLEN, len);
}

void MkFile(const char* pFilePath)
{
	FilePathCheck(pFilePath);

	FILE* file = NULL;
	errno_t err = fopen_s(&file, pFilePath, "w");
	if(0 != err)
		THROWNEXCEPT("create file:%s error.errno:%d", pFilePath, GetLastError());

	fclose(file);
}

void RmFile(const char* pFilePath)
{
	FilePathCheck(pFilePath);

	if(0 != remove(pFilePath))
		THROWNEXCEPT("remove file:%s error.errno:%d", pFilePath, GetLastError());
}

bool IsFileExist(const char* pFilePath)
{
	FilePathCheck(pFilePath);

	//00: existence, 02:w, 04:r, 06:wr
	if(-1 == _access(pFilePath, 0))
	{
		return false;
	}

	return true;
}

bool IsFile(const char* pFilePath)
{
	FilePathCheck(pFilePath);

	struct __stat64 buf;
	if(0 != _stat64(pFilePath, &buf))
	{
		//可能文件路径有非法字符等，属于逻辑错误，这里抛出异常，结束进程。
		switch(errno)
		{
		case ENOENT:
			THROWNEXCEPT("file path:%s not found.", pFilePath);
			break;
		case EINVAL:
			THROWNEXCEPT("invalid parameter:%s to _stat.", pFilePath);
			break;
		default:
			THROWNEXCEPT("unexpected error in _stat.errno:%d", GetLastError());
		}
	}

	if(_S_IFMT&buf.st_mode)
		return true;

	return false;
}

UI64 GetFileSize(const char* pFilePath)
{
	FilePathCheck(pFilePath);

	struct __stat64 buf;
	if(_stat64(pFilePath, &buf)!=0)
	{
		//可能文件路径有非法字符等，属于逻辑错误，这里抛出异常，结束进程。
		switch(errno)
		{
		case ENOENT:
			THROWNEXCEPT("file path:%s not found.", pFilePath);
			break;
		case EINVAL:
			THROWNEXCEPT("invalid parameter:%s to _stat.", pFilePath);
			break;
		default:
			THROWNEXCEPT("unexpected error in _stat.errno:%d", GetLastError());
		}
	}

	return buf.st_size;
}


void DirPathCheck(const char* pDirPath)
{
	if(NULL == pDirPath || '\0' == pDirPath[0])
		THROWNEXCEPT("dir path is invalid.");

	size_t len = strlen(pDirPath);
	if(len>MAXDIRLEN)
		THROWNEXCEPT("dir path len limit:%d, current path:%s len is:%d.", MAXDIRLEN, pDirPath, len);
}

void MkDir(const char* pDirPath)
{
	DirPathCheck(pDirPath);

	//check whether dir is exist
	if(IsDirExist(pDirPath))
		return;

	size_t len = strlen(pDirPath);
	char tmpDirPath[MAXDIRLEN] = {0};
	strcpy_s(tmpDirPath, sizeof(tmpDirPath), pDirPath);

	//dir is end of '\\'
	if('\\' == tmpDirPath[len-1])
		tmpDirPath[len-1]='\0';

	for(size_t i=0; i<len; i++)
	{
		if('\\' == tmpDirPath[i])
		{
			tmpDirPath[i] = '\0';

			//check whether dir is exist
			if(IsDirExist(tmpDirPath))
			{
				tmpDirPath[i] = '\\';
				continue;
			}

			if(_mkdir(tmpDirPath) != 0)
				THROWNEXCEPT("make dir:%s error.errno:%d", tmpDirPath, GetLastError());

			tmpDirPath[i] = '\\';
		}
	}

	if(_mkdir(tmpDirPath) !=0)
		THROWNEXCEPT("make dir:%s error.errno:%d", tmpDirPath, GetLastError());
}

void RmDir(const char* pDirPath)
{
	DirPathCheck(pDirPath);

	size_t len = strlen(pDirPath);
	char tmpDirPath[MAXDIRLEN]={0};

	//dir is end of '\\'
	if('\\' == pDirPath[len-1])
		sprintf_s(tmpDirPath, sizeof(tmpDirPath), "%s*.*", tmpDirPath);
	else
		sprintf_s(tmpDirPath, sizeof(tmpDirPath), "%s\\*.*", tmpDirPath);

	int hasFile=-1;
	char path[MAXDIRLEN]={0};
	struct _finddatai64_t c_file;
	intptr_t Handle = _findfirsti64(tmpDirPath, &c_file);
	if(-1 == Handle)
		return;

	do
	{
		if((strcmp(c_file.name, ".") !=0 )
			&& (strcmp(c_file.name, "..") != 0))
		{
			sprintf_s(path, sizeof(path), "%s\\%s", pDirPath, c_file.name);

			if(c_file.attrib & _A_SUBDIR)
			{
				//递归
				RmDir(path);
			}
			else
			{
				RmFile(path);
			}
		}

		hasFile=_findnexti64(Handle, &c_file);
	}
	while(0 == hasFile);

	_findclose(Handle);
	if(-1 == _rmdir(pDirPath))
	{
		THROWNEXCEPT("remove dir:%s error. errorno:%d", path, GetLastError());
	}
}


bool IsDir(const char* pDirPath)
{
	struct _stat buf;
	if(0 != _stat(pDirPath, &buf))
	{
		//get dir path information error.
		switch(errno)
		{
		case ENOENT:
			THROWNEXCEPT("pDirPath:%s not found.", pDirPath);
			break;
		case EINVAL:
			THROWNEXCEPT("invalid parameter:%s to _stat.", pDirPath);
			break;
		default:
			THROWNEXCEPT("unexpected error in _stat.errno:%d", GetLastError());
		}
	}

	if(_S_IFDIR&buf.st_mode)
		return true;

	return false;
}

bool IsDirExist(const char* pDirPath)
{
	if(-1 == _access(pDirPath, 0))
	{
		return false;
	}

	return true;
}

void CopyFile(const char* dst, const char* src)
{
	FilePathCheck(dst);
	FilePathCheck(src);

	//use fopen or iostream to non-platform specific
	//use windows api:CopyFile
	if(FALSE == ::CopyFileA(src, dst, FALSE))
	{
		//copy fail, mayge the dir is not exist.
		std::string str_file(dst);
		int index=str_file.find_last_of('\\');
		std::string str_path=str_file.substr(0, index);
		if(!IsDirExist(str_path.c_str()))
		{
			MkDir(str_path.c_str());
			if(FALSE == ::CopyFileA(src, dst, FALSE))
				THROWNEXCEPT("copy file from:%s to %s error.errno:%d", src, dst, GetLastError());
		}
	}
}

void CopyDir(const char* pDst, const char* pSrc)
{
	DirPathCheck(pDst);
	DirPathCheck(pSrc);

	//检查是否以\\结尾
	char srcDirPath[MAXDIRLEN]={0};
	char dstDirPath[MAXDIRLEN]={0};
	strcpy_s(srcDirPath, sizeof(srcDirPath), pSrc);
	strcpy_s(dstDirPath, sizeof(dstDirPath), pDst);

	size_t len = strlen(pSrc);
	if('\\' == pSrc[len-1])
	{
		srcDirPath[len-1]='\0';
	}

	len = strlen(pDst);
	if('\\' == pDst[len-1])
	{
		srcDirPath[len-1]='\0';
	}

	//check dst dir exist.
	if(!IsDirExist(dstDirPath))
	{
		MkDir(dstDirPath);
	}

	int hasFile=-1;
	char dirPathWC[MAXDIRLEN]={0};
	sprintf_s(dirPathWC, sizeof(dirPathWC), "%s\\*.*", srcDirPath);

	char subDstPath[MAXDIRLEN]={0};
	char subSrcPath[MAXDIRLEN]={0};
	struct _finddatai64_t c_file;
	intptr_t ret = _findfirsti64(dirPathWC, &c_file);
	if(-1 == ret)
		return;

	do
	{
		if(strcmp(c_file.name, ".")!=0 && strcmp(c_file.name, "..") != 0)
		{
			sprintf_s(subSrcPath, MAXDIRLEN, "%s\\%s", srcDirPath, c_file.name);
			sprintf_s(subDstPath, MAXDIRLEN, "%s\\%s", dstDirPath, c_file.name);

			if(c_file.attrib & _A_SUBDIR)
			{
				CopyDir(subDstPath, subSrcPath);
			}
			else
			{
				CopyFile(subDstPath, subSrcPath);
			}
		}

		hasFile=_findnexti64(ret, &c_file);
	}
	while(0 == hasFile);

	_findclose(ret);
}

NS_YY_END