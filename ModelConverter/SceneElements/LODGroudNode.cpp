#include "LODGroudNode.h"


CLODGroudNode::CLODGroudNode()
{
}

bool CLODGroudNode::VLoad( KFbxNode* pNode )
{
	if (!pNode)
		return false;

	return true;
}

bool CLODGroudNode::VLoad( TiXmlElement* pScene )
{

	return true;
}

void CLODGroudNode::Delete()
{

}