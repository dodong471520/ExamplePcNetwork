#include "net_lib.h"
#include "cfg.h"

CFG::CFG()
{

}

CFG::~CFG()
{

}

bool CFG::open( const char* str_file )
{
	std::ifstream ifs(str_file);
	if(!ifs)
		return false;
	char strLine[MAX_CFG_LINE_LEN];
	std::string str_group="";
	while(!ifs.eof())
	{
		ifs.getline(strLine,MAX_CFG_LINE_LEN);
		if(strLine[0]=='['&&strLine[strlen()])
	}
	ifs.close();
	return true;
}

