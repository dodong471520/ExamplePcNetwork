//==============================================================================
// 文件名称: MD5.cpp
// 
// Version 1.0
// 
// 修 改 人: 
// 修改日期: 
// 修改原因: 
// 描    述: 
//==============================================================================
//#include "stdafx.h"
#include "MD5lq.h"

#include <stdio.h>
#include <string>

// MD5 Context. 
typedef struct tagMD5CTX 
{
	unsigned long state[4];								// State(ABCD) 
	unsigned long count[2];								// Number Of Bits, Modulo 2^64 (lsb first) */
	unsigned char buffer[64];					// Input Buffer 
}MD5CTX, *LPMD5CTX;

void MD5Init(LPMD5CTX, unsigned long ulPara1, unsigned long ulPara2, unsigned long ulPara3, unsigned long ulPara4 );
void MD5Update(LPMD5CTX ,unsigned char *, unsigned long);	//MD5 Block Update Operation
void MD5Final(unsigned char[16], LPMD5CTX);	
void MD5Print (unsigned char digest[16],unsigned char* pbyResult);

void MD5Transform(unsigned long [4], unsigned char [64]);
void Encode(unsigned char *, unsigned long *, unsigned long);
void Decode(unsigned long *, unsigned char *, unsigned long);
void MD5MemCpy(unsigned char*, unsigned char*, unsigned long);
void MD5MemSet(unsigned char*, long, unsigned long);

// Constants For MD5 Transform Routine.
#define S11				7
#define S12				12
#define S13				17
#define S14				22
#define S21				5
#define S22				9
#define S23				14
#define S24				20
#define S31				4
#define S32				11
#define S33				16
#define S34				23
#define S41				6
#define S42				10
#define S43				15
#define S44				21

static unsigned char PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//F, G, H and I are basic MD5 functions.
#define F(x, y, z)		(((x) & (y)) | ((~x) & (z)))
#define G(x, y, z)		(((x) & (z)) | ((y) & (~z)))
#define H(x, y, z)		((x) ^ (y) ^ (z))
#define I(x, y, z)		((y) ^ ((x) | (~z)))

//ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x,n) (((x) << (n)) | ((x) >> (32-(n))))

//FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
//Rotation is separate from addition to prevent recomputation.

#define FF(a, b, c, d, x, s, ac) {					\
	(a) += F ((b), (c), (d)) + (x) + (unsigned long)(ac);   \
	(a) = ROTATE_LEFT ((a), (s));					\
	(a) += (b);										\
	}

#define GG(a, b, c, d, x, s, ac) {					\
	(a) += G ((b), (c), (d)) + (x) + (unsigned long)(ac);   \
	(a) = ROTATE_LEFT ((a), (s));					\
	(a) += (b);										\
	}

#define HH(a, b, c, d, x, s, ac) {					\
	(a) += H ((b), (c), (d)) + (x) + (unsigned long)(ac);		\
	(a) = ROTATE_LEFT ((a), (s));						\
	(a) += (b);										\
	}

#define II(a, b, c, d, x, s, ac) {					\
	(a) += I ((b), (c), (d)) + (x) + (unsigned long)(ac);		\
	(a) = ROTATE_LEFT ((a), (s));						\
	(a) += (b);										\
	}

void MD5Init (LPMD5CTX context, unsigned long ulPara1, unsigned long ulPara2, 
			  unsigned long ulPara3, unsigned long ulPara4 )
{
	context->count[0] = context->count[1] = 0;

	//Initialize The Four Constants
	context->state[0] = ulPara1; 
	context->state[1] = ulPara2; 
	context->state[2] = ulPara3; 
	context->state[3] = ulPara4; 
}

void MD5Update (LPMD5CTX context, unsigned char *input, unsigned long inputLen)
{
	unsigned long i, index, partLen;

	//Compute number of bytes mod 64 
	index = (unsigned long)((context->count[0] >> 3) & 0x3F);

	//Update number of bits
	if ((context->count[0] += ((unsigned long)inputLen << 3)) < ((unsigned long)inputLen << 3))
	{
		context->count[1]++;
	}
	context->count[1] += ((unsigned long)inputLen >> 29);

	partLen = 64 - index;

	// Transform as many times as possible.

	if (inputLen >= partLen) 
	{
		MD5MemCpy((unsigned char*)&context->buffer[index], (unsigned char*)input, partLen);
		MD5Transform (context->state, context->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
		{
			MD5Transform (context->state, &input[i]);
		}

		index = 0;
	}

	else
	{
		i = 0;
	}

	/* Buffer Remaining input */
	MD5MemCpy((unsigned char*)&context->buffer[index], (unsigned char*)&input[i],inputLen-i);
}

void MD5Final (unsigned char* digest, LPMD5CTX context)
{
	unsigned char bits[8];
	unsigned long index, padLen;

	// Save number of bits */
	Encode (bits, context->count, 8);

	// Pad out to 56 mod 64.
	index = (unsigned long)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);

	MD5Update (context, PADDING, padLen);

	// Append length (before padding) 
	MD5Update (context, bits, 8);

	//Store state in digest */
	Encode (digest, context->state, 16);

	//Zeroize sensitive information.
	MD5MemSet ((unsigned char*)context, 0, sizeof (*context));
}

void MD5Transform (unsigned long state[4], unsigned char block[64])
{

	unsigned long a = state[0];
	unsigned long b = state[1];
	unsigned long c = state[2]; 
	unsigned long d = state[3]; 
	unsigned long x[16];

	Decode (x, block, 64);

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	//Zeroize sensitive information.
	MD5MemSet ((unsigned char*)x, 0, sizeof (x));
}

void Encode (unsigned char *output, unsigned long *input, unsigned long len)
{
	unsigned long i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		output[j]   = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

void Decode (unsigned long *output, unsigned char* input, unsigned long len)
{
	unsigned long i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		output[i] = ((unsigned long)input[j]) | (((unsigned long)input[j+1]) << 8) |
			(((unsigned long)input[j+2]) << 16) | (((unsigned long)input[j+3]) << 24);
	}
}

void MD5MemCpy (unsigned char* output, unsigned char* input, unsigned long len)
{	
	for (unsigned long i = 0; i < len; i++)
	{
		output[i] = input[i];
	}
}

void MD5MemSet (unsigned char* output, long value,unsigned long len)
{
	for (unsigned long i = 0; i < len; i++)
	{
		((char *)output)[i] = (char)value;
	}
}

void MD5Print (unsigned char digest[16],unsigned char* pbyResult)
{
	long i;

	for (i = 0; i < 16; i++)
	{
		sprintf_s((char*)pbyResult, sizeof(pbyResult), "%02X", (unsigned char)digest[i]);
		pbyResult+=2;
	}
}

//==============================================================================
// 类    属: 
// 函数名称: MD5
// 参    数: LPCSTR lpszSource - 源字符串
//           int    nParam1 - 常数1
//           int    nParam2 - 常数2
//           int    nParam3 - 常数3
//           int    nParam4 - 常数4
//           LPSTR  lpszTarget - [out] 目标字符串
//           int    nSize - 目标字符串数组的大小
//			 bool   nParamType - 使用输入常数参数与否 true使用
// 返回类型: bool
// 功能描述: MD5
// 
// 全局变量: 
// 调用模块: 
// 备    注: static
// 
// 创 建 人: Tom Liu
// 创建日期: 2002/06/28
// 修 改 人: Tom Liu
// 修改日期: 2005/5/16
// 修改描述: 增加参数 bool   nParamType - 使用输入常数参数与否 true使用
//==============================================================================

bool MD5 ( const char* lpszSource, char* lpszTarget, long nSize,
		  bool bParamType,long &nParam1, long &nParam2,long &nParam3, long &nParam4 )
{
	MD5CTX context;
	unsigned char byDigest[16+1];
	//unsigned char byResults[32+1];
	//updated by junliang,赋值的时候，+2，有溢出错误。
	unsigned char byResults[32+5];

	if (!bParamType)
	{
		nParam1 = 0X67452301;
		nParam2 = 0XEFCDAB89;
		nParam3 = 0X98BADCFE;
		nParam4 = 0X10325476;
	}

	MD5Init ( &context, (unsigned long)nParam1, (unsigned long)nParam2, (unsigned long)nParam3, (unsigned long)nParam4 );
	MD5Update ( &context, (unsigned char*)lpszSource, nSize);//lstrlen(lpszSource) );
	MD5Final ( byDigest, &context );
	MD5Print ( byDigest, byResults );

	::memcpy( lpszTarget, byResults, 32 );
	return true;
} // MD5() end
