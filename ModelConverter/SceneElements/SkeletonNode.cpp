#include "SkeletonNode.h"


CSkeletonNode::CSkeletonNode()
{
}

bool CSkeletonNode::VLoad( KFbxNode* pNode, KFbxScene* pScene )
{
	if (!pNode)
		return false;

	KFbxSkeleton* lSkeleton = (KFbxSkeleton*) pNode->GetNodeAttribute();

	m_sName =  pNode->GetName();

	char* lSkeletonTypes[] = { "Root", "Limb", "Limb Node", "Effector" };

	m_sType = lSkeletonTypes[lSkeleton->GetSkeletonType()];


	if (lSkeleton->GetSkeletonType() == KFbxSkeleton::eLIMB)
	{
		m_fLimbLengh = (float)lSkeleton->LimbLength.Get();
	}
	else if (lSkeleton->GetSkeletonType() == KFbxSkeleton::eLIMB_NODE)
	{
		m_fLimbSize = (float)lSkeleton->Size.Get();
	}
	else if (lSkeleton->GetSkeletonType() == KFbxSkeleton::eROOT)
	{
		m_fRootSize = (float)lSkeleton->Size.Get();
	}

	m_Color = lSkeleton->GetLimbNodeColor();

	//Process Children
	for(int i = 0; i < pNode->GetChildCount(); i++)
	{
		m_pChildren.push_back( ProcessFBXSceneElement(  pNode->GetChild(i), pScene ) );
	}

	return true;
}

bool CSkeletonNode::VLoad( TiXmlElement* pScene )
{

	return true;
}

void CSkeletonNode::Delete()
{

}

TiXmlElement* CSkeletonNode::xml()
{
	TiXmlElement* xmlSkeleton = new TiXmlElement( GetType().c_str() );
	xmlSkeleton->SetAttribute( "Name", m_sName.c_str() );
	xmlSkeleton->SetAttribute( "Type", m_sType.c_str() );
	
	if (m_sType == "Limb")
	{
		stringstream stm;
		stm << m_fLimbLengh;

		xmlSkeleton->SetAttribute( "LimbLengh", stm.str().c_str() );
	}
	else if (m_sType == "Limb Node")
	{
		stringstream stm;
		stm << m_fLimbSize;

		xmlSkeleton->SetAttribute( "LimbSize", stm.str().c_str() );
	}
	else if (m_sType == "Root")
	{
		stringstream stm;
		stm << m_fRootSize;

		xmlSkeleton->SetAttribute( "RootSize", stm.str().c_str() );
	}

	TiXmlElement* xmlColor = new TiXmlElement( "Color" );
	xmlColor->LinkEndChild( xmlText(m_Color) );

	xmlSkeleton->LinkEndChild( xmlColor );

	return xmlSkeleton;
}
