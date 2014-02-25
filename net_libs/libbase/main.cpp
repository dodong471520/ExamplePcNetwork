#include "yy_log.h"
#include "yy_algorithm.h"
#include "yy_utility.h"
#include "yy_bstree.h"
#include <map>
USING_NS_YY


int test(int n)
{
	if(n==1)
		return 1;

	return n*test(n-1);
}

int main()
{

	int tttt=test(5);
	int aa=1;
	int bb=2;
	int c=aa^bb;
	bb=bb^c;
	aa=c^bb;
	
std::map<int, int> test_map;
test_map.insert(std::pair<int, int>(1,1));




	CBsTree<int, int> bs_tree;

	for(int i=0; i<10000; i++)
	{
		int t=rand();
		bs_tree.insert(t, t);
	}




	bool b=CheckBigendien();

	int a=sizeof(unsigned short);
	a=sizeof(unsigned long);
	a=sizeof(unsigned int);

	HashMap mp;

	for(int i=0; i<VAR_SIZE/10; i++)
	{
		char buf[100]={0};
		int r=rand();
		sprintf(buf, "abc %d dd123 %d", i, r);
		cvar_t* var=mp.get(buf);
		strcpy(var->value, "haha");
	}

	int no_chongtu=0;
	int chongtu=0;
	int not_inuse=0;
	for(int i=0; i<VAR_SIZE; i++)
	{
		cvar_t* kn=mp.m_table[i];
		if(!kn)
		{
			not_inuse++;
			LOG_INFO("index:%d not in use0000000000000000000000000000000000000", i);
			continue;
		}

		if(!kn->hash_next)
		{
			no_chongtu++;
			LOG_INFO("index:%d, key: %s, value:%s111111111111111111111111", i, kn->key, kn->value);
			continue;
		}

		while(kn)
		{
			chongtu++;
			LOG_INFO("index:%d,ÓÐ³åÍ»µÄkey, key: %s, value:%s", i, kn->key, kn->value);
			kn=kn->hash_next;
		}
	}

	LOG_INFO("chongtu:%d, weichongtu:%d, kong:%d", chongtu, no_chongtu, not_inuse);

	int i;
	scanf_s("%d", &i);
	return 0;

}