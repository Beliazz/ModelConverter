#include "Definitions.h"

//FBX SDK Manager
KFbxSdkManager* g_pSdkManager = NULL;

//Maps
map<KFbxNode*,KFbxMatrix> g_GlobalMap;
map<KFbxNode*,KFbxMatrix> g_BindPosMap;

string g_sTexturepath;

#define MAXBONES_PER_VERTEX 4


CBoneWeights::CBoneWeights() : m_fWeightSum(0)
{
}

//--------------------------------------------------------------------------------------
void CBoneWeights::AddBoneWeight(int nBoneIndex, float fBoneWeight)
{
	if( fBoneWeight <= 0.0f )
		return;

	m_fWeightSum += fBoneWeight;

	float bAdded = false;
	for(std::vector<std::pair<int,float>>::iterator it = m_boneWeights.begin(); it != m_boneWeights.end(); ++it )
	{
		if( fBoneWeight > it->second )
		{
			m_boneWeights.insert(it, std::make_pair(nBoneIndex, fBoneWeight));
			bAdded = true;
			break;
		}
	}
	if( !bAdded )
		m_boneWeights.push_back(std::make_pair(nBoneIndex,fBoneWeight));
}

//--------------------------------------------------------------------------------------
void CBoneWeights::Validate()
{
	m_fWeightSum = 0.0f;
	int nIndex = 0;

	std::vector<std::pair<int,float>>::iterator it = m_boneWeights.begin();
	while(it != m_boneWeights.end())
	{
		if( nIndex >= MAXBONES_PER_VERTEX )
		{
			it = m_boneWeights.erase(it);
		}
		else
		{
			m_fWeightSum += it->second;
			++nIndex;
			++it;
		}
	}
}

//--------------------------------------------------------------------------------------
void CBoneWeights::Normalize()
{
	Validate();

	float fScale = 1.0f/m_fWeightSum;
	std::vector<std::pair<int,float>>::iterator it = m_boneWeights.begin();
	while(it != m_boneWeights.end())
	{
		it->second *= fScale;
		++it;
	}
}



bool ParseMeshSkinning( KFbxMesh* pMesh, CSkinData* pSkinData, KFbxScene* pScene )
{
	DWORD dwDeformerCount = pMesh->GetDeformerCount( KFbxDeformer::eSKIN );
	if( dwDeformerCount == 0 )
		return false;

	const DWORD dwVertexCount = pMesh->GetControlPointsCount();
	const DWORD dwStride = 4;
	pSkinData->Alloc( dwVertexCount, dwStride );

	vector<float> weights;
	vector<UINT> indices;

	for( DWORD dwDeformerIndex = 0; dwDeformerIndex < dwDeformerCount; ++dwDeformerIndex )
	{
		KFbxSkin* pSkin = (KFbxSkin*)pMesh->GetDeformer( dwDeformerIndex, KFbxDeformer::eSKIN );
		DWORD dwClusterCount = pSkin->GetClusterCount();

		for( DWORD dwClusterIndex = 0; dwClusterIndex < dwClusterCount; ++dwClusterIndex )
		{
			KFbxCluster* pCluster = pSkin->GetCluster( dwClusterIndex );
			DWORD dwClusterSize = pCluster->GetControlPointIndicesCount();
			if( dwClusterSize == 0 )
				continue;

			KFbxNode* pLink = pCluster->GetLink();
			CBone bone;

			bone.pNode = pLink;
			bone.sName = pLink->GetName();
			bone.iParentIndex = -1;

			DWORD dwBoneIndex = pSkinData->GetBoneCount();
			pSkinData->AddBone(bone);

			KFbxXMatrix matXBindPose;
			pCluster->GetTransformLinkMatrix( matXBindPose );
			KFbxMatrix matBindPose = matXBindPose;

			//Check if this matrix already exists in the BindPosMap else add it
			map<KFbxNode*,KFbxMatrix>::iterator iter = g_BindPosMap.find( pLink );
			if( iter != g_BindPosMap.end() )
			{
				KFbxMatrix matExisting = iter->second;
				if( matExisting != matBindPose )
				{
					// found the bind pose matrix, but it is different than what we previously encountered
					g_BindPosMap[pLink] = matBindPose;
				}
			}
			else
			{
				// have not encountered this frame in the bind pose yet
				g_BindPosMap[pLink] = matBindPose;
			}


			INT* pIndices = pCluster->GetControlPointIndices();
			DOUBLE* pWeights = pCluster->GetControlPointWeights();

			for( DWORD i = 0; i < dwClusterSize; ++i )
			{
				indices.push_back( dwBoneIndex );
				weights.push_back((FLOAT)pWeights[i] );

				pSkinData->InsertWeight( pIndices[i], dwBoneIndex, (FLOAT)pWeights[i] );
			}
		}
	}

	//All bones are stored. Now get the hierarchy
	pSkinData->SetHierarchy( pScene->GetRootNode() );

	return true;
}

KFbxXMatrix GetPoseMatrix( KFbxPose* pPose, int pNodeIndex )
{
	KFbxXMatrix lPoseMatrix;
	KFbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

	memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

	return lPoseMatrix;
}

KFbxXMatrix GetGeometry( KFbxNode* pNode )
{
	const KFbxVector4 lT = pNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET);
	const KFbxVector4 lR = pNode->GetGeometricRotation(KFbxNode::eSOURCE_SET);
	const KFbxVector4 lS = pNode->GetGeometricScaling(KFbxNode::eSOURCE_SET);

	return KFbxXMatrix(lT, lR, lS);
}

KFbxXMatrix GetGlobalPosition( KFbxNode* pNode, const KTime& pTime, KFbxPose* pPose , KFbxXMatrix* pParentGlobalPosition )
{
	KFbxXMatrix lGlobalPosition;
	bool        lPositionFound = false;

	if (pPose)
	{
		int lNodeIndex = pPose->Find(pNode);

		if (lNodeIndex > -1)
		{
			// The bind pose is always a global matrix.
			// If we have a rest pose, we need to check if it is
			// stored in global or local space.
			if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
			{
				lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
			}
			else
			{
				// We have a local matrix, we need to convert it to
				// a global space matrix.
				KFbxXMatrix lParentGlobalPosition;

				if (pParentGlobalPosition)
				{
					lParentGlobalPosition = *pParentGlobalPosition;
				}
				else
				{
					if (pNode->GetParent())
					{
						lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
					}
				}

				KFbxXMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
				lGlobalPosition = lParentGlobalPosition * lLocalPosition;
			}

			lPositionFound = true;
		}
	}

	if (!lPositionFound)
	{
		// There is no pose entry for that node, get the current global position instead.

		// Ideally this would use parent global position and local position to compute the global position.
		// Unfortunately the equation 
		//    lGlobalPosition = pParentGlobalPosition * lLocalPosition
		// does not hold when inheritance type is other than "Parent" (RSrs).
		// To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
		lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
	}

	return lGlobalPosition;
}


//--------------------------------------------------------------------------------------
void ProcessBoneWeights(KFbxMesh* pFBXMesh, std::vector<CBoneWeights>& meshBoneWeights, CSkinData* skindata )
{
	for( int i = 0; i < pFBXMesh->GetDeformerCount(); ++i )
	{
		KFbxDeformer* pFBXDeformer = pFBXMesh->GetDeformer(i);

		if( !pFBXDeformer )
			continue;

		if( pFBXDeformer->GetDeformerType() == KFbxDeformer::eSKIN )
		{
			KFbxSkin* pFBXSkin = (KFbxSkin*)pFBXMesh->GetDeformer( i, KFbxDeformer::eSKIN );

			if( !pFBXSkin )
				continue;

			ProcessBoneWeights(pFBXSkin, meshBoneWeights, skindata);
		}
	}
}

//--------------------------------------------------------------------------------------
void ProcessBoneWeights(KFbxSkin* pFBXSkin, std::vector<CBoneWeights>& meshBoneWeights, CSkinData* skindata )
{
	KFbxCluster::ELinkMode linkMode = KFbxCluster::eNORMALIZE; //Default link mode

	std::vector<CBoneWeights> skinBoneWeights(meshBoneWeights.size(), CBoneWeights());
	int nClusterCount = pFBXSkin->GetClusterCount();
	for( int i = 0; i < nClusterCount; ++i )
	{
		KFbxCluster* pFBXCluster = pFBXSkin->GetCluster(i);

		if( !pFBXCluster )
			continue;

		linkMode = pFBXCluster->GetLinkMode();
		KFbxNode* pLinkNode = pFBXCluster->GetLink();

		if( !pLinkNode )
			continue;

		int nBoneIndex = skindata->FindBone(pLinkNode->GetName());
		if( nBoneIndex < 0 )
			continue;

		int* indices = pFBXCluster->GetControlPointIndices();
		double* weights = pFBXCluster->GetControlPointWeights();

		for( int j = 0; j < pFBXCluster->GetControlPointIndicesCount(); ++j )
		{
			skinBoneWeights[indices[j]].AddBoneWeight(nBoneIndex, (float)weights[j]);
		}
	}

	switch(linkMode)
	{
	case KFbxCluster::eNORMALIZE:	//Normalize so weight sum is 1.0.
		for( int i = 0; i < (int)skinBoneWeights.size(); ++i )
		{
			skinBoneWeights[i].Normalize();
		}
		break;

	case KFbxCluster::eADDITIVE:	//Not supported yet. Do nothing
		break;

	case KFbxCluster::eTOTAL1:		//The weight sum should already be 1.0. Do nothing.
		break;
	}

	for( int i = 0; i < (int)meshBoneWeights.size(); ++i )
	{
		meshBoneWeights[i].AddBoneWeights(skinBoneWeights[i]);
	}	
}

ModelElement* ProcessFBXSceneElement( KFbxNode* pNode, KFbxScene* pScene )
{
	//Declare SceneNodes
	CCameraNode*	pCameraNode		= NULL;
	CLightNode*		pLightNode		= NULL;
	CLODGroudNode*  pLODGroudNode	= NULL;
	CMarkerNode*	pMarkerNode		= NULL;
	CMeshNode*		pMeshNode		= NULL;
	CNurbNode*		pNurbNode		= NULL;
	CPatchNode*		pPatchNode		= NULL;
	CSkeletonNode*	pSkeletonNode	= NULL;


	if(pNode->GetNodeAttribute() == NULL)
	{
		CEmptyNode * pEmptyNode = new CEmptyNode();

		if (!pEmptyNode->VLoad( pNode, pScene ))
			return false;

		return pEmptyNode;
	}
	else
	{
		switch (pNode->GetNodeAttribute()->GetAttributeType())
		{

			//Mesh
			/////////////////////////////////////////////////////////////////////////////////////////
		case KFbxNodeAttribute::eMESH:

			pMeshNode = new CMeshNode();

			if (!pMeshNode->VLoad( pNode, pScene ))
				return false;

			return pMeshNode;

			break;


			//Marker
			/////////////////////////////////////////////////////////////////////////////////////////
		case KFbxNodeAttribute::eMARKER:

			pMarkerNode = new CMarkerNode();

			if (!pMarkerNode->VLoad( pNode, pScene ))
				return false;

			return pMarkerNode;

			break;

			//Skeleton
			/////////////////////////////////////////////////////////////////////////////////////////
		case KFbxNodeAttribute::eSKELETON:  

			pSkeletonNode = new CSkeletonNode();

			if (!pSkeletonNode->VLoad( pNode, pScene ))
				return false;

			return pSkeletonNode;

			break;

			//Nurb
			/////////////////////////////////////////////////////////////////////////////////////////
		case KFbxNodeAttribute::eNURB:   

			pNurbNode = new CNurbNode();

			if (!pNurbNode->VLoad( pNode, pScene  ))
				return false;

			return pNurbNode;

			break;

			//Patch
			/////////////////////////////////////////////////////////////////////////////////////////
		case KFbxNodeAttribute::ePATCH:     

			pPatchNode = new CPatchNode();

			if (!pPatchNode->VLoad( pNode , pScene))
				return false;

			return pPatchNode;

			break;

			//Camera
			/////////////////////////////////////////////////////////////////////////////////////////
		case KFbxNodeAttribute::eCAMERA:  

			pCameraNode = new CCameraNode();

			if (!pCameraNode->VLoad( pNode, pScene ))
				return false;

			return pCameraNode;

			break;

			//Light
			/////////////////////////////////////////////////////////////////////////////////////////
		case KFbxNodeAttribute::eLIGHT: 

			pLightNode = new CLightNode();

			if (!pLightNode->VLoad( pNode, pScene ))
				return false;

			return pLightNode;

			break;

			//LODGroup
			/////////////////////////////////////////////////////////////////////////////////////////
		case KFbxNodeAttribute::eLODGROUP:

			pLODGroudNode = new CLODGroudNode();

			if (!pLODGroudNode->VLoad( pNode ))
				return false;

			return pLODGroudNode;

			break;

		}
	}


	//Should not happen
	CEmptyNode * pEmptyNode = new CEmptyNode();

	if (!pEmptyNode->VLoad( pNode, pScene ))
		return false;

	return pEmptyNode;
}