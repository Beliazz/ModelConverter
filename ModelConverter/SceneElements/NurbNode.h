#ifndef NurbNode_H__
#define NurbNode_H__

#include "..\main.h"


class CNurbNode : public ModelElement
{
public:
	CNurbNode();

	virtual bool VLoad( KFbxNode* pNode, KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "NurbNode"; }
	virtual TiXmlElement* xml();

private:
	string m_sName;

	virtual void Delete();

};

#endif
