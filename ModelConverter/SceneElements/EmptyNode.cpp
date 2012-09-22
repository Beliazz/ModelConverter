#include "EmptyNode.h"


CEmptyNode::CEmptyNode()
{
}

bool CEmptyNode::VLoad( KFbxNode* pNode, KFbxScene* pScene  )
{
	if (!pNode)
		return false;


	//Process Children
	for(int i = 0; i < pNode->GetChildCount(); i++)
	{
		m_pChildren.push_back( ProcessFBXSceneElement(  pNode->GetChild(i), pScene ) );
	}

	return true;
}

bool CEmptyNode::VLoad( TiXmlElement* pScene )
{

	return true;
}

void CEmptyNode::Delete()
{

}

TiXmlElement* CEmptyNode::xml()
{
	TiXmlElement* xmlEmpty = new TiXmlElement( GetType().c_str() );

	return xmlEmpty; 
}
