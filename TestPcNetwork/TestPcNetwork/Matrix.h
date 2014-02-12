// Matrix.h: interface for the CMatrix class.

#ifndef __CMATRIX_H__
#define __CMATRIX_H__

#include <list>
using namespace std;

const long OTYPE_ITEM	= 1;
const long OTYPE_HUMAN  = 2;

class CObject;
class CMatrix  
{
public:
	int GetObjectCount();
	CObject * FindObject(long uid);
	void DestroyObject(CObject *ob);
	CObject * CreateObject(long type);
	CMatrix();
	virtual ~CMatrix();

private:
	list<CObject*> m_obList;
	long m_seq;
};

#endif 