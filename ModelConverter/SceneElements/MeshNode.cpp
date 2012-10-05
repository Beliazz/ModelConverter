#include "MeshNode.h"

#define cn "\""


void CalcMinMax(Vector3 &min,Vector3 &max, Vector3 src)
{
	max.x = max(max.x,src.x);
	max.y = max(max.y,src.y);
	max.z = max(max.z,src.z);

	min.x = min(min.x,src.x);
	min.y = min(min.y,src.y);
	min.z = min(min.z,src.z);
}

map<KFbxSurfaceMaterial*,vector<DWORD>> gMaterialMap;

//Fills VB & IB from a FBX Mesh
void GetGeometryData(KFbxMesh* pMesh, CSkinData* skindata, VertexBuffer* vb );

void GetMaterials(  KFbxGeometry* pGeometry, vector<sMaterial__>& materials );

void GetTextures( KFbxGeometry* pGeometry, vector<sTexture__>& textures );

void GetMaterialConnections(KFbxMesh* pMesh, sMaterialConnection__& materialConnection, vector<sMaterial__> materials );


CMeshNode::CMeshNode()
{
	m_VB = new VertexBuffer();
}

bool CMeshNode::VLoad( KFbxNode* pNode, KFbxScene* pScene  )
{
	if (!pNode)
		return false;


	if ( !pNode->GetMesh()->IsTriangleMesh() )
	{
		KFbxGeometryConverter GeometryConverter(pNode->GetFbxSdkManager());
		if( !GeometryConverter.TriangulateInPlace( pNode ) )
			return false;
	}

	//Check if it is triangulated
	KFbxMesh* pMesh = pNode->GetMesh();
	if( !pMesh )
		return false;


	//Mesh Name
	m_sName = pNode->GetName();

	CSkinData skindata;
	BOOL bSkinnedMesh = ParseMeshSkinning( pMesh, &skindata, pScene );

	//Geometry
	GetGeometryData( pMesh, &skindata, m_VB );

	//GlobalMatrix
	map<KFbxNode*,KFbxMatrix>::iterator it = g_GlobalMap.find( pNode );

	//Check if it exists
	if (it == g_GlobalMap.end())
	{
		printf("[Error] There is no GlobalMartix for Mesh: %s", pNode->GetName() );
		return false;
	}

	m_GlobalMatrix = it->second;

	if( bSkinnedMesh )
	{
		//Bones
		for (unsigned int i = 0; i < skindata.GetBoneCount() ; i++)
		{
			sMeshBone bone;

			bone.m_sName		= skindata.GetBoneName(i);
			bone.m_sParentName	= skindata.GetBoneParentName(i);

			bone.m_GlobalMatrix   =  g_GlobalMap[  skindata.GetBoneNode(i) ];
			bone.m_BindPoseMatrix =  g_BindPosMap[ skindata.GetBoneNode(i) ];

			bone.m_boundingMin = Vector3(99999.0f,99999.0f,99999.0f);
			bone.m_boundingMax = Vector3(-99999.0f,-99999.0f,-99999.0f);

			m_vecBones.push_back( bone );
		}

		//Animation

		KArrayTemplate<KString*> TakeNameArray;
		pScene->FillAnimStackNameArray( TakeNameArray );

		TiXmlElement* xmlAnimations = new TiXmlElement("Animations");

		DWORD dwTakeCount = (DWORD)TakeNameArray.GetCount();
		for( DWORD i = 0; i < dwTakeCount; ++i )
		{
			KString* strTakeName = TakeNameArray.GetAt(i);
			sAnimationsTake__ animTake;

			animTake.m_sName = strTakeName->Buffer();

			pScene->GetEvaluator()->SetContext(KFbxCast<KFbxAnimStack>(pScene->GetSrcObject(FBX_TYPE(KFbxAnimStack), i)));


			if( strTakeName->Compare( KFBXTAKENODE_DEFAULT_NAME ) == 0 )
			{
				continue;
			}
			else
			{

				KFbxTakeInfo* pTakeInfo = pScene->GetTakeInfo( *strTakeName );

				KTime FrameTime;
				FrameTime.SetTime( 0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode() );


				FLOAT fStartTime = 0.0f;
				FLOAT fEndTime = 1.0f;
				FLOAT fFrameTime = (FLOAT)FrameTime.GetSecondDouble();
				FLOAT fDeltaTime = fFrameTime; 
				FLOAT fCurrentTime = fStartTime;

				#ifdef FIXED_FRAMETIME
					fDeltaTime = FRAMETIME;
				#endif

				if( pTakeInfo != NULL )
				{
					fStartTime = (FLOAT)pTakeInfo->mLocalTimeSpan.GetStart().GetSecondDouble();
					fEndTime = (FLOAT)pTakeInfo->mLocalTimeSpan.GetStop().GetSecondDouble();

					animTake.m_fStart = fStartTime;
					animTake.m_fEnd = fEndTime;
				}
				else
				{
					printf( "[Warning ]Animation take (%s) has no takeinfo; using defaults.\n", strTakeName->Buffer() );
				}

				while( fCurrentTime <= fEndTime )
				{
					KTime CurrentTime;
					CurrentTime.SetSecondDouble( fCurrentTime );

					for (unsigned int i = 0; i < skindata.GetBoneCount() ; i++)
					{
						sAnimationsKey__ animKey;


						KFbxXMatrix BoneLocalSpaceV =  (*(KFbxXMatrix*)(double*)&g_BindPosMap[ skindata.GetBoneNode(i) ]);

						//int procent = (fCurrentTime/fEndTime) * 100;
						/*printf( "Parsing animation (%s)  %d%% \r", strTakeName->Buffer(), procent );*/

						KFbxXMatrix ModelSpaceAnimatedM = GetGlobalPosition( pNode, CurrentTime ).Inverse() * GetGlobalPosition( skindata.GetBoneNode(i), CurrentTime )  * BoneLocalSpaceV.Inverse();


						animKey.m_sInfulencedBone  = skindata.GetBoneName(i);
						animKey.m_dBoneIndex = i;
						animKey.m_fTimestamp = fCurrentTime;
						animKey.m_Matrix = ModelSpaceAnimatedM;

						animTake.m_vecAnimationKeys.push_back( animKey );

					}//for skindata.GetBoneCount()
					fCurrentTime += fDeltaTime;

				}//while fCurrentTime <= fEndTime
			}
			m_vecAnimationTakes.push_back( animTake );
		}//for dwTakeCount
	}

	GetMaterials( pMesh, m_vecMaterials );
	GetTextures(  pMesh, m_vecTextures  );

	for (unsigned int i = 0; i < m_vecTextures.size() ; i++)
	{
		m_vecMaterials[m_vecTextures[i].m_dAssignedMaterial].m_AssignedTexture = i;
		m_vecMaterials[m_vecTextures[i].m_dAssignedMaterial].m_HasTexture = true;
	}

	GetMaterialConnections( pMesh, m_MaterialConnection, m_vecMaterials );

	gMaterialMap.clear();

	for (unsigned int i = 0; i < m_VB->m_vecBoneIndices.size() ; i++)
	{
		UINT index = m_VB->m_vecBoneIndices[i].x;


		if (m_VB->m_vecBoneWeights[i].x > 0.2f)
		{
			CalcMinMax(m_vecBones[index].m_boundingMin,m_vecBones[index].m_boundingMax,m_VB->m_vecPositions[i]);

			m_vecBones[index].m_boundingBox.x = (m_vecBones[index].m_boundingMax.x - m_vecBones[index].m_boundingMin.x)/2.0f;
			m_vecBones[index].m_boundingBox.y = (m_vecBones[index].m_boundingMax.y - m_vecBones[index].m_boundingMin.y)/2.0f;
			m_vecBones[index].m_boundingBox.z = (m_vecBones[index].m_boundingMax.z - m_vecBones[index].m_boundingMin.z)/2.0f;
		}


		

		index = m_VB->m_vecBoneIndices[i].y;

		if (m_VB->m_vecBoneWeights[i].y > 0.2f)
		{
			CalcMinMax(m_vecBones[index].m_boundingMin,m_vecBones[index].m_boundingMax,m_VB->m_vecPositions[i]);

			m_vecBones[index].m_boundingBox.x = (m_vecBones[index].m_boundingMax.x - m_vecBones[index].m_boundingMin.x)/2.0f;
			m_vecBones[index].m_boundingBox.y = (m_vecBones[index].m_boundingMax.y - m_vecBones[index].m_boundingMin.y)/2.0f;
			m_vecBones[index].m_boundingBox.z = (m_vecBones[index].m_boundingMax.z - m_vecBones[index].m_boundingMin.z)/2.0f;
		}
		

		index = m_VB->m_vecBoneIndices[i].z;


		if (m_VB->m_vecBoneWeights[i].z > 0.2f)
		{
			CalcMinMax(m_vecBones[index].m_boundingMin,m_vecBones[index].m_boundingMax,m_VB->m_vecPositions[i]);

			m_vecBones[index].m_boundingBox.x = (m_vecBones[index].m_boundingMax.x - m_vecBones[index].m_boundingMin.x)/2.0f;
			m_vecBones[index].m_boundingBox.y = (m_vecBones[index].m_boundingMax.y - m_vecBones[index].m_boundingMin.y)/2.0f;
			m_vecBones[index].m_boundingBox.z = (m_vecBones[index].m_boundingMax.z - m_vecBones[index].m_boundingMin.z)/2.0f;
		}


		index = m_VB->m_vecBoneIndices[i].w;


		if (m_VB->m_vecBoneWeights[i].w > 0.2f)
		{
			CalcMinMax(m_vecBones[index].m_boundingMin,m_vecBones[index].m_boundingMax,m_VB->m_vecPositions[i]);

			m_vecBones[index].m_boundingBox.x = (m_vecBones[index].m_boundingMax.x - m_vecBones[index].m_boundingMin.x)/2.0f;
			m_vecBones[index].m_boundingBox.y = (m_vecBones[index].m_boundingMax.y - m_vecBones[index].m_boundingMin.y)/2.0f;
			m_vecBones[index].m_boundingBox.z = (m_vecBones[index].m_boundingMax.z - m_vecBones[index].m_boundingMin.z)/2.0f;
		}
	}


	//Process Children
	for(int i = 0; i < pNode->GetChildCount(); i++)
	{
		m_pChildren.push_back( ProcessFBXSceneElement(  pNode->GetChild(i), pScene ) );
	}
	return true;
}

bool CMeshNode::VLoad( TiXmlElement* pScene )
{

	return true;
}

void CMeshNode::Delete()
{
	safe_delete(m_VB);
}

std::string CMeshNode::str()
{
	stringstream stm;

	stm << m_sName << endl;

	return stm.str();
}

struct dat
{
	float f;
	int   i;
	DWORD d;
	char  c[256];
};

TiXmlElement* CMeshNode::xml()
{
	TiXmlElement* xmlMesh = new TiXmlElement( GetType().c_str() );
	xmlMesh->SetAttribute( "Name", m_sName.c_str() );

	//Geometry
	TiXmlElement* xmlGeometry	  = new TiXmlElement( "Geometry" );
	xmlGeometry->LinkEndChild( m_VB->xml() );



	//Joints
	TiXmlElement* xmlBones = new TiXmlElement( "Bones" );
	xmlBones->SetAttribute( "Size", m_vecBones.size() );

	for (unsigned int i = 0; i < m_vecBones.size() ; i++)
	{
		xmlBones->LinkEndChild( m_vecBones[i].xml() );
	}

	//Animation
	TiXmlElement* xmlAnimation = new TiXmlElement( "Animation" );
	xmlAnimation->SetAttribute( "Size", m_vecAnimationTakes.size() );

	for (unsigned int i = 0; i < m_vecAnimationTakes.size() ; i++)
	{
		xmlAnimation->LinkEndChild( m_vecAnimationTakes[i].xml() );
	}

	//Materials
	TiXmlElement* xmlMaterial = new TiXmlElement( "Materials" );
	xmlMaterial->SetAttribute( "Size", m_vecMaterials.size() );

	for (unsigned int i = 0; i < m_vecMaterials.size() ; i++)
	{
		xmlMaterial->LinkEndChild( m_vecMaterials[i].xml() );
	}

	//Textures
	TiXmlElement* xmlTextures = new TiXmlElement( "Textures" );
	xmlTextures->SetAttribute( "Size", m_vecTextures.size() );

	for (unsigned int i = 0; i < m_vecTextures.size() ; i++)
	{
		xmlTextures->LinkEndChild( m_vecTextures[i].xml() );
	}

	//Global Matrix
	TiXmlElement* xmlGlobalMatrix = new TiXmlElement( "GlobalMatrix" );
	xmlGlobalMatrix->LinkEndChild( xmlText(m_GlobalMatrix) );

	xmlMesh->LinkEndChild( xmlGlobalMatrix );
	xmlMesh->LinkEndChild( xmlGeometry );
	xmlMesh->LinkEndChild( xmlBones );
	xmlMesh->LinkEndChild( xmlAnimation );
	xmlMesh->LinkEndChild( xmlMaterial );
	xmlMesh->LinkEndChild( xmlTextures );
	xmlMesh->LinkEndChild( m_MaterialConnection.xml() );

	return xmlMesh;
}

void CMeshNode::bin( IOHelper* pWriter )
{
	pWriter->write( m_sName, 256 );


	//Geometry
	m_VB->bin(pWriter);


	//Bones
	pWriter->write( (DWORD)m_vecBones.size() );

	for (unsigned int i = 0; i < m_vecBones.size() ; i++)
		m_vecBones[i].bin(pWriter);


	//Animations
	pWriter->write( (DWORD)m_vecAnimationTakes.size() );

	for (unsigned int i = 0; i < m_vecAnimationTakes.size() ; i++)
		m_vecAnimationTakes[i].bin(pWriter);


	//Materials
	pWriter->write( (DWORD)m_vecMaterials.size() );

	for (unsigned int i = 0; i < m_vecMaterials.size() ; i++)
		m_vecMaterials[i].bin(pWriter);


	//Textures
	pWriter->write( (DWORD)m_vecTextures.size() );

	for (unsigned int i = 0; i < m_vecTextures.size() ; i++)
		m_vecTextures[i].bin(pWriter);


	//Materia Connetion
	m_MaterialConnection.bin( pWriter );

	//Global Matrix
	pWriter->write( m_GlobalMatrix );
}

int GetMappingIndex(KFbxLayerElement::EMappingMode MappingMode, int nPolygonIndex, int nPolygonVertexIndex, int nVertexIndex)
{
	switch(MappingMode)
	{
	case KFbxLayerElement::eALL_SAME:
		return 0;
		break;

	case KFbxLayerElement::eBY_CONTROL_POINT:
		return nVertexIndex;
		break;

	case KFbxLayerElement::eBY_POLYGON_VERTEX:
		return nPolygonIndex*3 + nPolygonVertexIndex;
		break;

	case KFbxLayerElement::eBY_POLYGON:
		return nPolygonIndex;
		break;

	case KFbxLayerElement::eNONE:
	case KFbxLayerElement::eBY_EDGE:
		break;
	}
	return -1;
}

KFbxSurfaceMaterial* GetMaterialLinkedWithPolygon(KFbxMesh* pFBXMesh, int nLayerIndex, int nPolygonIndex, int nPolygonVertexIndex, int nVertexIndex)
{
	if( nLayerIndex < 0 || nLayerIndex > pFBXMesh->GetLayerCount() )
		return NULL;

	KFbxNode* pNode = pFBXMesh->GetNode();

	if( !pNode )
		return NULL;

	KFbxLayerElementMaterial* pFBXMaterial = pFBXMesh->GetLayer(nLayerIndex)->GetMaterials();

	if( pFBXMaterial )
	{
		int nMappingIndex = GetMappingIndex( pFBXMaterial->GetMappingMode(), nPolygonIndex, 0, nVertexIndex );

		if( nMappingIndex < 0 )
			return NULL;

		KFbxLayerElement::EReferenceMode referenceMode = pFBXMaterial->GetReferenceMode();


		if( referenceMode == KFbxLayerElement::eDIRECT )
		{
			if( nMappingIndex < pNode->GetMaterialCount() )
			{
				return pNode->GetMaterial(nMappingIndex);
			}
		}
		else if( referenceMode == KFbxLayerElement::eINDEX_TO_DIRECT )
		{
			const KFbxLayerElementArrayTemplate<int>& pMaterialIndexArray = pFBXMaterial->GetIndexArray();

			if( nMappingIndex < pMaterialIndexArray.GetCount() )
			{
				int nIndex = pMaterialIndexArray.GetAt(nMappingIndex);
				if( nIndex < pNode->GetMaterialCount() )
				{
					return pNode->GetMaterial(nIndex);
				}
			}
		}
	}

	return NULL;
}

Vector2 GetTexCoord(KFbxMesh* pFBXMesh, int nLayerIndex, int nPolygonIndex, int nPolygonVertexIndex, int nVertexIndex)
{
	Vector2 texCoord;

	int nLayerCount = pFBXMesh->GetLayerCount();
	if( nLayerIndex < nLayerCount )//for( int i = 0; i < nLayerCount; ++i )
	{
		KFbxLayer* pFBXLayer = pFBXMesh->GetLayer(nLayerIndex);

		if( pFBXLayer )
		{
			KFbxLayerElementUV* pUVs = pFBXLayer->GetUVs(KFbxLayerElement::eDIFFUSE_TEXTURES);
			if( pUVs )
			{
				KFbxLayerElement::EMappingMode mappingMode = pUVs->GetMappingMode();
				KFbxLayerElement::EReferenceMode referenceMode = pUVs->GetReferenceMode();

				const KFbxLayerElementArrayTemplate<KFbxVector2>& pUVArray = pUVs->GetDirectArray();
				const KFbxLayerElementArrayTemplate<int>& pUVIndexArray = pUVs->GetIndexArray();

				switch(mappingMode)
				{
				case KFbxLayerElement::eBY_CONTROL_POINT:
					{
						int nMappingIndex = nVertexIndex;
						switch(referenceMode)
						{
						case KFbxLayerElement::eDIRECT:
							if( nMappingIndex < pUVArray.GetCount() )
							{
								return Vector2(pUVArray.GetAt(nMappingIndex).mData[0], 1-pUVArray.GetAt(nMappingIndex).mData[1]);
							}
							break;
						case KFbxLayerElement::eINDEX_TO_DIRECT:
							if( nMappingIndex < pUVIndexArray.GetCount() )
							{
								int nIndex = pUVIndexArray.GetAt(nMappingIndex);
								if( nIndex < pUVArray.GetCount() )
								{
									return Vector2(pUVArray.GetAt(nIndex).mData[0], 1-pUVArray.GetAt(nIndex).mData[1]);
								}
							}
							break;
						};
					}
					break;

				case KFbxLayerElement::eBY_POLYGON_VERTEX:
					{
						int nMappingIndex = pFBXMesh->GetTextureUVIndex(nPolygonIndex, nPolygonVertexIndex, KFbxLayerElement::eDIFFUSE_TEXTURES);
						switch(referenceMode)
						{
						case KFbxLayerElement::eDIRECT:
						case KFbxLayerElement::eINDEX_TO_DIRECT: //I have no idea why the index array is not used in this case.
							if( nMappingIndex < pUVArray.GetCount() )
							{
								return Vector2(pUVArray.GetAt(nMappingIndex).mData[0], 1-pUVArray.GetAt(nMappingIndex).mData[1]);
							}
							break;
						};
					}
					break;
				};
			}
		}
	}
	return Vector2();
}


//Helper Functions for processing FBX
void GetGeometryData(KFbxMesh* pMesh, CSkinData* skindata, VertexBuffer* vb)
{
	int i, j, lPolygonCount = pMesh->GetPolygonCount();
	KFbxVector4* lControlPoints = pMesh->GetControlPoints(); 


// 	std::vector<CBoneWeights> boneWeights( pMesh->GetControlPointsCount(), CBoneWeights());
// 	ProcessBoneWeights(pFBXMesh, boneWeights);

	float data[4];

	int vertexId = 0;
	for (i = 0; i < lPolygonCount; i++)
	{
		int l;

		KFbxSkin* pSkin = (KFbxSkin*)pMesh->GetDeformer( 0, KFbxDeformer::eSKIN );
		KFbxCluster* lCluster =pSkin->GetCluster(0);

		//Process Polygons
		int lPolygonSize = pMesh->GetPolygonSize(i);


		for (j = 0; j < lPolygonSize; j++)
		{
			//Get Position
			int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
			

			KFbxSurfaceMaterial* pSurfaceMaterial = GetMaterialLinkedWithPolygon(pMesh, 0, i, j, lControlPointIndex);

			if ( pSurfaceMaterial == NULL )
			{
				printf("Error processing Material Connections[GetMaterialConnections]\n");
			}
			else
			{
				gMaterialMap[pSurfaceMaterial].push_back( vertexId );
			}


			KFbxVector4 lDstVertex = lControlPoints[lControlPointIndex];

			KFbxCluster* lCluster =pSkin->GetCluster(0);
			if (lCluster)
			{
				KFbxXMatrix lMeshGlobal;

				lCluster->GetTransformMatrix(lMeshGlobal);
				KFbxXMatrix lMeshGeometry = GetGeometry(pMesh->GetNode());
				lMeshGlobal *= lMeshGeometry;

				lDstVertex = lMeshGlobal.MultT(lDstVertex);
			}
			else
			{				
				KFbxXMatrix lMeshGeometry = GetGeometry(pMesh->GetNode());
				lDstVertex = lMeshGeometry.MultT(lDstVertex);
			}
			

			vb->m_vecPositions.push_back(lDstVertex.mData);



			//Skinning Data
			if ( skindata->GetBoneCount() != 0 )
			{
				UINT  indices[4];
				float weights[4]; 
				indices[0] = 0;
				indices[1] = 0;
				indices[2] = 0;
				indices[3] = 0;

				weights[0] = 0.0f;
				weights[1] = 0.0f;
				weights[2] = 0.0f;
				weights[3] = 0.0f;


				vb->m_vecBoneWeights.push_back( skindata->GetWeights( lControlPointIndex ) );

				indices[0] = (UINT)skindata->GetIndices( lControlPointIndex )[0];
				indices[1] = (UINT)skindata->GetIndices( lControlPointIndex )[1];
				indices[2] = (UINT)skindata->GetIndices( lControlPointIndex )[2];
				indices[3] = (UINT)skindata->GetIndices( lControlPointIndex )[3];

				vb->m_vecBoneIndices.push_back( indices );
			}

			//Get VertexColor
			for (l = 0; l < pMesh->GetElementVertexColorCount(); l++)
			{
				KFbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor( l);

				switch (leVtxc->GetMappingMode())
				{
				case KFbxGeometryElement::eBY_CONTROL_POINT:
					switch (leVtxc->GetReferenceMode())
					{
					case KFbxGeometryElement::eDIRECT:

						vb->m_vecColors.push_back( leVtxc->GetDirectArray().GetAt(lControlPointIndex) );

						break;

					case KFbxGeometryElement::eINDEX_TO_DIRECT:
						{
							int id = leVtxc->GetIndexArray().GetAt(vertexId);

							vb->m_vecColors.push_back( leVtxc->GetDirectArray().GetAt(id) );
						}
						break;
					default:
						break; // other reference modes not shown here!
					}
					break;

				case KFbxGeometryElement::eBY_POLYGON_VERTEX:
					{
						switch (leVtxc->GetReferenceMode())
						{
						case KFbxGeometryElement::eDIRECT:

							vb->m_vecColors.push_back( leVtxc->GetDirectArray().GetAt(vertexId) );

							break;

						case KFbxGeometryElement::eINDEX_TO_DIRECT:
							{
								int id = leVtxc->GetIndexArray().GetAt(vertexId);

								vb->m_vecColors.push_back( leVtxc->GetDirectArray().GetAt(id) );
							}
							break;

						default:
							break; // other reference modes not shown here!
						}
					}
					break;

				case KFbxGeometryElement::eBY_POLYGON: // doesn't make much sense for UVs
				case KFbxGeometryElement::eALL_SAME:   // doesn't make much sense for UVs
				case KFbxGeometryElement::eNONE:       // doesn't make much sense for UVs
					break;
				}
			}


			vb->m_vecTexCoord.push_back( GetTexCoord(pMesh, 0, i, j, lControlPointIndex) );

			//Get Normals
			for( l = 0; l < pMesh->GetElementNormalCount(); ++l)
			{
				KFbxGeometryElementNormal* leNormal = pMesh->GetElementNormal( l);

				if(leNormal->GetMappingMode() == KFbxGeometryElement::eBY_POLYGON_VERTEX)
				{
					switch (leNormal->GetReferenceMode())
					{
					case KFbxGeometryElement::eDIRECT:

						vb->m_vecNormals.push_back( leNormal->GetDirectArray().GetAt(vertexId) );

						break;

					case KFbxGeometryElement::eINDEX_TO_DIRECT:
						{
							int id = leNormal->GetIndexArray().GetAt(vertexId);

							vb->m_vecNormals.push_back( leNormal->GetDirectArray().GetAt(id) );
						}
						break;
					default:
						break; // other reference modes not shown here!
					}
				}

			}

			//Get Tangents
			for( l = 0; l < pMesh->GetElementTangentCount(); ++l)
			{
				KFbxGeometryElementTangent* leTangent = pMesh->GetElementTangent( l);

				if(leTangent->GetMappingMode() == KFbxGeometryElement::eBY_POLYGON_VERTEX)
				{
					switch (leTangent->GetReferenceMode())
					{
					case KFbxGeometryElement::eDIRECT:

						vb->m_vecTangents.push_back( leTangent->GetDirectArray().GetAt(vertexId) );

						break;
					case KFbxGeometryElement::eINDEX_TO_DIRECT:
						{
							int id = leTangent->GetIndexArray().GetAt(vertexId);

							vb->m_vecTangents.push_back( leTangent->GetDirectArray().GetAt(id) );
						}
						break;
					default:
						break; // other reference modes not shown here!
					}
				}

			}

			//Get BiNormals
			for( l = 0; l < pMesh->GetElementBinormalCount(); ++l)
			{
				KFbxGeometryElementBinormal* leBinormal = pMesh->GetElementBinormal( l);

				if(leBinormal->GetMappingMode() == KFbxGeometryElement::eBY_POLYGON_VERTEX)
				{
					switch (leBinormal->GetReferenceMode())
					{
					case KFbxGeometryElement::eDIRECT:

						vb->m_vecBiNormals.push_back( leBinormal->GetDirectArray().GetAt(vertexId) );

						break;
					case KFbxGeometryElement::eINDEX_TO_DIRECT:
						{
							int id = leBinormal->GetIndexArray().GetAt(vertexId);

							vb->m_vecBiNormals.push_back( leBinormal->GetDirectArray().GetAt(id) );

						}
						break;
					default:
						break; // other reference modes not shown here!
					}
				}
			}
			vertexId++;

		} // for polygonSize
	} // for polygonCount
}

void GetMaterials(  KFbxGeometry* pGeometry, vector<sMaterial__>& materials )
{
	int lMaterialCount = 0;
	KFbxNode* lNode = NULL;
	if(pGeometry)
	{
		lNode = pGeometry->GetNode();
		if(lNode)
			lMaterialCount = lNode->GetMaterialCount();    
	}


	if (lMaterialCount > 0)
	{
		KFbxPropertyDouble3 lKFbxDouble3;
		KFbxPropertyDouble1 lKFbxDouble1;
		KFbxColor theColor;

		for (int lCount = 0; lCount < lMaterialCount; lCount ++)
		{
			sMaterial__ material;

			material.m_HasTexture = false;

			KFbxSurfaceMaterial *lMaterial = lNode->GetMaterial(lCount);

			//Name
			material.m_sName = lMaterial->GetName();

			//Get the implementation to see if it's a hardware shader.
			const KFbxImplementation* lImplementation = GetImplementation(lMaterial, ImplementationHLSL);
			KString lImplemenationType = "HLSL";
			if(!lImplementation)
			{
				lImplementation = GetImplementation(lMaterial, ImplementationCGFX);
				lImplemenationType = "CGFX";
			}
			if(lImplementation)
			{
				material.m_hardwareShader.m_sType = lImplemenationType.Buffer();

				KFbxBindingTable const* lRootTable = lImplementation->GetRootTable();
				fbxString lFileName = lRootTable->DescAbsoluteURL.Get();
				fbxString lTechniqueName = lRootTable->DescTAG.Get(); 

				KFbxBindingTable const* lTable = lImplementation->GetRootTable();
				size_t lEntryNum = lTable->GetEntryCount();

				for(int i=0;i <(int)lEntryNum; ++i)
				{
					const KFbxBindingTableEntry& lEntry = lTable->GetEntry(i);
					const char* lEntrySrcType = lEntry.GetEntryType(true); 
					KFbxProperty lFbxProp;


					KString lTest = lEntry.GetSource();
					material.m_hardwareShader.m_sEntry = lTest.Buffer();


					if ( strcmp( KFbxPropertyEntryView::sEntryType, lEntrySrcType ) == 0 )
					{   
						lFbxProp = lMaterial->FindPropertyHierarchical(lEntry.GetSource()); 
						if(!lFbxProp.IsValid())
						{
							lFbxProp = lMaterial->RootProperty.FindHierarchical(lEntry.GetSource());
						}


					}
					else if( strcmp( KFbxConstantEntryView::sEntryType, lEntrySrcType ) == 0 )
					{
						lFbxProp = lImplementation->GetConstants().FindHierarchical(lEntry.GetSource());
					}
					if(lFbxProp.IsValid())
					{
						if( lFbxProp.GetSrcObjectCount( FBX_TYPE(KFbxTexture) ) > 0 )
						{
							//do what you want with the textures
							for(int j=0; j<lFbxProp.GetSrcObjectCount(FBX_TYPE(KFbxFileTexture)); ++j)
							{
								KFbxFileTexture *lTex = lFbxProp.GetSrcObject(FBX_TYPE(KFbxFileTexture),j);

								sTexture__ texture;
								texture.m_sFilename = lTex->GetFileName();
								texture.m_sType = "Texture";

								material.m_hardwareShader.m_vecTextures.push_back(texture);
							}
							for(int j=0; j<lFbxProp.GetSrcObjectCount(FBX_TYPE(KFbxLayeredTexture)); ++j)
							{
								KFbxLayeredTexture *lTex = lFbxProp.GetSrcObject(FBX_TYPE(KFbxLayeredTexture),j);

								sTexture__ texture;
								texture.m_sFilename = lTex->GetName();
								texture.m_sType = "Layered-Texture";

								material.m_hardwareShader.m_vecTextures.push_back(texture);
							}
							for(int j=0; j<lFbxProp.GetSrcObjectCount(FBX_TYPE(KFbxProceduralTexture)); ++j)
							{
								KFbxProceduralTexture *lTex = lFbxProp.GetSrcObject(FBX_TYPE(KFbxProceduralTexture),j);

								sTexture__ texture;
								texture.m_sFilename = lTex->GetName();
								texture.m_sType = "Procedural-Texture";

								material.m_hardwareShader.m_vecTextures.push_back(texture);
							}
						}
						else{//Todo
// 							KFbxDataType lFbxType = lFbxProp.GetPropertyDataType();
// 							KString name = lFbxType.GetName();
// 							if(DTBool == lFbxType)
// 							{
// 
// 								TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 								xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 								if (KFbxGet <bool> (lFbxProp))
// 									xmlProperty->SetAttribute("value", "true" );
// 								else
// 									xmlProperty->SetAttribute("value", "false" );
// 
// 								xmlImplemention->LinkEndChild(xmlProperty);
// 
// 							}
// 							else if ( DTInteger == lFbxType ||  DTEnum  == lFbxType )
// 							{
// 
// 								TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 								xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 								xmlProperty->SetAttribute("value", KFbxGet <int> (lFbxProp) );
// 
// 								xmlImplemention->LinkEndChild(xmlProperty);
// 
// 							}
// 							else if ( DTFloat == lFbxType)
// 							{
// 
// 								TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 								xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 								stringstream stm;
// 								stm << KFbxGet <float> (lFbxProp);
// 
// 								xmlProperty->SetAttribute("value", stm.str().c_str() );
// 
// 								xmlImplemention->LinkEndChild(xmlProperty);
// 
// 							}
// 							else if ( DTDouble == lFbxType)
// 							{
// 
// 								TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 								xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 								stringstream stm;
// 								stm << KFbxGet <double> (lFbxProp);
// 
// 								xmlProperty->SetAttribute("value", stm.str().c_str() );
// 
// 								xmlImplemention->LinkEndChild(xmlProperty);
// 
// 							}
// 							else if ( DTString == lFbxType
// 								||  DTUrl  == lFbxType
// 								||  DTXRefUrl  == lFbxType )
// 							{
// 
// 								TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 								xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 
// 								xmlProperty->SetAttribute("value", KFbxGet <fbxString> (lFbxProp).Buffer() );
// 
// 								xmlImplemention->LinkEndChild(xmlProperty);
// 
// 							}
// 							else if ( DTDouble2 == lFbxType)
// 							{
// 								fbxDouble2 lDouble2=KFbxGet <fbxDouble2> (lFbxProp);
// 								KFbxVector2 lVect;
// 								lVect[0] = lDouble2[0];
// 								lVect[1] = lDouble2[1];
// 
// 
// 								TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 								xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 
// 								xmlProperty->SetAttribute("value", Vector2String( lVect ).c_str() );
// 
// 								xmlImplemention->LinkEndChild(xmlProperty);
// 
// 							}
// 							else if ( DTVector3D == lFbxType||DTDouble3 == lFbxType || DTColor3 == lFbxType)
// 							{
// 								fbxDouble3 lDouble3 = KFbxGet <fbxDouble3> (lFbxProp);
// 
// 
// 								KFbxVector4 lVect;
// 								lVect[0] = lDouble3[0];
// 								lVect[1] = lDouble3[1];
// 								lVect[2] = lDouble3[2];
// 
// 
// 								TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 								xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 
// 								xmlProperty->SetAttribute("value", Vector2String( lVect ).c_str() );
// 
// 								xmlImplemention->LinkEndChild(xmlProperty);
// 
// 							}
// 
// 							else if ( DTVector4D == lFbxType || DTDouble4 == lFbxType || DTColor4 == lFbxType)
// 							{
// 								fbxDouble4 lDouble4 = KFbxGet <fbxDouble4> (lFbxProp);
// 								KFbxVector4 lVect;
// 								lVect[0] = lDouble4[0];
// 								lVect[1] = lDouble4[1];
// 								lVect[2] = lDouble4[2];
// 								lVect[3] = lDouble4[3];
// 
// 								TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 								xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 
// 								xmlProperty->SetAttribute("value", Vector2String( lVect ).c_str() );
// 
// 								xmlImplemention->LinkEndChild(xmlProperty);
// 
// 							}
// 							else if ( DTDouble44 == lFbxType)
// 							{
// 								fbxDouble44 lDouble44 = KFbxGet <fbxDouble44> (lFbxProp);
// 								for(int j=0; j<4; ++j)
// 								{
// 
// 									KFbxVector4 lVect;
// 									lVect[0] = lDouble44[j][0];
// 									lVect[1] = lDouble44[j][1];
// 									lVect[2] = lDouble44[j][2];
// 									lVect[3] = lDouble44[j][3];
// 
// 									TiXmlElement* xmlProperty = new TiXmlElement("PropertyDataType");
// 									xmlProperty->SetAttribute("name", name.Buffer() );
// 
// 
// 									xmlProperty->SetAttribute("value", Vector2String( lVect ).c_str() );
// 
// 									xmlImplemention->LinkEndChild(xmlProperty);
// 								}
// 							}}
						}
					}   
				}
			}
			//(FBX only supports Phong & Lambert)
			else if (lMaterial->GetClassId().Is(KFbxSurfacePhong::ClassId))
			{
				material.m_sType = "Phong";

				//Ambient Color
				lKFbxDouble3 =((KFbxSurfacePhong *) lMaterial)->Ambient;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				material.m_phong.m_colAmbient = theColor;

				//Diffuse Color
				lKFbxDouble3 =((KFbxSurfacePhong *) lMaterial)->Diffuse;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				material.m_phong.m_colDiffuse = theColor;

				//Specular Color (unique to Phong materials)
				lKFbxDouble3 =((KFbxSurfacePhong *) lMaterial)->Specular;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				material.m_phong.m_colSpecular = theColor;


				//Emissive Color
				lKFbxDouble3 =((KFbxSurfacePhong *) lMaterial)->Emissive;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				material.m_phong.m_colEmissive = theColor;

				//Opacity is Transparency factor now
				lKFbxDouble1 =((KFbxSurfacePhong *) lMaterial)->TransparencyFactor;

				material.m_phong.m_fTransparency = (float)(1.0-lKFbxDouble1.Get());


				//Shininess
				lKFbxDouble1 =((KFbxSurfacePhong *) lMaterial)->Shininess;

				material.m_phong.m_fShininess = (float)lKFbxDouble1.Get();


				//Reflectivity
				lKFbxDouble1 =((KFbxSurfacePhong *) lMaterial)->ReflectionFactor;

				material.m_phong.m_fReflectivity = (float)lKFbxDouble1.Get();
			}
			else if(lMaterial->GetClassId().Is(KFbxSurfaceLambert::ClassId) )
			{
				material.m_sType = "Lambert";

				// We found a Lambert material.
				TiXmlElement* xmlLambertMaterial = new TiXmlElement("Lambert");

				//Ambient Color
				lKFbxDouble3=((KFbxSurfaceLambert *)lMaterial)->Ambient;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				material.m_lambert.m_colAmbient = theColor;

				//Diffuse Color
				lKFbxDouble3 =((KFbxSurfaceLambert *)lMaterial)->Diffuse;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				material.m_lambert.m_colDiffuse = theColor;

				// Display the Emissive
				lKFbxDouble3 =((KFbxSurfaceLambert *)lMaterial)->Emissive;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				material.m_lambert.m_colEmissive = theColor;

				// Display the Opacity
				lKFbxDouble1 =((KFbxSurfaceLambert *)lMaterial)->TransparencyFactor;

				material.m_lambert.m_fTransparency = (float)(1.0-lKFbxDouble1.Get());
			}
			else
			{
				material.m_sType = "Unknown";
			}

			material.m_sShadingModel = lMaterial->ShadingModel.Get().Buffer();
			materials.push_back(material);

		}// for lMaterialCount

	}// if lMaterialCount > 0
}

void GetTextures( KFbxGeometry* pGeometry, vector<sTexture__>& textures )
{
	int lMaterialIndex;
	KFbxProperty lProperty;
	if(pGeometry->GetNode()==NULL)
		return;
	int lNbMat = pGeometry->GetNode()->GetSrcObjectCount(KFbxSurfaceMaterial::ClassId);
	for (lMaterialIndex = 0; lMaterialIndex < lNbMat; lMaterialIndex++)
	{
		KFbxSurfaceMaterial *lMaterial = (KFbxSurfaceMaterial *)pGeometry->GetNode()->GetSrcObject(KFbxSurfaceMaterial::ClassId, lMaterialIndex);
		bool lDisplayHeader = true;

		//go through all the possible textures
		if(lMaterial)
		{
			bool hasTextures = false;

			int lTextureIndex;
			FOR_EACH_TEXTURE(lTextureIndex)
			{
				sTexture__ texture;
				texture.m_dAssignedMaterial = lMaterialIndex;

				hasTextures = true;

				lProperty = lMaterial->FindProperty(KFbxLayerElement::TEXTURE_CHANNEL_NAMES[lTextureIndex]);
				if( lProperty.IsValid() )
				{
					int lTextureCount = lProperty.GetSrcObjectCount(KFbxTexture::ClassId);

					for (int j = 0; j < lTextureCount; ++j)
					{
						//Here we have to check if it's layered textures, or just textures:
						KFbxLayeredTexture *lLayeredTexture = KFbxCast <KFbxLayeredTexture>(lProperty.GetSrcObject(KFbxLayeredTexture::ClassId, j));
						if (lLayeredTexture)
						{
							printf("[Warning] Layered Textures not supported now!\n");
// 							TiXmlElement* xmlLayeredTexture = new TiXmlElement( "Texture" );
// 							xmlLayeredTexture->SetAttribute("type", "LayeredTexture");
// 
// 							KFbxLayeredTexture *lLayeredTexture = KFbxCast <KFbxLayeredTexture>(pProperty.GetSrcObject(KFbxLayeredTexture::ClassId, j));
// 							int lNbTextures = lLayeredTexture->GetSrcObjectCount(KFbxTexture::ClassId);
// 							for(int k =0; k<lNbTextures; ++k)
// 							{
// 								KFbxTexture* lTexture = KFbxCast <KFbxTexture> (lLayeredTexture->GetSrcObject(KFbxTexture::ClassId,k));
// 								if(lTexture)
// 								{
// 									//NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the texture.
// 									//Why is that?  because one texture can be shared on different layered textures and might
// 									//have different blend modes.
// 
// 									KFbxLayeredTexture::EBlendMode lBlendMode;
// 									lLayeredTexture->GetTextureBlendMode(k, lBlendMode);
// 
// 									TiXmlElement* xmlProperty = new TiXmlElement( "Property" );
// 									xmlProperty->SetAttribute( "Filename", (char *) KFbxCast<KFbxFileTexture>(lTexture)->GetFileName() );
// 									xmlProperty->SetAttribute( "Textures-for", pProperty.GetName() );
// 
// 									xmlLayeredTexture->LinkEndChild(xmlProperty);
// 									parent->LinkEndChild( xmlLayeredTexture );
// 								}
// 							}
						}
						else
						{
							//no layered texture simply get on the property
							KFbxTexture* lTexture = KFbxCast <KFbxTexture> (lProperty.GetSrcObject(KFbxTexture::ClassId,j));
							if(lTexture)
							{

								texture.m_sType = "Texture";
								texture.m_sFilename = (char *) KFbxCast<KFbxFileTexture>(lTexture)->GetFileName();
								texture.m_sTextureFor = lProperty.GetName();

								textures.push_back(texture);
							}

						}//lLayeredTexture
					}
				}//end if pProperty

			}//FOR_EACH_TEXTURE

		}//end if(lMaterial)

	}// end for lMaterialIndex    
}

void GetMaterialConnections(KFbxMesh* pMesh, sMaterialConnection__& materialConnection, vector<sMaterial__> materials )
{

	bool foundAssingnedMaterial = false;

	for(map<KFbxSurfaceMaterial*,vector<DWORD>>::iterator it = gMaterialMap.begin(); it != gMaterialMap.end(); it++) 
	{
		sMaterialGroup__ matGroup;
		matGroup.m_sMaterialName = it->first->GetName();

		//Copy Data
		for (unsigned int i = 0; i < it->second.size()/3 ; i++)
		{
			 matGroup.m_vecVertexIndices.push_back( it->second[i*3 ] );
			 matGroup.m_vecVertexIndices.push_back( it->second[i*3+1] );
			 matGroup.m_vecVertexIndices.push_back( it->second[i*3+2] );
		}


		//Search Material for Index
		matGroup.m_dMaterialIndex = -1;
		for (unsigned int i = 0; i < materials.size() ; i++)
		{
			if ( materials[i].m_sName == it->first->GetName() )
			{
				matGroup.m_dMaterialIndex = i; 

				foundAssingnedMaterial = true;
				break;
			}
		}

		if (!foundAssingnedMaterial)
		{
			printf("Error could not find the correct Material[GetMaterialConnections]\n");
		}

		materialConnection.m_vecMaterialGroups.push_back(matGroup);
	}




}

TiXmlElement* sTexture__::xml()
{
	//Converte and save textures
	string converterPath = string(getenv( "BLOCO_HOME" )) + "\\trunk\\bin\\TextureConverter\\texconv.exe";
	string param1 = "\"" + FixFilePath(m_sFilename) + "\"";
	string param2 = "-o \"" + g_sTexturepath + "\"";
	string command = "\"" + converterPath + "\" " + param1 + " " + param2;
	WinExec(command.c_str(), SW_SHOWNORMAL);

	//Change the filename
	string tmp = "textures\\" + GetNameFromFilepath(FixFilePath(m_sFilename)) + ".dds";
	m_sFilename = tmp;

	TiXmlElement* xmlTexture = new TiXmlElement( "Texture" );
	xmlTexture->SetAttribute( "Filename", m_sFilename.c_str() );
	xmlTexture->SetAttribute( "Type", m_sType.c_str() );
	xmlTexture->SetAttribute( "TextureFor",	m_sTextureFor.c_str() );

	stringstream stm;
	stm << m_dAssignedMaterial;
	xmlTexture->SetAttribute( "AssignedMaterialIndex",	stm.str().c_str() );

	return xmlTexture;
}

void sTexture__::bin( IOHelper* pWriter )
{
	//Converte and save textures
	string converterPath = string(getenv( "DXSDK_DIR" )) + "\\Utilities\\bin\\x64\\texconv.exe";
	string param1 = "\"" + FixFilePath(m_sFilename) + "\"";
	string param2 = "-o \"" + g_sTexturepath + "\"";
	string command = "\"" + converterPath + "\" " + param1 + " " + param2;
	WinExec(command.c_str(), SW_SHOWNORMAL);

	//Change the filename
	string tmp = "textures\\" + GetNameFromFilepath(FixFilePath(m_sFilename)) + ".dds";
	m_sFilename = tmp;

	pWriter->write( m_sFilename, 256*2 );
	pWriter->write( m_sType, 32 );
	pWriter->write( m_sTextureFor, 256 );

	pWriter->write( m_dAssignedMaterial );
}
