#include "PatchNode.h"


CPatchNode::CPatchNode()
{
}

bool CPatchNode::VLoad( KFbxNode* pNode, KFbxScene* pScene )
{
	if (!pNode)
		return false;

	m_sName = pNode->GetName();


	//Process Children
	for(int i = 0; i < pNode->GetChildCount(); i++)
	{
		m_pChildren.push_back( ProcessFBXSceneElement(  pNode->GetChild(i), pScene ) );
	}

	return true;
}

bool CPatchNode::VLoad( TiXmlElement* pScene )
{

	return true;
}

void CPatchNode::Delete()
{

}

TiXmlElement* CPatchNode::xml()
{
	TiXmlElement* xmlPatch = new TiXmlElement( GetType().c_str() );
	xmlPatch->SetAttribute( "Name", m_sName.c_str() );
	xmlPatch->SetAttribute( "Info", "Unsupported" );

	return xmlPatch;
}
