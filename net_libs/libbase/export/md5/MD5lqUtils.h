// MD5lqUtils.h

#ifndef LQINC_MD5LQUTILS_H
#define LQINC_MD5LQUTILS_H

/************************************************************************/
/*
md5码对外的接口,考虑到大文件，所以分块读，并把上一块的MD5码重新编译进去。
这样，导致算出来的MD5码和通用的MD5码工具生成不一样，但是比1991年的那个老的
MD5类生成的快

string test = "a";
char md5Buf[33]={0};
md5Buf[33]='\0';
bool bRet = GenerateMemInfo(test.c_str(), test.length(), md5Buf);

string filename = "E:\\game\\9yin1.exe";
char md5FileBuf[32]={"\0"};
bool bRet = GetMD5FromFile(filename.c_str(), md5FileBuf);
*/
/************************************************************************/


// 根据内存算MD5码
bool GenerateMemInfo( const char* lpMem, unsigned long dwSize, char bufMD5[32] );

//从文件生成md5码
bool GetMD5FromFile(const char* szFileName, char bufMD5[32]);

#endif//LQINC_MD5LQUTILS_H
