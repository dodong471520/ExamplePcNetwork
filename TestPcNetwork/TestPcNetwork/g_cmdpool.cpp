// g_cmdpool.cpp: implementation of the CG_CmdPool class.

#include "g_cmdpool.h"

CG_CmdPool::CG_CmdPool()
{

}

CG_CmdPool::~CG_CmdPool()
{

}

bool CG_CmdPool::AddCmd(SCmd *cmd)
{
	m_cmdList.push_back(*cmd);
	return true;
}

SCmd *CG_CmdPool::GetCmdByStr(char *str)
{
	for(int i=0;i<m_cmdList.size();i++)
	{
		if(m_cmdList[i].str==str) return &m_cmdList[i]; 
	}
	return NULL;
}

SCmd *CG_CmdPool::GetCmdByNum(int num)
{
	for(int i=0;i<m_cmdList.size();i++)
	{
		if (m_cmdList[i].num==num) return &m_cmdList[i];
	}
	return NULL;
}

int CG_CmdPool::GetCmdCount()
{
	return m_cmdList.size();
}