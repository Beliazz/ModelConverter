#ifndef MarkerNode_H__
#define MarkerNode_H__

#include "..\main.h"


class CMarkerNode : public ModelElement
{
public:
	CMarkerNode();

	virtual bool VLoad( KFbxNode* pNode, KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "MarkerNode"; }
	virtual TiXmlElement* xml();

private:
	string m_sName;
	string m_sType;
	string m_sLook;
	float  m_fSize;
	Color  m_Color;
	Vector m_IKPivot;

	virtual void Delete();

};

#endif
