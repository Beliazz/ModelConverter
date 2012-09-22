#include "MarkerNode.h"


CMarkerNode::CMarkerNode()
{
}

bool CMarkerNode::VLoad( KFbxNode* pNode, KFbxScene* pScene )
{
	if (!pNode)
		return false;


	KFbxMarker* lMarker = (KFbxMarker*)pNode->GetNodeAttribute();
	KString     lString;

	m_sName =  pNode->GetName();
	
	// Type
	switch (lMarker->GetType())
	{
	case KFbxMarker::eSTANDARD:    m_sType += "Standard ";    break;
	case KFbxMarker::eOPTICAL:     m_sType += "Optical ";     break;
	case KFbxMarker::eIK_EFFECTOR: m_sType += "IK Effector "; break;
	case KFbxMarker::eFK_EFFECTOR: m_sType += "FK Effector "; break;
	}
	

	// Look
	switch (lMarker->Look.Get())
	{
	case KFbxMarker::eCUBE:         m_sLook += "Cube";        break;
	case KFbxMarker::eHARD_CROSS:   m_sLook += "Hard Cross";  break;
	case KFbxMarker::eLIGHT_CROSS:  m_sLook += "Light Cross"; break;
	case KFbxMarker::eSPHERE:       m_sLook += "Sphere";      break;
	}

	// Size
	m_fSize = (float)lMarker->Size.Get();

	// Color
	fbxDouble3 c = lMarker->Color.Get();
	KFbxColor color(c[0], c[1], c[2]);
	m_Color = Color(color);

	// IKPivot
	m_IKPivot = Vector(lMarker->IKPivot.Get().mData);

	//Process Children
	for(int i = 0; i < pNode->GetChildCount(); i++)
	{
		m_pChildren.push_back( ProcessFBXSceneElement(  pNode->GetChild(i), pScene ) );
	}

	return true;
}

bool CMarkerNode::VLoad( TiXmlElement* pScene )
{

	return true;
}

void CMarkerNode::Delete()
{

}

TiXmlElement* CMarkerNode::xml()
{
	TiXmlElement* xmlMarker = new TiXmlElement( GetType().c_str() );
	xmlMarker->SetAttribute( "Name", m_sName.c_str() );
	xmlMarker->SetAttribute( "Type", m_sType.c_str() );
	xmlMarker->SetAttribute( "Look", m_sLook.c_str() );

	stringstream stm;
	stm << m_fSize;

	xmlMarker->SetAttribute( "Size", stm.str().c_str() );

	TiXmlElement* xmlColor = new TiXmlElement( "Color" );
	xmlColor->LinkEndChild( xmlText(m_Color) );
	xmlMarker->LinkEndChild( xmlColor );

	TiXmlElement* xmlIKPivot = new TiXmlElement( "IKPivot" );
	xmlIKPivot->LinkEndChild( xmlText(m_IKPivot.str()) );
	xmlMarker->LinkEndChild( xmlIKPivot );

	return xmlMarker;
}
