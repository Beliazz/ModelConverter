#ifndef GlobalTimeSettings_H__
#define GlobalTimeSettings_H__

#include "..\main.h"


class CGlobalTimeSettings : public ModelElement
{
public:
	CGlobalTimeSettings();

	virtual bool VLoad( KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "GlobalTimeSettings"; }

	virtual string str();
	virtual TiXmlElement* xml();
	virtual void bin( IOHelper* pWriter );

private:
	virtual void Delete();

	string m_sTimeMode;
	float  m_fStart;
	float  m_fEnd;
};

#endif
