#ifndef _FILE_H_
#define _FILE_H_

#include "yy_macros.h"
#include "yy_exception.h"

NS_YY_BEGIN

void MkFile(const char* pFilePath);
void RmFile(const char* pFilePath);
bool IsFileExist(const char* pFilePath);
bool IsFile(const char* pFilePath);
UI64 GetFileSize(const char* pFilePath);
void CopyFile(const char* pDst, const char* pSrc);

void MkDir(const char* pDirPath);
void RmDir(const char* pDirPath);
bool IsDir(const char* pDirPath);
bool IsDirExist(const char* pDirPath);
void CopyDir(const char* pDst, const char* pSrc);


NS_YY_END
#endif