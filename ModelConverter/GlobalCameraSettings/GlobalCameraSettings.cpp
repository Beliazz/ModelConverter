#include "GlobalCameraSettings.h"


CGlobalCameraSettings::CGlobalCameraSettings()
{
	m_sDefaultCamera = "UNKOWN";
}

bool CGlobalCameraSettings::VLoad( KFbxScene* pScene )
{
	if (!pScene)
		return false;

	m_sDefaultCamera = pScene->GetGlobalSettings().GetDefaultCamera();

	return true;
}

bool CGlobalCameraSettings::VLoad( TiXmlElement* pScene )
{

	return true;
}

void CGlobalCameraSettings::Delete()
{

}

std::string CGlobalCameraSettings::str()
{
	stringstream stm;

	stm << m_sDefaultCamera << endl;

	return stm.str();
}

TiXmlElement* CGlobalCameraSettings::xml()
{
	TiXmlElement* xmlCameraSettings = new TiXmlElement( GetType().c_str() );

	TiXmlElement* xmlDefaultCamera = new TiXmlElement("DefaultCamera");
	xmlDefaultCamera->LinkEndChild( xmlText( m_sDefaultCamera ) );
	xmlCameraSettings->LinkEndChild( xmlDefaultCamera );

	return xmlCameraSettings;
}

void CGlobalCameraSettings::bin( IOHelper* pWriter )
{
	pWriter->write( m_sDefaultCamera.c_str(), 256 );
}
