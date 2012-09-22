#ifndef LODGroudNode_H__
#define LODGroudNode_H__

#include "..\main.h"


class CLODGroudNode : public ModelElement
{
public:
	CLODGroudNode();

	virtual bool VLoad( KFbxNode* pNode );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "LODGroudNode"; }

private:
	virtual void Delete();

};

#endif
