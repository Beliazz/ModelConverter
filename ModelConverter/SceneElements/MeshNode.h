#ifndef MeshNode_H__
#define MeshNode_H__

#include "..\main.h"


class VertexBuffer
{
public:
	VertexBuffer(){}
	~VertexBuffer(){}

	DWORD  Size() { return m_vecPositions.size(); }

	TiXmlElement* xml()
	{
		TiXmlElement* xmlVB = new TiXmlElement( "VertexBuffer" );
		xmlVB->SetAttribute( "Size", m_vecPositions.size() );

		TiXmlElement* xmlPosition	= new TiXmlElement( "Position" );
		TiXmlElement* xmlNormal		= new TiXmlElement( "Normal" );
		TiXmlElement* xmlTexCoord	= new TiXmlElement( "TexCoord" );
		TiXmlElement* xmlColor		= new TiXmlElement( "Color" );
		TiXmlElement* xmlTangent	= new TiXmlElement( "Tangent" );
		TiXmlElement* xmlBiNormal	= new TiXmlElement( "BiNormal" );
		TiXmlElement* xmlBoneWeight	= new TiXmlElement( "BoneWeight" );
		TiXmlElement* xmlBoneIndex	= new TiXmlElement( "BoneIndex" );


		string strPosition	 = "";
		string strNormal	 = "";
		string strTexCoord	 = "";
		string strColor		 = "";
		string strTangent	 = "";
		string strBiNormal	 = "";
		string strBoneWeight = "";
		string strBoneIndex	 = "";

		for (unsigned int i = 0; i < m_vecPositions.size() ; i++)
		{
			stringstream stm_boneindex;

			//Must have
			strPosition	  += m_vecPositions[i].str();

			//Intivitual
			if (!m_vecNormals.empty())
				strNormal += m_vecNormals[i].str();
		
			if (!m_vecTexCoord.empty())
				strTexCoord += m_vecTexCoord[i].str();

			if (!m_vecColors.empty())
				strTexCoord += m_vecColors[i].str();

			if (!m_vecColors.empty())
				strColor += m_vecColors[i].str();

			if (!m_vecTangents.empty())
				strTangent += m_vecTangents[i].str();


			if (!m_vecBiNormals.empty())
				strBiNormal += m_vecBiNormals[i].str();

			if (!m_vecBoneWeights.empty())
				strBoneWeight += m_vecBoneWeights[i].str();
			
			if (!m_vecBoneWeights.empty())
			{
				strBoneIndex  += m_vecBoneIndices[i].str();
			}
		}


		xmlPosition->LinkEndChild(	 xmlText(strPosition) );

		xmlVB->LinkEndChild( xmlPosition );

		if (strNormal != "")
		{
			xmlNormal->LinkEndChild( xmlText(strNormal) );
			xmlVB->LinkEndChild( xmlNormal );
		}

		if (strTexCoord != "")
		{
			xmlTexCoord->LinkEndChild( xmlText(strTexCoord) );
			xmlVB->LinkEndChild( xmlTexCoord );
		}

		if (strColor != "")
		{
			xmlColor->LinkEndChild( xmlText(strColor) );
			xmlVB->LinkEndChild( xmlColor );
		}

		if (strTangent != "")
		{
			xmlTangent->LinkEndChild( xmlText(strTangent) );
			xmlVB->LinkEndChild( xmlTangent );
		}

		if (strBiNormal != "")
		{
			xmlBiNormal->LinkEndChild( xmlText(strBiNormal) );
			xmlVB->LinkEndChild( xmlBiNormal );
		}

		if (strBoneWeight != "")
		{
			xmlBoneWeight->LinkEndChild( xmlText(strBoneWeight) );
			xmlVB->LinkEndChild( xmlBoneWeight );
		}

		if (strBoneIndex != "")
		{
			xmlBoneIndex->LinkEndChild( xmlText(strBoneIndex) );
			xmlVB->LinkEndChild( xmlBoneIndex );
		}

		return xmlVB;
	}
	void  bin( IOHelper* pWriter )
	{
		pWriter->write( (DWORD)m_vecPositions.size() );
		pWriter->write( (char*)m_vecPositions.data(), sizeof(float)*3, m_vecPositions.size() );

		pWriter->write( (DWORD)m_vecNormals.size() );
		pWriter->write( (char*)m_vecNormals.data(), sizeof(float)*3, m_vecNormals.size() );

		pWriter->write( (DWORD)m_vecColors.size() );
		pWriter->write( (char*)m_vecColors.data(), sizeof(float)*4, m_vecColors.size() );

		pWriter->write( (DWORD)m_vecTexCoord.size() );
		pWriter->write( (char*)m_vecTexCoord.data(), sizeof(float)*2, m_vecTexCoord.size() );

		pWriter->write( (DWORD)m_vecTangents.size() );
		pWriter->write( (char*)m_vecTangents.data(), sizeof(float)*3, m_vecTangents.size() );

		pWriter->write( (DWORD)m_vecBiNormals.size() );
		pWriter->write( (char*)m_vecBiNormals.data(), sizeof(float)*3, m_vecBiNormals.size() );

		pWriter->write( (DWORD)m_vecBoneWeights.size() );
		pWriter->write( (char*)m_vecBoneWeights.data(), sizeof(float)*4, m_vecBoneWeights.size() );

		pWriter->write( (DWORD)m_vecBoneIndices.size() );
		pWriter->write( (char*)m_vecBoneIndices.data(), sizeof(UINT)*4, m_vecBoneIndices.size() );
	}

	vector<Vector3> m_vecPositions;
	vector<Vector3> m_vecNormals;
	vector<Color>	m_vecColors;
	vector<Vector2> m_vecTexCoord;
	vector<Vector3> m_vecTangents;
	vector<Vector3> m_vecBiNormals;
	vector<Vector>	m_vecBoneWeights;
	vector<VectorUINT>	m_vecBoneIndices;
};

struct sMeshBone
{
	string	   m_sName;
	string	   m_sParentName;
	KFbxMatrix m_GlobalMatrix;
	KFbxMatrix m_BindPoseMatrix;

	TiXmlElement* xml()
	{
		TiXmlElement* xmlBone = new TiXmlElement( "Bone" );
		xmlBone->SetAttribute( "Name", m_sName.c_str() );
		xmlBone->SetAttribute( "Parent", m_sParentName.c_str() );

		TiXmlElement* xmlGlobalMatrix	= new TiXmlElement( "GlobalMatrix" );
		TiXmlElement* xmlBindPoseMatrix = new TiXmlElement( "BindPoseMatrix" );

		xmlGlobalMatrix->LinkEndChild(	 xmlText( m_GlobalMatrix ) );
		xmlBindPoseMatrix->LinkEndChild( xmlText( m_BindPoseMatrix ) );

		xmlBone->LinkEndChild( xmlGlobalMatrix );
		xmlBone->LinkEndChild( xmlBindPoseMatrix );
		
		return xmlBone;
	}
	void  bin( IOHelper* pWriter )
	{
		pWriter->write(m_sName, 256 );
		pWriter->write( m_sParentName, 256 );

		pWriter->write( m_GlobalMatrix );
		pWriter->write( m_BindPoseMatrix );
	}
};

struct sAnimationsKey__
{
	string m_sInfulencedBone;
	DWORD  m_dBoneIndex;
	float  m_fTimestamp;
	KFbxMatrix m_Matrix;

	TiXmlElement* xml()
	{
		TiXmlElement* xmlAnimKey = new TiXmlElement( "AnimationKey" );
		xmlAnimKey->SetAttribute( "InfulencedBone", m_sInfulencedBone.c_str() );

		stringstream stm;
		stm << m_fTimestamp;
		xmlAnimKey->SetAttribute( "Timestamp", stm.str().c_str() );
		
		TiXmlElement* xmlKeyMatrix = new TiXmlElement( "KeyMatrix" );
		xmlKeyMatrix->LinkEndChild( xmlText( m_Matrix ) );

		xmlAnimKey->LinkEndChild( xmlKeyMatrix );

		return xmlAnimKey;
	}
	void  bin( IOHelper* pWriter )
	{
		pWriter->write( m_dBoneIndex );
		pWriter->write( m_fTimestamp );

		pWriter->write( m_Matrix );
	}
};

struct sAnimationsTake__
{
	string m_sName;
	vector<sAnimationsKey__> m_vecAnimationKeys;

	float m_fStart;
	float m_fEnd;

	TiXmlElement* xml()
	{
		TiXmlElement* xmlAnimTake = new TiXmlElement( "AnimationTake" );
		xmlAnimTake->SetAttribute( "Size", m_vecAnimationKeys.size() );
		xmlAnimTake->SetAttribute( "Name",		m_sName.c_str() );

		stringstream stm;
		stm << m_fStart;
		xmlAnimTake->SetAttribute( "StartTime", stm.str().c_str() );

		stm = stringstream("");
		stm << m_fEnd;
		xmlAnimTake->SetAttribute( "EndTime", stm.str().c_str() );


		for (unsigned int i = 0; i < m_vecAnimationKeys.size() ; i++)
		{
			xmlAnimTake->LinkEndChild( m_vecAnimationKeys[i].xml() );
		}

		return xmlAnimTake;
	}
	void  bin( IOHelper* pWriter )
	{

		pWriter->write( m_sName.c_str(), 256 );
		pWriter->write( m_fStart );
		pWriter->write( m_fEnd );

		pWriter->write( (DWORD)m_vecAnimationKeys.size() );

		for (unsigned int i = 0; i < m_vecAnimationKeys.size() ; i++)
			m_vecAnimationKeys[i].bin(pWriter);

	}
};

struct sTexture__
{
	string m_sType;
	string m_sFilename;
	string m_sTextureFor;
	DWORD m_dAssignedMaterial;

	TiXmlElement* xml();
	void  bin( IOHelper* pWriter );
};

struct Hardwareshader__
{
	string m_sType;
	string m_sEntry;
	vector<sTexture__> m_vecTextures;
};

struct sPhong__
{
	Color m_colAmbient;
	Color m_colDiffuse;
	Color m_colSpecular;
	Color m_colEmissive;

	float m_fTransparency;
	float m_fShininess;
	float m_fReflectivity;

	TiXmlElement* xml()
	{
		TiXmlElement* xmlPhong = new TiXmlElement( "Phong" );
		
		TiXmlElement* xmlAmbient  = new TiXmlElement( "Ambient" );
		TiXmlElement* xmlDiffuse  = new TiXmlElement( "Diffuse" );
		TiXmlElement* xmlSpecular = new TiXmlElement( "Specular" );
		TiXmlElement* xmlEmissive		= new TiXmlElement( "Emissive" );
		TiXmlElement* xmlTransparency	= new TiXmlElement( "Transparency" );
		TiXmlElement* xmlShininess		= new TiXmlElement( "Shininess" );
		TiXmlElement* xmlReflectivity	= new TiXmlElement( "Reflectivity" );

		xmlAmbient->LinkEndChild( xmlText( m_colAmbient ) );
		xmlDiffuse->LinkEndChild( xmlText( m_colDiffuse ) );
		xmlSpecular->LinkEndChild( xmlText( m_colSpecular ) );
		xmlEmissive->LinkEndChild( xmlText( m_colEmissive ) );

		stringstream stm;
		stm << m_fTransparency;
		xmlTransparency->LinkEndChild( xmlText( stm.str().c_str() ) );

		stm = stringstream("");
		stm << m_fShininess;
		xmlShininess->LinkEndChild( xmlText( stm.str().c_str() ) );

		stm = stringstream("");
		stm << m_fReflectivity;
		xmlReflectivity->LinkEndChild( xmlText( stm.str().c_str() ) );

		xmlPhong->LinkEndChild( xmlAmbient );
		xmlPhong->LinkEndChild( xmlDiffuse );
		xmlPhong->LinkEndChild( xmlSpecular );
		xmlPhong->LinkEndChild( xmlEmissive );
		xmlPhong->LinkEndChild( xmlTransparency );
		xmlPhong->LinkEndChild( xmlShininess );
		xmlPhong->LinkEndChild( xmlReflectivity );

		return xmlPhong;
	}
	void  bin( IOHelper* pWriter )
	{
		pWriter->write( m_colAmbient );
		pWriter->write( m_colDiffuse );
		pWriter->write( m_colSpecular );
		pWriter->write( m_colEmissive );

		pWriter->write( m_fTransparency );
		pWriter->write( m_fShininess );
		pWriter->write( m_fReflectivity );
	}
};

struct sLambert__
{
	Color m_colAmbient;
	Color m_colDiffuse;
	Color m_colEmissive;

	float m_fTransparency;

	TiXmlElement* xml()
	{
		TiXmlElement* xmlLambert = new TiXmlElement( "Lambert" );

		TiXmlElement* xmlAmbient  = new TiXmlElement( "Ambient" );
		TiXmlElement* xmlDiffuse  = new TiXmlElement( "Diffuse" );
		TiXmlElement* xmlEmissive		= new TiXmlElement( "Emissive" );
		TiXmlElement* xmlTransparency	= new TiXmlElement( "Transparency" );

		xmlAmbient->LinkEndChild( xmlText( m_colAmbient ) );
		xmlDiffuse->LinkEndChild( xmlText( m_colDiffuse ) );
		xmlEmissive->LinkEndChild( xmlText( m_colEmissive ) );

		stringstream stm;
		stm << m_fTransparency;
		xmlTransparency->LinkEndChild( xmlText( stm.str().c_str() ) );


		xmlLambert->LinkEndChild( xmlAmbient );
		xmlLambert->LinkEndChild( xmlDiffuse );
		xmlLambert->LinkEndChild( xmlEmissive );
		xmlLambert->LinkEndChild( xmlTransparency );

		return xmlLambert;
	}
	void  bin( IOHelper* pWriter )
	{
		pWriter->write( m_colAmbient );
		pWriter->write( m_colDiffuse );
		pWriter->write( m_colEmissive );

		pWriter->write( m_fTransparency );
	}
};

struct sMaterial__
{
	string m_sName;
	Hardwareshader__ m_hardwareShader;
	string m_sType;
	string m_sShadingModel;
	
	bool m_HasTexture;
	DWORD m_AssignedTexture;

	sPhong__	m_phong;
	sLambert__	m_lambert;

	TiXmlElement* xml()
	{
		TiXmlElement* xmlMaterial = new TiXmlElement( "Material" );
		xmlMaterial->SetAttribute( "Name", m_sName.c_str() );
		xmlMaterial->SetAttribute( "Type", m_sType.c_str() );
		xmlMaterial->SetAttribute( "ShadingModel", m_sShadingModel.c_str() );

		if (m_HasTexture)
		{
			xmlMaterial->SetAttribute( "HasTexture", "true" );
			xmlMaterial->SetAttribute( "TextureIndex", m_AssignedTexture );
		}
		else
		{
			xmlMaterial->SetAttribute( "HasTexture", "false" );
		}

		if (m_sType == "Phong" )
		{
			xmlMaterial->LinkEndChild( m_phong.xml() );
		}
		else if ( m_sType == "Lambert" )
		{
			xmlMaterial->LinkEndChild( m_lambert.xml() );
		}


		return xmlMaterial;
	}
	void  bin( IOHelper* pWriter )
	{
		pWriter->write( m_sName.c_str(), 256 );
		pWriter->write( m_sType.c_str(), 32 );
		pWriter->write( m_sShadingModel.c_str(), 256 );

		pWriter->write( m_HasTexture );

		if (m_HasTexture)
			pWriter->write( m_AssignedTexture );


		if (m_sType == "Phong" )
		{
			m_phong.bin(pWriter);
		}
		else if ( m_sType == "Lambert" )
		{
			m_lambert.bin(pWriter);
		}
	}
};

struct sMaterialGroup__
{
	DWORD  m_dMaterialIndex;
	string m_sMaterialName;
	vector<DWORD> m_vecVertexIndices;

	Vector3	m_sizeMax;
	Vector3	m_sizeMin;

	void CalculateAABB( VertexBuffer* buffer )
	{
		m_sizeMax = Vector3(-99999.0f,-99999.0f,-99999.0f);
		m_sizeMin = Vector3(99999.0f,99999.0f,99999.0f);

		for (unsigned int i = 0; i < m_vecVertexIndices.size() ; i++)
		{
			//Minimum
			if ( m_sizeMax.x < buffer->m_vecPositions[ m_vecVertexIndices[i] ].x )
				m_sizeMax.x = buffer->m_vecPositions[ m_vecVertexIndices[i] ].x;

			if ( m_sizeMax.y < buffer->m_vecPositions[ m_vecVertexIndices[i] ].y )
				m_sizeMax.y = buffer->m_vecPositions[ m_vecVertexIndices[i] ].y;

			if ( m_sizeMax.z < buffer->m_vecPositions[ m_vecVertexIndices[i] ].z )
				m_sizeMax.z = buffer->m_vecPositions[ m_vecVertexIndices[i] ].z;

			//Maximum
			if ( m_sizeMin.x > buffer->m_vecPositions[ m_vecVertexIndices[i] ].x )
				m_sizeMin.x = buffer->m_vecPositions[ m_vecVertexIndices[i] ].x;

			if ( m_sizeMin.y > buffer->m_vecPositions[ m_vecVertexIndices[i] ].y )
				m_sizeMin.y = buffer->m_vecPositions[ m_vecVertexIndices[i] ].y;

			if ( m_sizeMin.z > buffer->m_vecPositions[ m_vecVertexIndices[i] ].z )
				m_sizeMin.z = buffer->m_vecPositions[ m_vecVertexIndices[i] ].z;
		}
	}

	TiXmlElement* xml()
	{
		TiXmlElement* xmlMaterialGroup = new TiXmlElement( "MaterialGroup" );
		xmlMaterialGroup->SetAttribute( "MaterialName", m_sMaterialName.c_str() );
		
		stringstream stm;
		stm << m_dMaterialIndex;
		xmlMaterialGroup->SetAttribute( "MaterialIndex", stm.str().c_str() );

		TiXmlElement* xmlInfluencedPolygons = new TiXmlElement( "InfluencedPolygons" );
		xmlInfluencedPolygons->SetAttribute( "Size", m_vecVertexIndices.size() );
	
		stm = stringstream("");

		for (unsigned int i = 0; i < m_vecVertexIndices.size() ; i++)
		{
			stm << m_vecVertexIndices[i] << " ";
		}
		
		//Bounding Box
		TiXmlElement* xmlBoundingBox = new TiXmlElement( "Bounding-Box" );
		TiXmlElement* xmlBoundingBoxMin = new TiXmlElement( "Min" );
		TiXmlElement* xmlBoundingBoxMax = new TiXmlElement( "Max" );

		xmlBoundingBoxMin->LinkEndChild( xmlText(m_sizeMin.str()) );
		xmlBoundingBoxMax->LinkEndChild( xmlText(m_sizeMax.str()) );

		xmlBoundingBox->LinkEndChild( xmlBoundingBoxMax );
		xmlBoundingBox->LinkEndChild( xmlBoundingBoxMin );

		
		xmlInfluencedPolygons->LinkEndChild( xmlText( stm.str().c_str() ) );
		xmlMaterialGroup->LinkEndChild( xmlInfluencedPolygons );

		return xmlMaterialGroup;
	}
	void  bin( IOHelper* pWriter )
	{
		pWriter->write( m_sMaterialName.c_str(), 256 );
		pWriter->write( m_dMaterialIndex );

		pWriter->write( (DWORD)m_vecVertexIndices.size() );
		pWriter->write( (char*)m_vecVertexIndices.data(), sizeof(DWORD), m_vecVertexIndices.size() );

		pWriter->write( (char*)&m_sizeMax, sizeof(float), 3 );
		pWriter->write( (char*)&m_sizeMin, sizeof(float), 3 );
	}
};

struct sMaterialConnection__
{
	string m_sType;
	vector<sMaterialGroup__> m_vecMaterialGroups;

	TiXmlElement* xml()
	{
		TiXmlElement* xmlMaterialConnections = new TiXmlElement( "Material-Connections" );
		xmlMaterialConnections->SetAttribute( "Size", m_vecMaterialGroups.size() );

		for (unsigned int i = 0; i < m_vecMaterialGroups.size() ; i++)
		{
			xmlMaterialConnections->LinkEndChild( m_vecMaterialGroups[i].xml() );
		}

		return xmlMaterialConnections;
	}
	void  bin( IOHelper* pWriter )
	{
		pWriter->write( m_sType.c_str(), 32 );

		pWriter->write( (DWORD)m_vecMaterialGroups.size() );

		for (unsigned int i = 0; i < m_vecMaterialGroups.size() ; i++)
			m_vecMaterialGroups[i].bin(pWriter);
	}
};

class CMeshNode : public ModelElement
{
public:
	CMeshNode();

	virtual bool VLoad( KFbxNode* pNode, KFbxScene* pScene );
	virtual bool VLoad( TiXmlElement* pScene );

	virtual string GetType() { return "MeshNode"; }
	virtual string str();

	virtual TiXmlElement* xml();
	virtual void bin( IOHelper* pWriter );

private:
	virtual void Delete();

	string		  m_sName;
	VertexBuffer* m_VB;
	vector<sMeshBone> m_vecBones;
	vector<sAnimationsTake__> m_vecAnimationTakes;
	vector<sMaterial__> m_vecMaterials;
	vector<sTexture__> m_vecTextures;
	sMaterialConnection__ m_MaterialConnection;
};

#endif
