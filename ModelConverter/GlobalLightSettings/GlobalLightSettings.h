#ifndef GlobalLightSettings_H__
#define GlobalLightSettings_H__

#include "..\main.h"

class CGlobalLightSettings : public ModelElement
{
public:
	CGlobalLightSettings();

	virtual bool VLoad( KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "GlobalLightSettings"; }
	virtual string str();
	virtual TiXmlElement* xml();
	virtual void bin( IOHelper* pWriter );

private:
	virtual void Delete();

	Color m_colAmbient;

};

#endif
