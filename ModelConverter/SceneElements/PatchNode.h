#ifndef PatchNode_H__
#define PatchNode_H__

#include "..\main.h"


class CPatchNode : public ModelElement
{
public:
	CPatchNode();

	virtual bool VLoad( KFbxNode* pNode, KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "PatchNode"; }
	virtual TiXmlElement* xml();

private:
	string m_sName;

	virtual void Delete();

};

#endif
