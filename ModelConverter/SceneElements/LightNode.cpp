#include "LightNode.h"


CLightNode::CLightNode()
{
}

bool CLightNode::VLoad( KFbxNode* pNode, KFbxScene* pScene  )
{
	if (!pNode)
		return false;

	KFbxLight* lLight = (KFbxLight*) pNode->GetNodeAttribute();

	char* lLightTypes[] = { "Point", "Directional", "Spot" };

	m_sName = pNode->GetName();

	m_sType = lLightTypes[lLight->LightType.Get()];

	m_bCastLight = lLight->CastLight.Get();

	m_bHasGobo = false;

	if (!(lLight->FileName.Get().IsEmpty()))
	{
		m_bHasGobo = true;

		m_Gobo.m_sFilename = lLight->FileName.Get().Buffer();
		m_Gobo.m_bGroundProjection = lLight->DrawGroundProjection.Get();
		m_Gobo.m_bVolumetricProjection = lLight->DrawVolumetricLight.Get();
		m_Gobo.m_bFrontVolumetricProjection = lLight->DrawFrontFacingVolumetricLight.Get();
	}


	fbxDouble3 c = lLight->Color.Get();
	KFbxColor lColor(c[0], c[1], c[2]);

	m_DefaultColor = lColor;
	m_fDefaultIntensity = (float)lLight->Intensity.Get();
	m_fDefaultConeAngle = (float)lLight->ConeAngle.Get();
	m_fDefaultFog		= (float)lLight->Fog.Get();

	//Process Children
	for(int i = 0; i < pNode->GetChildCount(); i++)
	{
		m_pChildren.push_back( ProcessFBXSceneElement(  pNode->GetChild(i), pScene ) );
	}
	return true;
}

bool CLightNode::VLoad( TiXmlElement* pScene )
{


	return true;
}

void CLightNode::Delete()
{

}

TiXmlElement* CLightNode::xml()
{
	TiXmlElement* xmlLight = new TiXmlElement( GetType().c_str() );
	xmlLight->SetAttribute( "Name", m_sName.c_str() );
	xmlLight->SetAttribute( "Type", m_sType.c_str() );

	if (m_bCastLight)
		xmlLight->SetAttribute( "CastLight", "true" );
	else 
		xmlLight->SetAttribute( "CastLight", "false" );

	if (m_bHasGobo)
	{
		xmlLight->SetAttribute( "Gobo", "true" );
		xmlLight->LinkEndChild( m_Gobo.xml() );
	}
	else
		xmlLight->SetAttribute( "Gobo", "false" );


	//Default Animation Values
	TiXmlElement* xmlDefaultAnimation = new TiXmlElement( "Default-Animation-Values" );

	TiXmlElement* xmlColor = new TiXmlElement( "Color" );
	xmlColor->LinkEndChild( xmlText( m_DefaultColor ) );
	xmlDefaultAnimation->LinkEndChild( xmlColor );

	TiXmlElement* xmlDefaultIntensity = new TiXmlElement( "DefaultIntensity" );
	xmlDefaultIntensity->LinkEndChild( xmlText( m_fDefaultIntensity ) );
	xmlDefaultAnimation->LinkEndChild( xmlDefaultIntensity );

	TiXmlElement* xmlDefaultConeAngle = new TiXmlElement( "DefaultConeAngle" );
	xmlDefaultConeAngle->LinkEndChild( xmlText( m_fDefaultConeAngle ) );
	xmlDefaultAnimation->LinkEndChild( xmlDefaultConeAngle );

	TiXmlElement* xmlDefaultFog = new TiXmlElement( "DefaultFog" );
	xmlDefaultFog->LinkEndChild( xmlText( m_fDefaultFog ) );
	xmlDefaultAnimation->LinkEndChild( xmlDefaultFog );

	xmlLight->LinkEndChild( xmlDefaultAnimation );

	return xmlLight;
}

TiXmlElement* sGobo::xml()
{
	TiXmlElement* xmlGobo = new TiXmlElement( "Gobo" );
	xmlGobo->SetAttribute( "Filename", m_sFilename.c_str() );

	if (m_bGroundProjection)
		xmlGobo->SetAttribute( "DrawGroundProjection", "true" );
	else
		xmlGobo->SetAttribute( "DrawGroundProjection", "false" );

	if (m_bVolumetricProjection)
		xmlGobo->SetAttribute( "VolumetricProjection", "true" );
	else
		xmlGobo->SetAttribute( "VolumetricProjection", "false" );

	if (m_bFrontVolumetricProjection)
		xmlGobo->SetAttribute( "FrontVolumetricProjection", "true" );
	else
		xmlGobo->SetAttribute( "FrontVolumetricProjection", "false" );

	return xmlGobo;
}
