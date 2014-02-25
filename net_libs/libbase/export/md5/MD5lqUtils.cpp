// MD5lqUtils.cpp
//include "stdafx.h"
#include "MD5lqUtils.h"
#include "MD5lq.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
//#include <Windows.h>

inline bool GetMD5( const char* src, unsigned long size, char md5[32] ) 
{
    long param[4] = { 0x10325476, 0xEFCDAB89, 0x67452301, 0x98BADCFE };

    return MD5( src, md5, size, true, param[0], param[1], param[2], param[3] );
}

// 模拟ReadFile 改写读取内存
bool ReadMemBuffer(unsigned long nStartReadPos,         // 读的起始位置
				   unsigned long nEndReadPos,           // 结束位置
                    const char* lpMem,          // 原始数据内存地址
				   void* lpBuffer,             // 缓冲长度
				   unsigned long nNumberOfBytesToRead,  // 读的BUF长度
				   unsigned long *lpNumberOfBytesRead // 实际读出的长度
)
{
	// 寻址出错
	if(nStartReadPos > nEndReadPos)
		return false;
	//::ZeroMemory(lpBuffer,nNumberOfBytesToRead);
	memset(lpBuffer, 0, nNumberOfBytesToRead);
    unsigned long nDeffReadPos = nEndReadPos-nStartReadPos;

	// 如果文件内存地址插值读的位置和结束位置相差小于一个Buf长度,则说明快到文件尾部的
	if( nDeffReadPos <= nNumberOfBytesToRead )
	{
		::memcpy(lpBuffer,lpMem+nStartReadPos, nDeffReadPos );
		*lpNumberOfBytesRead = nDeffReadPos;
	}
	else //这里都是数据正常数据块读取
	{
		::memcpy(lpBuffer,lpMem+nStartReadPos, nNumberOfBytesToRead );
		*lpNumberOfBytesRead = nNumberOfBytesToRead;
	}

    return true;
}

// 根据内存算MD5码
bool GenerateMemInfo( const char* lpMem, unsigned long dwSize, char bufMD5[32] )
{
	if(!lpMem)
		return false;
    const unsigned long BLOCK_SIZE = 10 * 1024 * 1024;//文件分块，每块为10M
    char *buffer = new char[BLOCK_SIZE];
    if(buffer == NULL) 
        return false;

	bufMD5[0] = '\0';
	buffer[0] = '\0';
    
	bool bRe = true;
    char md5[64 + 1] = {'\0'};
    unsigned long size = BLOCK_SIZE;
	unsigned long start_pos = 0;//起始位置
	while(size == BLOCK_SIZE)	//读取一直到SIZE是文件尾，SIZE<=BLOCK_SIZE
	{
		//读取一个BUFFER
		if(::ReadMemBuffer(start_pos,dwSize,lpMem,buffer,BLOCK_SIZE, &size) != true)
		{
			bRe = false;
			break;
		}
		start_pos += BLOCK_SIZE;

		//一个数据块产生一个MD5
        GetMD5( buffer, size, md5 );
		//初始化md5的后32位为空
        ::memcpy( md5 + 32, bufMD5, 32 );
		//再把此md5重新生成一个新的md5
        GetMD5( md5, 64, bufMD5 );
	}

	delete []buffer;
    return bRe;
}


bool GenerateFileInfo(FILE* file, char bufMD5[32] )
{
	if( file == NULL )
		return false; 

	const unsigned long BLOCK_SIZE = 10 * 1024 * 1024;
	char *buffer = new char[BLOCK_SIZE];
	if(buffer == NULL) 
		return false;

	bufMD5[0] = '\0';
	buffer[0] = '\0';

	bool bRe = true;
	char md5[64 + 1] = {'\0'};
	unsigned long size = BLOCK_SIZE;

	fseek(file, 0, SEEK_SET);
	while(size == BLOCK_SIZE) 
	{
		size = fread(buffer, 1, BLOCK_SIZE, file);
		if(size == 0)
		{
			bRe = false;
			break;
		}

		GetMD5( buffer, size, md5 );
		::memcpy( md5 + 32, bufMD5, 32 );
		GetMD5( md5, 64, bufMD5 );
	}
	delete []buffer;
	return bRe;
}

//从文件生成md5码
bool GetMD5FromFile(const char* szFileName, char bufMD5[32])
{

	FILE* file=NULL;
	fopen_s(&file, szFileName, "rb");

	//如果文件不存在
	if(file == NULL)
		return false;

	memset(bufMD5, 0, 32);

	if(!GenerateFileInfo( file, bufMD5 ))
	{
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}