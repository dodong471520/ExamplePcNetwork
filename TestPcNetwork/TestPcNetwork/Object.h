#ifndef __COBJECT_H__
#define __COBJECT_H__

class CObject  
{
public:
	long GetObjectType();
	void SetObjectType(long type);
	void SetUniqueId(long id);
	long GetUniqueId();

	virtual void HeartBeat();
	CObject();
	virtual ~CObject();

private:
	long m_uid;			// ����ID
	long m_obType;		// ��������

};

#endif