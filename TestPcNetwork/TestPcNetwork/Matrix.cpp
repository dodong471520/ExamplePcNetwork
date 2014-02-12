#include "Matrix.h"
#include "Object.h"
#include "Item.h"
#include "Human.h"

CMatrix::CMatrix()
{
	// ³õÊ¼ID = 1
	m_seq = 1;

}

CMatrix::~CMatrix()
{

}

CObject * CMatrix::CreateObject( long type )
{
	CObject *ob=NULL;
	switch(type)
	{
	case OTYPE_ITEM:
		ob=new CItem;
		break;
	case OTYPE_HUMAN:
		ob=new CHuman;
		break;
	}
	if(ob)
	{
		ob->SetUniqueId(m_seq++);
		m_obList.push_back(ob);
	}
	return ob;
}

void CMatrix::DestroyObject( CObject *ob )
{
	m_obList.remove(ob);
	delete ob;
}

int CMatrix::GetObjectCount()
{
	return m_obList.size();
}

CObject * CMatrix::FindObject( long uid )
{
	CObject *ob=NULL;
	list<CObject*>::iterator it;
	for(it=m_obList.begin();it!=m_obList.end();++it)
	{
		if((*it)->GetUniqueId()==uid)
		{
			ob=*it;
			break;
		}
	}
	return ob;
}

