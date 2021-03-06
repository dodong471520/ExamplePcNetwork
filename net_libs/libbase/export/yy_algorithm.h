/************************************************************************/
/*
@author:  junliang
@brief:   一些算法的简单实现, 不用模版，不考虑通用性
          以后优化的时候，可以把某些stl重新自己实现
@time:    20130327
*/
/************************************************************************/
#ifndef YY_ALGORIGTHM_H_
#define YY_ALGORIGTHM_H_

#include <assert.h>
/************************************************************************/
/*
@brief:  链表
*/
/************************************************************************/
struct ListNode
{
	int elem;
	ListNode* next;
	ListNode* prev;
};
/************************************************************************/
/*
@brief:  二叉树 binary tree
*/
/************************************************************************/
struct BinNode
{
	int elem;
	BinNode* left;
	BinNode* right;
	BinNode* parent;
};
/************************************************************************/
/*
@brief:  哈希函数, 把一串字符串转换成散列值， stl hash map, 查找复杂度O(1)
         参考quake3
         测试发现，VAR_SIZE和插入的数据量，差10倍以上，冲突的比例会比较好
*/
/************************************************************************/
#define VAR_SIZE 1024

struct cvar_t
{
	char key[100];
	char value[100];
	cvar_t* hash_next;
	cvar_t* next;
};

static long generateHashValue( const char *fname )
{
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0')
	{
		letter = tolower(fname[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}

	hash &= (VAR_SIZE-1);
	return hash;
}

class HashMap
{
public:
    HashMap()
	{
		m_capacity = VAR_SIZE;
		int n=sizeof(m_table);
		memset(m_table, 0, sizeof(m_table));
		m_nodes = new cvar_t[m_capacity];
		m_size = 0;
		m_node_index=0;
		m_node_head=NULL;
	}

    ~HashMap()
	{
		delete []m_nodes;
	}

	cvar_t *get( const char *key)
	{
		assert(strlen(key) < 100);

		long hash;
		cvar_t* kn=find(key);
		if(kn)
			return kn;

		//找不到，则新建
		assert(m_node_index < m_capacity);
		cvar_t* kn_new=&m_nodes[m_node_index++];
		strcpy(kn_new->key, key);

		//新节点插入数组
		hash = generateHashValue(key);
		kn_new->hash_next=m_table[hash];
		m_table[hash]=kn_new;

		//插入链表
		kn_new->next=m_node_head;
		m_node_head=kn_new;

		return kn_new;
	}

    bool isExist(const char* key)
	{
		if(NULL ==find(key))
			return false;

		return true;
	}

	//用hashmap就是为了快速查找海量数据，所以不应该支持remove
	//bool remove(const std::string& key){return false;}


private:
	cvar_t* find(const char* key)
	{
		long hash = generateHashValue(key);
		cvar_t* kn=NULL;
		for(kn=m_table[hash]; kn; kn=kn->hash_next)
		{
			if(strcmp(kn->key, key)==0)
				return kn;
		}

		return NULL;
	}
public:
    cvar_t *m_table[VAR_SIZE];		//std::string->index
	cvar_t *m_nodes;				//新节点从此数组获得新空间
	cvar_t* m_node_head;			//链表头,通过此节点，可以遍历所有在使用的节点
    int m_size;
    int m_capacity;
	int m_node_index;
};


/************************************************************************/
/*
@brief:  平衡二叉树
*/
/************************************************************************/


/************************************************************************/
/*
@brief:  红黑树(自平衡二叉树)
         stl set, multiset, map, multimap, 查找复杂度O(log n)
*/
/************************************************************************/


#endif