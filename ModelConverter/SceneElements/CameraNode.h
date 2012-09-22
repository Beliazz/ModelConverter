#ifndef CameraNode_H__
#define CameraNode_H__

#include "..\main.h"


class CCameraNode : public ModelElement
{
public:
	CCameraNode();

	virtual bool VLoad( KFbxNode* pNode, KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "CameraNode"; }
	virtual TiXmlElement* xml();


private:
	string m_sName;
	string m_sTargetName;
	string m_sTargetUpName;
	string m_sProjectionType;

	Vector m_InterestPosition;
	Vector m_Position;
	Vector m_Up;

	string m_sFormat;
	string m_sAspectRatioMode;

	float m_fRoll;
	float m_fAspectWidth;
	float m_fAspectHeight;

	float m_fPixelAspectRatio;
	float m_fNearPlane;
	float m_fFarPlane;
	bool  m_bLockMode;

	float m_fDefaultFieldOfView;
	float m_fDefaultFieldOfViewX;
	float m_fDefaultFieldOfViewY;
	float m_fDefaultOpticalCenterX;
	float m_fDefaultOpticalCenterY;
	float m_fDefaultRoll;

	virtual void Delete();

};

#endif
