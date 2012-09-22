#ifndef LightNode_H__
#define LightNode_H__

#include "..\main.h"

struct sGobo
{
	string  m_sFilename;
	bool m_bGroundProjection;
	bool m_bVolumetricProjection;
	bool m_bFrontVolumetricProjection;

	TiXmlElement* xml();

};

class CLightNode : public ModelElement
{
public:
	CLightNode();

	virtual bool VLoad( KFbxNode* pNode, KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "LightNode"; }

	virtual TiXmlElement* xml();

private:
	string m_sName;
	string m_sType;
	bool   m_bCastLight;

	bool m_bHasGobo;
	sGobo m_Gobo;

	//Default Animation Values
	Color m_DefaultColor;
	float m_fDefaultIntensity;
	float m_fDefaultConeAngle;
	float m_fDefaultFog;

	virtual void Delete();

};

#endif
