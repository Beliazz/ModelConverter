#include "ModelSDK.h"
#include "main.h"

enum FileTyp
{
	FBX,
	OBJ,
	STL,
	BLEND,
	DAE,
	_3DS,
	DXF,
	UNSUPPORTED,
};

/************************************************************************/
/* Common FBX Functions                                                 */
/************************************************************************/
void InitializeSdkObjects(KFbxSdkManager*& pSdkManager, KFbxScene*& pScene)
{
	// The first thing to do is to create the FBX SDK manager which is the 
	// object allocator for almost all the classes in the SDK.
	pSdkManager = KFbxSdkManager::Create();

	if (!pSdkManager)
	{
		printf("Unable to create the FBX SDK manager\n");
		exit(0);
	}

	// create an IOSettings object
	KFbxIOSettings * ios = KFbxIOSettings::Create(pSdkManager, IOSROOT );
	pSdkManager->SetIOSettings(ios);

	// Load plugins from the executable directory
	KString lPath = KFbxGetApplicationDirectory();
#if defined(KARCH_ENV_WIN)
	KString lExtension = "dll";
#elif defined(KARCH_ENV_MACOSX)
	KString lExtension = "dylib";
#elif defined(KARCH_ENV_LINUX)
	KString lExtension = "so";
#endif
	pSdkManager->LoadPluginsDirectory(lPath.Buffer(), lExtension.Buffer());

	// Create the entity that will hold the scene.
	pScene = KFbxScene::Create(pSdkManager,"");
}

void DestroySdkObjects(KFbxSdkManager* pSdkManager)
{
	// Delete the FBX SDK manager. All the objects that have been allocated 
	// using the FBX SDK manager and that haven't been explicitly destroyed 
	// are automatically destroyed at the same time.
	if (pSdkManager) pSdkManager->Destroy();
	pSdkManager = NULL;
}

bool LoadScene(KFbxSdkManager* pSdkManager, KFbxDocument* pScene, const char* pFilename)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor,  lSDKMinor,  lSDKRevision;
	//int lFileFormat = -1;
	int i, lAnimStackCount;
	bool lStatus;
	char lPassword[1024];

	// Get the file version number generate by the FBX SDK.
	KFbxSdkManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	// Create an importer.
	KFbxImporter* lImporter = KFbxImporter::Create(pSdkManager,"");

	// Initialize the importer by providing a filename.
	const bool lImportStatus = lImporter->Initialize(pFilename, -1, pSdkManager->GetIOSettings());
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if( !lImportStatus )
	{
		printf("Call to KFbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetLastErrorString());

		if (lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_YET ||
			lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_ANYMORE)
		{
			printf("FBX version number for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
			printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
		}

		return false;
	}

	printf("FBX version number for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

	if (lImporter->IsFBX())
	{
		printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.

		printf("Animation Stack Information\n");

		lAnimStackCount = lImporter->GetAnimStackCount();

		printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
		printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
		printf("\n");

		for(i = 0; i < lAnimStackCount; i++)
		{
			KFbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

			printf("    Animation Stack %d\n", i);
			printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
			printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

			// Change the value of the import name if the animation stack should be imported 
			// under a different name.
			printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

			// Set the value of the import state to false if the animation stack should be not
			// be imported. 
			printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
			printf("\n");
		}

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        true);
		IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         true);
		IOS_REF.SetBoolProp(IMP_FBX_LINK,            true);
		IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           true);
		IOS_REF.SetBoolProp(IMP_FBX_GOBO,            true);
		IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       true);
		IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}

	// Import the scene.
	lStatus = lImporter->Import(pScene);

	if(lStatus == false && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
	{
		printf("Please enter password: ");

		lPassword[0] = '\0';

		scanf("%s", lPassword);
		KString lString(lPassword);

		IOS_REF.SetStringProp(IMP_FBX_PASSWORD,      lString);
		IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

		lStatus = lImporter->Import(pScene);

		if(lStatus == false && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
		{
			printf("\nPassword is wrong, import aborted.\n");
		}
	}

	// Destroy the importer.
	lImporter->Destroy();

	return lStatus;
}


FileTyp	FilenameType( string filename )
{
	string extension = FilenameExtension( filename );

	if ( extension == "FBX" || extension == "fbx" )
		return FBX;

	if ( extension == "OBJ" || extension == "obj" )
		return OBJ;

	if ( extension == "STL" || extension == "STL" )
		return STL;

	if ( extension == "BLEND" || extension == "blend" )
		return BLEND;

	if ( extension == "3DS" || extension == "3ds" )
		return _3DS;

	if ( extension == "DXF" || extension == "dxf" )
		return DXF;

	return UNSUPPORTED;
}

void ProcessContent(KFbxNode* pNode, KFbxXMatrix& pParentGlobalPosition );


CModel::CModel(void)
{
	m_pRootElement = new RootElement();
}

CModel::~CModel(void)
{
	Delete();
}

void CModel::Delete()
{

}

bool CModel::Load( string filename )
{
	//Clear Global & Pose Map
	g_GlobalMap.clear();
	g_BindPosMap.clear();

	switch(FilenameType( filename ))
	{
	case FBX:
		return LoadFBX( filename );
		break;

	case OBJ:
		return LoadFBX( filename );
		break;

	case _3DS:
		return LoadFBX( filename );
		break;

	case DAE:
		return LoadFBX( filename );
		break;

	case DXF:
		return LoadFBX( filename );
		break;

	case STL:
		printf("[ERROR] Unsupported File format: %s\n",FilenameExtension(filename).c_str());
		return LoadSTL( filename );
		break;

	case BLEND:
		printf("[ERROR] Unsupported File format: %s\n",FilenameExtension(filename).c_str());
		return LoadBLEND( filename );
		break;

	case UNSUPPORTED:
		printf("[ERROR] Unsupported File format: %s\n",FilenameExtension(filename).c_str());
		return false;
		break;
	}

	printf("[ERROR-Fatal] (CModel::Load)\n");

	return false;
}

bool CModel::LoadFBX( string filename )
{
	//Scene to load from file
	KFbxScene* pScene = NULL;
	KFbxXMatrix lDummyGlobalPosition;

	// Prepare the FBX SDK.
	InitializeSdkObjects(g_pSdkManager, pScene);

	// Load the scene.
	bool lResult = LoadScene(g_pSdkManager, pScene, filename.c_str());

	if(lResult == false)
	{
		printf("\n\nAn error occurred while saving the scene...\n");
		DestroySdkObjects(g_pSdkManager);
		return 1;
	}

	//
	//Start Loading
	//

	//GlobalTimeSettings
	CGlobalTimeSettings* pTimeSettings = new CGlobalTimeSettings();

	if (!pTimeSettings->VLoad(pScene))
		return false;


	//GlobalLightSettings
	CGlobalLightSettings* pLightSettings = new CGlobalLightSettings();

	if (!pLightSettings->VLoad(pScene))
		return false;

	//GlobalCameraSettings
	CGlobalCameraSettings* pCameraSettings = new CGlobalCameraSettings();

	if (!pCameraSettings->VLoad(pScene))
		return false;

	//Add the Settings to the RootElement
	m_pRootElement->AddElement( pTimeSettings );
	m_pRootElement->AddElement( pLightSettings );
	m_pRootElement->AddElement( pCameraSettings );

	//
	//Load Scene Elements
	//

	//Declare SceneNodes
	CCameraNode*	pCameraNode		= NULL;
	CLightNode*		pLightNode		= NULL;
	CLODGroudNode*  pLODGroudNode	= NULL;
	CMarkerNode*	pMarkerNode		= NULL;
	CMeshNode*		pMeshNode		= NULL;
	CNurbNode*		pNurbNode		= NULL;
	CPatchNode*		pPatchNode		= NULL;
	CSkeletonNode*	pSkeletonNode	= NULL;
	CEmptyNode*		pEmptyNode		= NULL;

	//Get Root Node
	KFbxNode* pNode = pScene->GetRootNode();

	if(pNode)
	{

		//First Process als Nodes but only save the GlobalPosition
		for(int i = 0; i < pNode->GetChildCount(); i++)
		{
			KFbxNode* pChildeNode = pNode->GetChild(i);

			ProcessContent( pChildeNode, lDummyGlobalPosition );
		}

		lDummyGlobalPosition = KFbxXMatrix();


		//Now Save all the Data
		for(int i = 0; i < pNode->GetChildCount(); i++)
		{
			KFbxNode* pChildeNode = pNode->GetChild(i);


			if(!pChildeNode->GetNodeAttribute())
			{
				pEmptyNode = new CEmptyNode();

				if (!pEmptyNode->VLoad( pChildeNode, pScene ))
					return false;

				m_pRootElement->AddElement( pEmptyNode );

			}
			else
			{
				switch (pChildeNode->GetNodeAttribute()->GetAttributeType())
				{

					//Mesh
					/////////////////////////////////////////////////////////////////////////////////////////
				case KFbxNodeAttribute::eMESH:

					pMeshNode = new CMeshNode();

					if (!pMeshNode->VLoad( pChildeNode, pScene ))
						return false;

					m_pRootElement->AddElement( pMeshNode );

					break;


					//Marker
					/////////////////////////////////////////////////////////////////////////////////////////
				case KFbxNodeAttribute::eMARKER:

					pMarkerNode = new CMarkerNode();

					if (!pMarkerNode->VLoad( pChildeNode, pScene ))
						return false;

					m_pRootElement->AddElement( pMarkerNode );

					break;

					//Skeleton
					/////////////////////////////////////////////////////////////////////////////////////////
				case KFbxNodeAttribute::eSKELETON:  

					pSkeletonNode = new CSkeletonNode();

					if (!pSkeletonNode->VLoad( pChildeNode, pScene ))
						return false;

					m_pRootElement->AddElement( pSkeletonNode );

					break;

					//Nurb
					/////////////////////////////////////////////////////////////////////////////////////////
				case KFbxNodeAttribute::eNURB:   

					pNurbNode = new CNurbNode();

					if (!pNurbNode->VLoad( pChildeNode, pScene ))
						return false;

					m_pRootElement->AddElement( pNurbNode );

					break;

					//Patch
					/////////////////////////////////////////////////////////////////////////////////////////
				case KFbxNodeAttribute::ePATCH:     

					pPatchNode = new CPatchNode();

					if (!pPatchNode->VLoad( pChildeNode, pScene ))
						return false;

					m_pRootElement->AddElement( pPatchNode );

					break;

					//Camera
					/////////////////////////////////////////////////////////////////////////////////////////
				case KFbxNodeAttribute::eCAMERA:  

					pCameraNode = new CCameraNode();

					if (!pCameraNode->VLoad( pChildeNode, pScene  ))
						return false;

					m_pRootElement->AddElement( pCameraNode );

					break;

					//Light
					/////////////////////////////////////////////////////////////////////////////////////////
				case KFbxNodeAttribute::eLIGHT: 

					pLightNode = new CLightNode();

					if (!pLightNode->VLoad( pChildeNode, pScene ))
						return false;

					m_pRootElement->AddElement( pLightNode );

					break;

					//LODGroup
					/////////////////////////////////////////////////////////////////////////////////////////
				case KFbxNodeAttribute::eLODGROUP:

					pLODGroudNode = new CLODGroudNode();

					if (!pLODGroudNode->VLoad( pChildeNode ))
						return false;

					m_pRootElement->AddElement( pLODGroudNode );

					break;

				}
			}
		}
	}

	//Free FBX Objects
	if (g_pSdkManager) 
		g_pSdkManager->Destroy();

	g_pSdkManager = NULL;

	return true;
}

bool CModel::LoadSTL( string filename )
{
	return true;
}

bool CModel::LoadBLEND( string filename )
{
	return true;
}

bool CModel::Save( string directory, string filename, SaveType mode  )
{	
	string dataDir;
	string modelDir;
	string name;
	string savePath;
	string texturePath;
	string newFilename;

	//Get name from filename
	name = GetNameFromFilepath(filename); 

	savePath       = directory + name + "\\";
	g_sTexturepath = savePath + "textures";
	newFilename    = savePath + FilenameTitle(filename);

	//Create a directory for the model
	CreateDirectoryA( savePath.c_str(), NULL );

	//Create a directory for the textures
	CreateDirectoryA( g_sTexturepath.c_str(), NULL );

	switch(mode)
	{
	case ASCII:

		break;

	case XML:
		{

		//Test for saving it to XML
		TiXmlDocument doc;
		TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
		doc.LinkEndChild( decl );
		TiXmlElement* xmlRoot = new TiXmlElement( "Bloco-XML-Scene" );

		m_pRootElement->VSave( xmlRoot );

		doc.LinkEndChild( xmlRoot );
		doc.SaveFile( newFilename.c_str() );	

		}
		break;

	case BINARY:
		{
			IOHelper* writer = new IOHelper();

			if( !writer->open( newFilename.c_str(), "wb" ) )
				return false;

			m_pRootElement->VSave( writer );

			writer->close();

			delete writer;
		}
		break;
	}

	return true;
}

void ProcessContent(KFbxNode* pNode, KFbxXMatrix& pParentGlobalPosition )
{
	//global Position
	KFbxXMatrix lGlobalPosition = GetGlobalPosition(pNode, 0, NULL, &pParentGlobalPosition);

	if ( pNode->GetNodeAttribute() )
	{
		// Geometry offset.
		// it is not inherited by the children.
		KFbxXMatrix lGeometryOffset = GetGeometry(pNode);
		KFbxXMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;

// 		KFbxXMatrix negativScaleMatrix;
// 		negativScaleMatrix.SetIdentity();
// 		negativScaleMatrix.SetS(KFbxVector4(1.0f,1.0f,-1.0f));

		g_GlobalMap[pNode] = lGlobalOffPosition;
	}


	for(int i = 0; i < pNode->GetChildCount(); i++)
	{
		KFbxNode* pChildeNode = pNode->GetChild(i);

		ProcessContent( pChildeNode, pParentGlobalPosition );
	}
}
