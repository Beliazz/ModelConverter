#include "GlobalTimeSettings.h"

CGlobalTimeSettings::CGlobalTimeSettings()
{
	m_sTimeMode = "30 fps";
	m_fStart    = 0.0f;
	m_fEnd		= 0.0f;
}

bool CGlobalTimeSettings::VLoad( KFbxScene* pScene )
{
	if (!pScene)
		return false;

	KTimeSpan lTs;
	pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTs);

	m_sTimeMode = KTime_GetTimeModeName( pScene->GetGlobalSettings().GetTimeMode() );
				  
	m_fStart = (float)lTs.GetStart().GetSecondDouble();
	m_fEnd	 = (float)lTs.GetStop().GetSecondDouble();

	return true;
}

bool CGlobalTimeSettings::VLoad( TiXmlElement*  )
{
	return true;
}

void CGlobalTimeSettings::Delete()
{

}

std::string CGlobalTimeSettings::str()
{
	stringstream stm;

	stm << m_sTimeMode << endl;
	stm << m_fStart << endl;
	stm << m_fEnd;

	return stm.str();
}

TiXmlElement* CGlobalTimeSettings::xml()
{
	TiXmlElement* xmlTimeSettings = new TiXmlElement( GetType().c_str() );
	xmlTimeSettings->SetAttribute("Mode", m_sTimeMode.c_str() );
	
	TiXmlElement* xmlStartTime = new TiXmlElement("Start");
	xmlStartTime->SetAttribute( "Format", "Seconds" );
	xmlStartTime->LinkEndChild( xmlText(m_fStart) );
	xmlTimeSettings->LinkEndChild( xmlStartTime );

	TiXmlElement* xmlEndTime = new TiXmlElement("End");
	xmlEndTime->SetAttribute( "Format", "Seconds" );
	xmlEndTime->LinkEndChild( xmlText(m_fEnd) );
	xmlTimeSettings->LinkEndChild( xmlEndTime );

	return xmlTimeSettings;
}

void CGlobalTimeSettings::bin( IOHelper* pWriter )
{
	//Time Mode
	pWriter->write( m_sTimeMode.c_str(), 32 );

	//Start Time
	pWriter->write( m_fStart );

	//End Time
	pWriter->write( m_fEnd );
}
