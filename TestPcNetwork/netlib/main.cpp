// netlib.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "cfg.h"

int _tmain(int argc, char* argv[])
{
	CFG cfg;
	cfg.open("game_server.ini");
	uint16 port=cfg.GetLong("main","port");

	return 0;
}

