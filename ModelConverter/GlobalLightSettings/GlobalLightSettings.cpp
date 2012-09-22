#include "GlobalLightSettings.h"


CGlobalLightSettings::CGlobalLightSettings()
{
	m_colAmbient = Color(1.0f,1.0f,1.0f,1.0f);
}

bool CGlobalLightSettings::VLoad( KFbxScene* pScene )
{
	if (!pScene)
		return false;

	m_colAmbient = Color(pScene->GetGlobalSettings().GetAmbientColor());

	return true;
}

bool CGlobalLightSettings::VLoad( TiXmlElement*  )
{
	return true;
}

void CGlobalLightSettings::Delete()
{

}

std::string CGlobalLightSettings::str()
{
	stringstream stm;

	stm << m_colAmbient.str() << endl;

	return stm.str();
}

TiXmlElement* CGlobalLightSettings::xml()
{
	TiXmlElement* xmlLightSettings = new TiXmlElement( GetType().c_str() );

	TiXmlElement* xmlAmbientColor = new TiXmlElement("AmbientColor");
	xmlAmbientColor->LinkEndChild( xmlText( m_colAmbient ) );
	xmlLightSettings->LinkEndChild( xmlAmbientColor );

	return xmlLightSettings;
}

void CGlobalLightSettings::bin( IOHelper* pWriter )
{
	pWriter->write( m_colAmbient );
}
