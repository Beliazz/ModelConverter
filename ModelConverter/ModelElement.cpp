#include "main.h"
#include "ModelElement.h"

RootElement::RootElement()
{

}

void RootElement::Delete()
{

}

bool RootElement::VSave( TiXmlElement* parent )
{
	for (unsigned int i = 0; i < m_pChildren.size() ; i++)
	{
		if( !m_pChildren[i]->VSave( parent ) )
			return false;
	}

	TiXmlElement* xmlScripts = new TiXmlElement("Scripts");
	TiXmlElement* xmlOnUpdate = new TiXmlElement("OnUpdate");
	xmlOnUpdate->SetAttribute("AssignedTo","Update");

	stringstream exampleUpdateScript;

	exampleUpdateScript<<"\nprint('Update from Model Script')\n";
	string strScript = exampleUpdateScript.str();
	TiXmlText* text = new TiXmlText(strScript.c_str());
	text->SetCDATA(true);

	xmlOnUpdate->LinkEndChild(text);


	xmlScripts->LinkEndChild(xmlOnUpdate);
	parent->LinkEndChild(xmlScripts);

	return true;
}

bool RootElement::VSave( IOHelper* pWriter)
{
	//Number of childs
	pWriter->write( (DWORD)m_pChildren.size() );

	for (unsigned int i = 0; i < m_pChildren.size() ; i++)
	{
		if( !m_pChildren[i]->VSave( pWriter ) )
			return false;
	}

	return true;
}



ModelElement::~ModelElement()
{
	Delete();
}

bool ModelElement::VSave( TiXmlElement* parent )
{
	TiXmlElement* xmlVariable = xml();

	for (unsigned int i = 0; i < m_pChildren.size() ; i++)
	{
		m_pChildren[i]->VSave( xmlVariable );
	}

	parent->LinkEndChild( xmlVariable );

	return true;
}

bool ModelElement::VSave( IOHelper* pWriter )
{
	//Type
	pWriter->write(  GetType().c_str(), 256 );

	//Main Data
	bin(pWriter);

	//Number of childs
	pWriter->write( (DWORD)m_pChildren.size() );

	for (unsigned int i = 0; i < m_pChildren.size() ; i++)
	{
		m_pChildren[i]->VSave( pWriter );
	}

	return true;
}
