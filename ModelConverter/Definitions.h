#ifndef DEFINITION_H__
#define DEFINITION_H__

#include "main.h"

//Maps
extern map<KFbxNode*,KFbxMatrix> g_GlobalMap;
extern map<KFbxNode*,KFbxMatrix> g_BindPosMap;


//Save & Delete 
template<typename T> void safe_delete(T*& a) 
{
	if (a)
	{
		delete a;
		a = NULL;
	}
}

class CBone
{
public:
	string    sName;
	int		  iParentIndex;
	string    sParentName;
	KFbxNode *pNode;

	bool operator<(CBone bone) { return iParentIndex < bone.iParentIndex; }
};

class CSkinData
{
public:

	vector<CBone> vecBones;
	DWORD dwVertexCount;
	DWORD dwVertexStride;
	BYTE* pBoneIndices;
	FLOAT* pBoneWeights;

	CSkinData()
		: pBoneWeights( NULL ),
		pBoneIndices( NULL ),
		dwVertexCount( 0 ),
		dwVertexStride( 0 )
	{
	}

	~CSkinData()
	{
		if( pBoneIndices != NULL ) delete[] pBoneIndices;
		if( pBoneWeights != NULL ) delete[] pBoneWeights;
	}

	VOID Alloc( DWORD dwCount, DWORD dwStride )
	{
		dwVertexCount = dwCount;
		dwVertexStride = dwStride;

		DWORD dwBufferSize = dwVertexCount * dwVertexStride;
		pBoneIndices = new BYTE[ dwBufferSize ];
		ZeroMemory( pBoneIndices, sizeof(BYTE) * dwBufferSize );
		pBoneWeights = new FLOAT[ dwBufferSize ];
		ZeroMemory( pBoneWeights, sizeof(FLOAT) * dwBufferSize );
	}


	BYTE*  GetIndices( DWORD dwIndex ) 
	{ 
		assert( dwIndex < dwVertexCount ); 
		return pBoneIndices + ( dwIndex * dwVertexStride ); 
	}
	FLOAT* GetWeights( DWORD dwIndex ) 
	{ 
		assert( dwIndex < dwVertexCount ); 
		return pBoneWeights + ( dwIndex * dwVertexStride ); 
	}

	int  GetRoot( KFbxNode* node )
	{
		int index = FindBone( node );

		if(index != -1 )
			return index;

		for (int i = 0; i < node->GetChildCount() ; i++)
		{
			int  _index = GetRoot( node->GetChild( i ) );

			if(_index != -1 )
				return _index;
		}

		return -1;
	}

	void FillHierarchy( CBone* parentBone, KFbxNode* parent )
	{
		for (int i = 0; i < parent->GetChildCount() ; i++)
		{
			int index = FindBone( parent->GetChild(i) );

			if ( index != -1 )
			{
				GetBone(index)->iParentIndex = FindBone( parentBone->pNode );
				GetBone(index)->sParentName  = parentBone->sName;

				FillHierarchy( GetBone(index) );
			}
			else
			{
				FillHierarchy( parentBone, parent->GetChild(i) );
			}
		}
	}

	void FillHierarchy( CBone* parentBone )
	{
		for (int i = 0; i < parentBone->pNode->GetChildCount() ; i++)
		{
			int index = FindBone( parentBone->pNode->GetChild(i) );

			if ( index != -1 )
			{
				GetBone(index)->iParentIndex = FindBone( parentBone->pNode );
				GetBone(index)->sParentName  = parentBone->sName;

				FillHierarchy( GetBone(index) );
			}
			else
			{
				FillHierarchy( parentBone, parentBone->pNode->GetChild(i) );
			}
		}
	}

	void SetHierarchy( KFbxNode* root )
	{
		int rootIndex = 0;
		CBone* rootBone = NULL;

		for (int i = 0; i < root->GetChildCount() ; i++)
		{
			rootIndex = GetRoot( root->GetChild(i) );

			if (rootIndex != -1 )
			{
				rootBone = GetBone(rootIndex);
				rootBone->sParentName = "Root";
				rootBone->iParentIndex = -2;
				break;
			}
		}

		if (rootBone)
		{
			FillHierarchy( rootBone );
		}
		else
		{
			printf("Fatal Error: can not find root Bone!\n");
		}
	}
	int  FindBone( string name ) 
	{
		for (unsigned int i = 0; i < GetBoneCount() ; i++)
		{
			if ( name == GetBoneName(i) )
			{
				return i;
			}
		}

		return -1;
	}
	int  FindBone( KFbxNode* node ) 
	{
		for (unsigned int i = 0; i < GetBoneCount() ; i++)
		{
			if ( node->GetName() == GetBoneName(i) )
			{
				return i;
			}
		}

		return -1;
	}

	void	  AddBone( CBone bone )						{ vecBones.push_back(bone);}
	CBone*	  GetBone( unsigned int index )				{ return &vecBones[index]; }
	string	  GetBoneName( unsigned int index )			{ return vecBones[index].sName; }
	string	  GetBoneParentName( unsigned int index )	{ return vecBones[index].sParentName; }
	DWORD	  GetBoneParendIndex( unsigned int index )	{ return vecBones[index].iParentIndex; }
	KFbxNode* GetBoneNode( unsigned int index )			{ return vecBones[index].pNode; }
	DWORD	  GetBoneCount() const						{ return (DWORD)vecBones.size(); }

	void Sort()
	{
		sort(vecBones.begin(), vecBones.end());
	}
	VOID InsertWeight( DWORD dwIndex, DWORD dwBoneIndex, FLOAT fBoneWeight )
	{
		assert( dwBoneIndex < 256 );

		BYTE* pIndices = GetIndices( dwIndex );
		FLOAT* pWeights = GetWeights( dwIndex );

		for( DWORD i = 0; i < dwVertexStride; ++i )
		{
			if( fBoneWeight > pWeights[i] )
			{
				for( DWORD j = (dwVertexStride - 1); j > i; --j )
				{
					pIndices[j] = pIndices[j - 1];
					pWeights[j] = pWeights[j - 1];
				}
				pIndices[i] = (BYTE)dwBoneIndex;
				pWeights[i] = fBoneWeight;
				break;
			}
		}
	}
};

class ModelElement;

class CBoneWeights
{
public:
	CBoneWeights();

	void AddBoneWeight(int nBoneIndex, float fBoneWeight);
	inline void AddBoneWeight(std::pair<int,float> boneWeightPair) { AddBoneWeight(boneWeightPair.first, boneWeightPair.second); }
	inline void AddBoneWeights(const CBoneWeights& boneWeights) { for( int i = 0; i < boneWeights.GetBoneWeightCount(); ++i)	{ AddBoneWeight(boneWeights.GetBoneWeight(i)); } }

	void Validate();
	void Normalize();

	inline int GetBoneWeightCount() const						 { return m_boneWeights.size(); }
	inline std::pair<int,float> GetBoneWeight(int nIndex) const  { return m_boneWeights.at(nIndex); }
protected:
	std::vector<std::pair<int,float>> m_boneWeights;	// <BoneIndex,BoneWeight>
	float m_fWeightSum;
};

void ProcessBoneWeights(KFbxSkin* pFBXSkin, std::vector<CBoneWeights>& meshBoneWeights, CSkinData* skindata );

//--------------------------------------------------------------------------------------
void ProcessBoneWeights(KFbxMesh* pFBXMesh, std::vector<CBoneWeights>& meshBoneWeights, CSkinData* skindata );


bool ParseMeshSkinning( KFbxMesh* pMesh, CSkinData* pSkinData, KFbxScene* pScene );

extern KFbxXMatrix GetPoseMatrix( KFbxPose* pPose, int pNodeIndex );

extern KFbxXMatrix GetGeometry( KFbxNode* pNode );

extern KFbxXMatrix GetGlobalPosition( KFbxNode* pNode, const KTime& pTime, KFbxPose* pPose = NULL, KFbxXMatrix* pParentGlobalPosition = NULL );

extern ModelElement* ProcessFBXSceneElement( KFbxNode* pNode, KFbxScene* pScene );

extern KFbxSdkManager* g_pSdkManager;

#endif