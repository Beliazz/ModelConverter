#ifndef EmptyNode_H__
#define EmptyNode_H__

#include "..\main.h"


class CEmptyNode : public ModelElement
{
public:
	CEmptyNode();

	virtual bool VLoad( KFbxNode* pNode, KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "EmptyNode"; }

	virtual TiXmlElement* xml();

private:
	virtual void Delete();

};

#endif
