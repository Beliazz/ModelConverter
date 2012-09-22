#include "NurbNode.h"


CNurbNode::CNurbNode()
{
}

bool CNurbNode::VLoad( KFbxNode* pNode, KFbxScene* pScene  )
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

bool CNurbNode::VLoad( TiXmlElement* pScene )
{

	return true;
}


void CNurbNode::Delete()
{

}

TiXmlElement* CNurbNode::xml()
{
	TiXmlElement* xmlNurb = new TiXmlElement( GetType().c_str() );
	xmlNurb->SetAttribute( "Name", m_sName.c_str() );
	xmlNurb->SetAttribute( "Info", "Unsupported" );

	return xmlNurb;
}
