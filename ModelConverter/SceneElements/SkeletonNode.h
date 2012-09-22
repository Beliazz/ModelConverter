#ifndef SkeletonNode_H__
#define SkeletonNode_H__

#include "..\main.h"


class CSkeletonNode : public ModelElement
{
public:
	CSkeletonNode();

	virtual bool VLoad( KFbxNode* pNode, KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "SkeletonNode"; }
	virtual TiXmlElement* xml();

private:
	string m_sName;
	string m_sType; 
	float  m_fLimbLengh;
	float  m_fLimbSize;
	float  m_fRootSize;
	Color  m_Color;

	virtual void Delete();

};

#endif
