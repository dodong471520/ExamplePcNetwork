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
	char strLine[MAX_CFG_LINE_LEN]="";
	std::string str_group="";
	while(!ifs.eof())
	{
		ifs.getline(strLine,MAX_CFG_LINE_LEN);
		if(strLine[0]=='['&&strLine[strlen(strLine)-1]==']')
		{
			strLine[strlen(strLine)-1]=0;
			str_group=strLine+1;
			continue;
		}
		if(!str_group.empty())
		{
			char seps[]="=\t\n";
			char* str_option=strtok(strLine,seps);
			if(str_option==NULL)
				continue;
			char* str_value=strtok(NULL,seps);
			if(str_value)
				m_map_group[str_group][str_option]=str_value;
			else
				m_map_group[str_group][str_option]="";
		}
	}
	ifs.close();
	return true;
}

void CFG::close()
{

}

bool CFG::save(const char *str_file)
{

	return true;
}

LPCSTR CFG::GetString( LPCSTR strSection, LPCSTR szName, LPCSTR szDefault )
{
	MAP_GROUP::iterator it=m_map_group.find(strSection);
	if(it!=m_map_group.end())
	{
		MAP_OPTION::iterator it_opt=it->second.find(szName);
		if(it_opt!=it->second.end())
			return it_opt->second.c_str();
	}
	return szDefault;
}

long CFG::GetLong( LPCSTR strSection, LPCSTR szName, LPCSTR szDefault )
{
	return strtol(GetString(strSection, szName, szDefault),TEXT('\0'),10);
}

float CFG::GetFloat( LPCSTR strSection, LPCSTR szName, LPCSTR szDefault )
{
	return (float)strtod(GetString(strSection,szName,szDefault),TEXT('\0'));
}

DWORD CFG::GetHex( LPCSTR strSection, LPCSTR szName, LPCSTR szDefault )
{
	DWORD dwResult=0;
	if(sscanf(GetString(strSection,szName,szDefault),TEXT("0x%x"),&dwResult)!=1)
		dwResult=0;
	return dwResult;
}