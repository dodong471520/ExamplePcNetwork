// Object.cpp: implementation of the CObject class.

#include "Object.h"

CObject::CObject()
{

}

CObject::~CObject()
{

}

void CObject::HeartBeat()
{

}
void CObject::SetUniqueId( long id )
{
	m_uid=id;
}

long CObject::GetUniqueId()
{
	return m_uid;
}

long CObject::GetObjectType()
{
	return m_obType;
}

void CObject::SetObjectType( long type )
{
	m_obType=type;
}
