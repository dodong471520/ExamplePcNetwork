#ifndef __CG_CMD_POOL_H__
#define __CG_CMD_POOL_H__
// use stl
#include <string>
#include <vector>
using namespace std;

struct SCmd
{
	void   *fun;
	int		num;
	string	str;
	string	desc;
};

class CG_CmdPool
{
public:
	int   GetCmdCount();
	bool  AddCmd(SCmd *cmd);
	SCmd *GetCmdByNum(int num);
	SCmd *GetCmdByStr(char *str);

	CG_CmdPool();
	virtual ~CG_CmdPool();

private:
	vector<SCmd> m_cmdList;

};

#endif