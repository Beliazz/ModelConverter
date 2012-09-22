#include "CameraNode.h"


CCameraNode::CCameraNode()
{
}

bool CCameraNode::VLoad( KFbxNode* pNode, KFbxScene* pScene )
{
	if (!pNode)
		return false;

	m_sName = pNode->GetName();

	KFbxCamera* pCamera		= (KFbxCamera*) pNode->GetNodeAttribute();
	KFbxNode* pTargetNode = pNode->GetTarget();
	KFbxNode* pTargetUpNode = pNode->GetTargetUp();

	//Camera Position And Orientation
	m_Position = Vector(pCamera->Position.Get().mData);
	m_InterestPosition = Vector(pCamera->InterestPosition.Get());
	m_Up = Vector(pCamera->UpVector.Get());

	m_fRoll = (float)pCamera->Roll.Get();

	m_sTargetName = pTargetNode->GetName();

	if(!pTargetUpNode)
		m_sTargetUpName = "Unknown";
	else
		m_sTargetName = pTargetUpNode->GetName();


	char* lProjectionTypes[] = { "Perspective", "Orthogonal" };

	m_sProjectionType = lProjectionTypes[pCamera->ProjectionType.Get()];



	//Viewing Area Controls
	char* lCameraFormat[] = { "Custom", "D1 NTSC", "NTSC", "PAL", "D1 PAL", 
		"HD", "640x480", "320x200", "320x240", "128x128",
		"Full Screen"  };

	char* lAspectRatioModes[] = { "Window Size", "Fixed Ratio", "Fixed Resolution", 
		"Fixed Width", "Fixed Height" };

	m_sFormat = lCameraFormat[pCamera->GetFormat()];

	m_sAspectRatioMode = lAspectRatioModes[pCamera->GetAspectRatioMode()];


	// If the ratio mode is eWINDOW_SIZE, both width and height values aren't relevant.
	if (pCamera->GetAspectRatioMode() != KFbxCamera::eWINDOW_SIZE)
	{
		m_fAspectWidth  = (float)pCamera->AspectWidth.Get();
		m_fAspectHeight = (float)pCamera->AspectHeight.Get();
	}

	m_fPixelAspectRatio = (float)pCamera->PixelAspectRatio.Get();
	m_fNearPlane = (float)pCamera->NearPlane.Get();
	m_fFarPlane = (float)pCamera->FarPlane.Get();
	m_bLockMode = pCamera->LockMode.Get();


	//Default Animation Values
	m_fDefaultFieldOfView	 = (float)pCamera->FieldOfView.Get();
	m_fDefaultFieldOfViewX	 = (float)pCamera->FieldOfViewX.Get();
	m_fDefaultFieldOfViewY	 = (float)pCamera->FieldOfViewY.Get();
	m_fDefaultOpticalCenterX = (float)pCamera->OpticalCenterX.Get();
	m_fDefaultOpticalCenterY = (float)pCamera->OpticalCenterY.Get();
	m_fDefaultRoll			 = (float)pCamera->Roll.Get();


	//Process Children
	for(int i = 0; i < pNode->GetChildCount(); i++)
	{
		m_pChildren.push_back( ProcessFBXSceneElement(  pNode->GetChild(i), pScene ) );
	}

	return true;
}

bool CCameraNode::VLoad( TiXmlElement* pScene )
{

	return true;
}

void CCameraNode::Delete()
{

}

TiXmlElement* CCameraNode::xml()
{
	TiXmlElement* xmlCamera = new TiXmlElement( GetType().c_str() );

	xmlCamera->SetAttribute( "Name", m_sName.c_str() );
	xmlCamera->SetAttribute( "ProjectionType", m_sProjectionType.c_str() );

	//Camera Position and Orientation
	TiXmlElement* xmlPositionOrientation = new TiXmlElement( "Position-Orientation" );
	TiXmlElement* xmlPosition = new TiXmlElement( "Position" );
	xmlPosition->LinkEndChild( xmlText( m_Position.str() ) );

	TiXmlElement* xmlCameraInterest = new TiXmlElement( "CameraInterest" );
	xmlCameraInterest->LinkEndChild( xmlText( m_sTargetName ) );

	TiXmlElement* xmlCameraInterestPos = new TiXmlElement( "CameraInterest-Position" );
	xmlCameraInterestPos->LinkEndChild( xmlText( m_InterestPosition.str() ) );

	TiXmlElement* xmlCameraUpTarget = new TiXmlElement( "CameraUpTarget" );
	xmlCameraUpTarget->LinkEndChild( xmlText( m_sTargetUpName ) );

	TiXmlElement* xmlUpVector = new TiXmlElement( "UpVector" );
	xmlUpVector->LinkEndChild( xmlText( m_Up.str() ) );

	TiXmlElement* xmlRoll = new TiXmlElement( "Roll" );
	xmlRoll->LinkEndChild( xmlText( m_fRoll ) );

	xmlPositionOrientation->LinkEndChild( xmlPosition );
	xmlPositionOrientation->LinkEndChild( xmlCameraInterest );
	xmlPositionOrientation->LinkEndChild( xmlCameraInterestPos );
	xmlPositionOrientation->LinkEndChild( xmlCameraUpTarget );
	xmlPositionOrientation->LinkEndChild( xmlUpVector );
	xmlPositionOrientation->LinkEndChild( xmlRoll );

	//Viewing Area Controls
	TiXmlElement* xmlViewingAreaControls = new TiXmlElement( "Viewing-Area-Controls" );
	xmlViewingAreaControls->SetAttribute( "Format", m_sFormat.c_str() );
	xmlViewingAreaControls->SetAttribute( " Aspect-Ratio-Mode", m_sAspectRatioMode.c_str() );

	if (m_sAspectRatioMode == "Window Size")
	{
		stringstream stm;

		stm << m_fAspectWidth;
		xmlViewingAreaControls->SetAttribute( "Aspect-Width", stm.str().c_str() );

		stm = stringstream("");
		stm << m_fAspectHeight;
		xmlViewingAreaControls->SetAttribute( "Aspect-Height", stm.str().c_str() );
	}


	TiXmlElement* xmlPixelRatio = new TiXmlElement( "Pixel-Ratio" );
	xmlPixelRatio->LinkEndChild( xmlText( m_fPixelAspectRatio ) );

	TiXmlElement* xmlNearPlane = new TiXmlElement( "Near-Plane" );
	xmlNearPlane->LinkEndChild( xmlText( m_fNearPlane ) );

	TiXmlElement* xmlFarPlane = new TiXmlElement( "Far-Plane" );
	xmlFarPlane->LinkEndChild( xmlText( m_fFarPlane ) );

	TiXmlElement* xmlMouseLock = new TiXmlElement( "Mouse-Lock" );
	xmlMouseLock->LinkEndChild( xmlText( m_bLockMode ) );

	xmlViewingAreaControls->LinkEndChild( xmlPixelRatio );
	xmlViewingAreaControls->LinkEndChild( xmlNearPlane );
	xmlViewingAreaControls->LinkEndChild( xmlFarPlane );
	xmlViewingAreaControls->LinkEndChild( xmlMouseLock );


	//DefaultAnimationValues
	TiXmlElement* xmlDefaultAnimationValues = new TiXmlElement( "Default-Animation-Values" );
	
	TiXmlElement* xmlDefaultFieldofView = new TiXmlElement( "Default-Field-of-View" );
	xmlDefaultFieldofView->LinkEndChild( xmlText( m_fDefaultFieldOfView ) );

	TiXmlElement* xmlDefaultFieldofViewX = new TiXmlElement( "Default-Field-of-View-X" );
	xmlDefaultFieldofViewX->LinkEndChild( xmlText( m_fDefaultFieldOfViewX ) );

	TiXmlElement* xmlDefaultFieldofViewY = new TiXmlElement( "Default-Field-of-View-Y" );
	xmlDefaultFieldofViewY->LinkEndChild( xmlText( m_fDefaultFieldOfViewY ) );

	TiXmlElement* xmlDefaultOpticalCenterX = new TiXmlElement( "Default-Optical-Center-X" );
	xmlDefaultOpticalCenterX->LinkEndChild( xmlText( m_fDefaultOpticalCenterX ) );

	TiXmlElement* xmlDefaultOpticalCenterY = new TiXmlElement( "Default-Optical-Center-Y" );
	xmlDefaultOpticalCenterY->LinkEndChild( xmlText( m_fDefaultOpticalCenterY ) );

	TiXmlElement* xmlDefaultRoll = new TiXmlElement( "Default-Roll" );
	xmlDefaultRoll->LinkEndChild( xmlText( m_fDefaultRoll ) );


	xmlDefaultAnimationValues->LinkEndChild( xmlDefaultFieldofView );
	xmlDefaultAnimationValues->LinkEndChild( xmlDefaultFieldofViewX );
	xmlDefaultAnimationValues->LinkEndChild( xmlDefaultFieldofViewY );
	xmlDefaultAnimationValues->LinkEndChild( xmlDefaultOpticalCenterX );
	xmlDefaultAnimationValues->LinkEndChild( xmlDefaultOpticalCenterY );
	xmlDefaultAnimationValues->LinkEndChild( xmlDefaultRoll );


	xmlCamera->LinkEndChild( xmlPositionOrientation );
	xmlCamera->LinkEndChild( xmlViewingAreaControls );
	xmlCamera->LinkEndChild( xmlDefaultAnimationValues );
	
	return xmlCamera;
}

