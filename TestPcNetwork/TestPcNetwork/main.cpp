// TestPcNetwork.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "g_netsocket.h"
#include "g_cmdpacket.h"
#include "g_cmdpool.h"

#include "Object.h"
#include "Item.h"
#include "Matrix.h"
CMatrix g_matrix;
int _tmain(int argc, _TCHAR* argv[])
{
	CItem *item;
	item=(CItem*)g_matrix.CreateObject(OTYPE_ITEM);
	Sys_Log("id=%d",item->GetUniqueId());
	Sys_Log("total=%d",g_matrix.GetObjectCount());
	g_matrix.DestroyObject(item);
	Sys_Log("total=%d",g_matrix.GetObjectCount());
	getchar();
	return 0;
}

