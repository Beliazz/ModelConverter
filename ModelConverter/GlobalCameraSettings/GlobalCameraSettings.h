#ifndef GlobalCameraSettings_H__
#define GlobalCameraSettings_H__

#include "..\main.h"


class CGlobalCameraSettings : public ModelElement
{
public:
	CGlobalCameraSettings();

	virtual bool VLoad( KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "GlobalCameraSettings"; }
	virtual string str();
	virtual TiXmlElement* xml();
	virtual void bin( IOHelper* pWriter );

private:
	virtual void Delete();

	string m_sDefaultCamera;

};

#endif
