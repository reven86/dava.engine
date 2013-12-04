
#include "Render/Highlevel/SpatialTree.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
QuadTree::QuadTreeNode::QuadTreeNode()
{
	Reset();
}

void QuadTree::QuadTreeNode::Reset()
{
	parent=INVALID_TREE_NODE_INDEX;
	for (int32 i=0; i<4; i++)
		children[i] = INVALID_TREE_NODE_INDEX;	
	nodeInfo = 0;
	
}

QuadTree::QuadTree(const AABBox3& _worldBox, int32 _maxTreeDepth): worldBox(_worldBox), maxTreeDepth(_maxTreeDepth)
{	
	QuadTreeNode root;
	
	root.bbox = worldBox;
	nodes.push_back(root);
    
}

bool QuadTree::CheckBoxIntersectBranch(const AABBox3& objBox, float32 xmin, float32 ymin, float32 xmax, float32 ymax)
{
	
	if ((xmin>objBox.max.x)||(ymin>objBox.max.y)||(xmax<objBox.min.x)||(ymax<objBox.min.y))
		return false;
	return true;
}

bool QuadTree::CheckObjectFitNode(const AABBox3& objBox, const AABBox3& nodeBox)
{
	return (objBox.min.x>=nodeBox.min.x)&&(objBox.min.y>=nodeBox.min.y)&&(objBox.max.x<=nodeBox.max.x)&&(objBox.max.y<=nodeBox.max.y);
}

bool QuadTree::CheckBoxIntersectChild(const AABBox3& objBox, const AABBox3& nodeBox, QuadTreeNode::eNodeType nodeType)
{
	//note - this code assumes box already intersects parent
	switch (nodeType)
	{
	case QuadTreeNode::NODE_LB:
		return (0.5f*(nodeBox.min.x+nodeBox.max.x)>=objBox.min.x)&&(0.5f*(nodeBox.min.y+nodeBox.max.y)>=objBox.min.y);		
	case QuadTreeNode::NODE_RB:
		return (0.5f*(nodeBox.min.x+nodeBox.max.x)<=objBox.max.x)&&(0.5f*(nodeBox.min.y+nodeBox.max.y)>=objBox.min.y);		
	case QuadTreeNode::NODE_LT:
		return (0.5f*(nodeBox.min.x+nodeBox.max.x)>=objBox.min.x)&&(0.5f*(nodeBox.min.y+nodeBox.max.y)<=objBox.max.y);		
	case QuadTreeNode::NODE_RT:
		return (0.5f*(nodeBox.min.x+nodeBox.max.x)<=objBox.max.x)&&(0.5f*(nodeBox.min.y+nodeBox.max.y)<=objBox.max.y);		
	}			
	return false;
}

void QuadTree::UpdateChildBox(AABBox3 &parentBox, QuadTreeNode::eNodeType childType)
{
	switch (childType)
	{
	case  QuadTreeNode::NODE_LB:
		parentBox.max.x = (parentBox.max.x+parentBox.min.x)*0.5f;
		parentBox.max.y = (parentBox.max.y+parentBox.min.y)*0.5f;
		break;
	case  QuadTreeNode::NODE_RB:
		parentBox.min.x = (parentBox.max.x+parentBox.min.x)*0.5f;
		parentBox.max.y = (parentBox.max.y+parentBox.min.y)*0.5f;
		break;
	case  QuadTreeNode::NODE_LT:
		parentBox.max.x = (parentBox.max.x+parentBox.min.x)*0.5f;
		parentBox.min.y = (parentBox.max.y+parentBox.min.y)*0.5f;
		break;
	case  QuadTreeNode::NODE_RT:
		parentBox.min.x = (parentBox.max.x+parentBox.min.x)*0.5f;
		parentBox.min.y = (parentBox.max.y+parentBox.min.y)*0.5f;
		break;
	}
}

void QuadTree::UpdateParentBox(AABBox3 &childtBox, QuadTreeNode::eNodeType childType)
{
	switch (childType)
	{
	case  QuadTreeNode::NODE_LB:
		childtBox.max.x += (childtBox.max.x-childtBox.min.x);
		childtBox.max.y += (childtBox.max.y-childtBox.min.y);
		break;
	case  QuadTreeNode::NODE_RB:
		childtBox.min.x -= (childtBox.max.x-childtBox.min.x);
		childtBox.max.y += (childtBox.max.y-childtBox.min.y);
		break;
	case  QuadTreeNode::NODE_LT:
		childtBox.max.x += (childtBox.max.x-childtBox.min.x);
		childtBox.min.y -= (childtBox.max.y-childtBox.min.y);
		break;
	case  QuadTreeNode::NODE_RT:
		childtBox.min.x -= (childtBox.max.x-childtBox.min.x);
		childtBox.min.y -= (childtBox.max.y-childtBox.min.y);
		break;
	}
}

uint16 QuadTree::FindObjectAddNode(uint16 startNodeId, const AABBox3& objBox)
{
	uint16 currIndex = startNodeId;					

	bool placeHere = false;

	do 
	{
		QuadTreeNode & currNode = nodes[currIndex];
		currNode.bbox.min.z = Min(currNode.bbox.min.z, objBox.min.z);
		currNode.bbox.max.z = Max(currNode.bbox.max.z, objBox.max.z);		
		placeHere = ((currNode.nodeInfo>>QuadTreeNode::NODE_DEPTH_OFFSET) >= maxTreeDepth);

		QuadTreeNode::eNodeType fitNode = QuadTreeNode::NODE_NONE;
		if (!placeHere)
		{		
			float32 midx = 0.5f*(currNode.bbox.min.x+currNode.bbox.max.x);
			float32 midy = 0.5f*(currNode.bbox.min.y+currNode.bbox.max.y);
			if ((midx>=objBox.min.x)&&(midy>=objBox.min.y))
				fitNode = QuadTreeNode::NODE_LB;			
			if ((midx<=objBox.max.x)&&(midy>=objBox.min.y))	
			{
				if (fitNode == QuadTreeNode::NODE_NONE)
					fitNode = QuadTreeNode::NODE_RB;
				else 
					placeHere = true;
			}
			if ((!placeHere)&&(midx>=objBox.min.x)&&(midy<=objBox.max.y))
			{
				if (fitNode == QuadTreeNode::NODE_NONE)
					fitNode = QuadTreeNode::NODE_LT;
				else 
					placeHere = true;
			}
			if ((!placeHere)&&(midx<=objBox.max.x)&&(midy<=objBox.max.y))
			{
				if (fitNode == QuadTreeNode::NODE_NONE)
					fitNode = QuadTreeNode::NODE_RT;
				else 
					placeHere = true;									
			}
		}
		
		if (!placeHere) //continue downwards
		{
			if (currNode.children[fitNode]==INVALID_TREE_NODE_INDEX) //set child node if not exist
			{
				DVASSERT((nodes[currIndex].nodeInfo&QuadTreeNode::NUM_CHILD_NODES_MASK)!=4)
				uint16 newNodeIndex;
				if (emptyNodes.size()) //take from empty
				{
					newNodeIndex = emptyNodes.back();
					emptyNodes.pop_back();
					nodes[newNodeIndex].Reset();
				}
				else //or create new node
				{
					newNodeIndex = nodes.size();
					nodes.resize(newNodeIndex+1);				//starting from here currNode may be invalid
				} 
				nodes[newNodeIndex].nodeInfo = (nodes[currIndex].nodeInfo&QuadTreeNode::NODE_DEPTH_MASK)+(1<<QuadTreeNode::NODE_DEPTH_OFFSET); //depth
				nodes[newNodeIndex].parent = currIndex;
				nodes[newNodeIndex].bbox = nodes[currIndex].bbox;
				UpdateChildBox(nodes[newNodeIndex].bbox, fitNode);
				nodes[newNodeIndex].bbox.min.z = AABBOX_INFINITY;
				nodes[newNodeIndex].bbox.max.z = -AABBOX_INFINITY;

				nodes[currIndex].children[fitNode] = newNodeIndex;				
				nodes[currIndex].nodeInfo++; //numChildNodes++

			}								
			currIndex = nodes[currIndex].children[fitNode];
		}		
	} while (!placeHere);

	
	return currIndex;
	
}

void QuadTree::MarkNodeDirty(uint16 nodeId)
{
	if ((nodes[nodeId].nodeInfo&QuadTreeNode::DIRTY_Z_MASK)!=QuadTreeNode::DIRTY_Z_MASK)
	{		
		nodes[nodeId].nodeInfo |= QuadTreeNode::DIRTY_Z_MASK;
		dirtyZNodes.push_back(nodeId);		
	}
}

void QuadTree::MarkObjectDirty(RenderObject *object)
{
	if (!(object->GetFlags()&RenderObject::TREE_NODE_NEED_UPDATE))
	{
		object->AddFlag(RenderObject::TREE_NODE_NEED_UPDATE);
		dirtyObjects.push_back(object);
	}
}

void QuadTree::RecalculateNodeZLimits(uint16 nodeId)
{
	QuadTreeNode& currNode = nodes[nodeId];
	currNode.bbox.min.z = AABBOX_INFINITY;
	currNode.bbox.max.z = -AABBOX_INFINITY;
	for (int32 i=0; i<QuadTreeNode::NODE_NONE; i++)
	{
		uint16 childId = currNode.children[i];
		if (childId!=INVALID_TREE_NODE_INDEX)
		{
			currNode.bbox.min.z = Min(currNode.bbox.min.z, nodes[childId].bbox.min.z);
			currNode.bbox.max.z = Max(currNode.bbox.max.z, nodes[childId].bbox.max.z);
		}
	}
	for (int32 i=0, size = currNode.objects.size(); i<size; i++)
	{
		const AABBox3 &objBox = currNode.objects[i]->GetWorldBoundingBox();
		currNode.bbox.min.z = Min(currNode.bbox.min.z, objBox.min.z);
		currNode.bbox.max.z = Max(currNode.bbox.max.z, objBox.max.z);
		
	}
	currNode.nodeInfo &= ~QuadTreeNode::DIRTY_Z_MASK;
}

void QuadTree::AddObject(RenderObject *object)
{
	DVASSERT(object->GetTreeNodeIndex()==INVALID_TREE_NODE_INDEX);		
	const AABBox3& objBox = object->GetWorldBoundingBox();
	
	//special treatment for root - as it can contain objects outside the world
	if (!worldBox.IsInside(objBox))
	{
		//object is somehow outside the world - just add to root
		nodes[0].objects.push_back(object);
		object->SetTreeNodeIndex(0);
		return;
	}
	uint16 nodeToAdd = FindObjectAddNode(0, object->GetWorldBoundingBox());			
	nodes[nodeToAdd].objects.push_back(object);
	object->SetTreeNodeIndex(nodeToAdd);
	object->RemoveFlag(RenderObject::TREE_NODE_NEED_UPDATE);
}

void QuadTree::RemoveObject(RenderObject *object)
{
	uint16 currIndex = object->GetTreeNodeIndex();
	DVASSERT(currIndex!=INVALID_TREE_NODE_INDEX);	
	object->SetTreeNodeIndex(INVALID_TREE_NODE_INDEX);
	Vector<RenderObject*>::iterator it = std::find(nodes[currIndex].objects.begin(), nodes[currIndex].objects.end(), object);
	DVASSERT(it!=nodes[currIndex].objects.end());
	nodes[currIndex].objects.erase(it);	

	if (object->GetFlags()&RenderObject::TREE_NODE_NEED_UPDATE)
	{		
		List<RenderObject *>::iterator it = std::find(dirtyObjects.begin(), dirtyObjects.end(), object);
		DVASSERT(it!=dirtyObjects.end());
		dirtyObjects.erase(it);
		object->RemoveFlag(RenderObject::TREE_NODE_NEED_UPDATE);
	}
	

	//update tree branch info	
	while(currIndex!=INVALID_TREE_NODE_INDEX) 
	{
		QuadTreeNode& currNode = nodes[currIndex];
		
		if ((currIndex!=0)&& //do not remove root node anyway
			currNode.objects.empty()&&
			(currNode.children[0]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[1]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[2]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[3]==INVALID_TREE_NODE_INDEX))
		{ //empty node - just remove it from tree
			emptyNodes.push_back(currIndex);			
			for (int32 i=0; i<4; i++)
				if (nodes[currNode.parent].children[i]==currIndex)
					nodes[currNode.parent].children[i]=INVALID_TREE_NODE_INDEX;
			DVASSERT(nodes[currNode.parent].nodeInfo&QuadTreeNode::NUM_CHILD_NODES_MASK);
			nodes[currNode.parent].nodeInfo--; //numChildNodes--
		}		
		else
		{				
			MarkNodeDirty(currIndex);
		}
		currIndex = currNode.parent;
		
	}
}


void QuadTree::ObjectUpdated(RenderObject *object)
{		
	//remove object from its current tree node
	uint16 baseIndex = object->GetTreeNodeIndex();
	DVASSERT(baseIndex!=INVALID_TREE_NODE_INDEX);		

	
	//climb up
	const AABBox3& objBox = object->GetWorldBoundingBox();	
	uint16 reverseIndex = baseIndex;
	while (reverseIndex&&(!CheckObjectFitNode(objBox, nodes[reverseIndex].bbox)))
	{		
		reverseIndex =  nodes[reverseIndex].parent;		
	}
	
	nodes[reverseIndex].bbox.min.z = Min(nodes[reverseIndex].bbox.min.z, objBox.min.z);
	nodes[reverseIndex].bbox.max.z = Max(nodes[reverseIndex].bbox.max.z, objBox.max.z);
	MarkObjectDirty(object);

	if (reverseIndex!=baseIndex)
	{
		//remove from base
		int32 objectsSize = nodes[baseIndex].objects.size();
		int32 objIndex = 0;
		for (; objIndex<objectsSize; ++objIndex)
		{
			if (nodes[baseIndex].objects[objIndex]==object) break;
		}
		DVASSERT(objIndex<objectsSize);
		if (objectsSize>1)
			nodes[baseIndex].objects[objIndex] = nodes[baseIndex].objects[objectsSize-1];
		nodes[baseIndex].objects.resize(objectsSize-1);
		//and add to target
		nodes[reverseIndex].objects.push_back(object);
		object->SetTreeNodeIndex(reverseIndex);

		/*only now we can climb back and remove/mark nodes*/
		uint16 currIndex = baseIndex;
		while (currIndex!=reverseIndex)
		{
			QuadTreeNode& currNode = nodes[currIndex];

			if ((currIndex!=0)&& //do not remove root node anyway
				currNode.objects.empty()&&
				(currNode.children[0]==INVALID_TREE_NODE_INDEX)&&
				(currNode.children[1]==INVALID_TREE_NODE_INDEX)&&
				(currNode.children[2]==INVALID_TREE_NODE_INDEX)&&
				(currNode.children[3]==INVALID_TREE_NODE_INDEX))
			{ //empty node - just remove it from tree
				emptyNodes.push_back(currIndex);			
				for (int32 i=0; i<4; i++)
					if (nodes[currNode.parent].children[i]==currIndex)
						nodes[currNode.parent].children[i]=INVALID_TREE_NODE_INDEX;
				DVASSERT(nodes[currNode.parent].nodeInfo&QuadTreeNode::NUM_CHILD_NODES_MASK);
				nodes[currNode.parent].nodeInfo--; //numChildNodes--				
			}		
			else
			{	
				MarkNodeDirty(currIndex);			
			}
			currIndex = currNode.parent;			
		}
	}
	else
	{		
		MarkNodeDirty(baseIndex);
	}

}

void QuadTree::ProcessNodeClipping(uint16 nodeId, uint8 clippingFlags)
{	
	QuadTreeNode& currNode = nodes[nodeId];	
	int32 objectsSize = currNode.objects.size();
	int32 clipBoxCount = (currNode.nodeInfo&QuadTreeNode::NUM_CHILD_NODES_MASK) + objectsSize; //still can sometime try to clip node with only invisible objects
	
	
	if (clippingFlags&&(clipBoxCount>1)&&nodeId) //root node is considered as always pass  - as objects out of worldBox are added here
	{		
		uint8 startClipPlane = (currNode.nodeInfo&QuadTreeNode::START_CLIP_PLANE_MASK)>>QuadTreeNode::START_CLIP_PLANE_OFFSET;
		if (currFrustum->Classify(currNode.bbox, clippingFlags, startClipPlane)==Frustum::EFR_OUTSIDE)
			return; //node box is outside - return
		currNode.nodeInfo&=~QuadTreeNode::START_CLIP_PLANE_MASK;
		currNode.nodeInfo|=(uint16(startClipPlane))<<QuadTreeNode::START_CLIP_PLANE_OFFSET;
	}
	//process objects in current node	
	if (!clippingFlags) //node is fully inside frustum - no need to clip anymore
	{
		for (int32 i = 0; i<objectsSize; ++i)
		{
			currNode.objects[i]->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
		}
	}
	else
	{		
		for (int32 i = 0; i<objectsSize; ++i)
		{			
			uint32 flags = currNode.objects[i]->GetFlags();
			if ((flags&RenderObject::CLIPPING_VISIBILITY_CRITERIA)==RenderObject::CLIPPING_VISIBILITY_CRITERIA)
			{				
				if ((flags&RenderObject::ALWAYS_CLIPPING_VISIBLE)||currFrustum->IsInside(currNode.objects[i]->GetWorldBoundingBox(), clippingFlags, currNode.objects[i]->startClippingPlane))
					currNode.objects[i]->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
			}				
		}
	}
	
	//process children	
	for (int32 i=0; i<QuadTreeNode::NODE_NONE; ++i)
	{
		uint16 childNodeId = currNode.children[i];
		if (childNodeId!=INVALID_TREE_NODE_INDEX)
		{
			ProcessNodeClipping(childNodeId, clippingFlags);
		}
	}		
}

void QuadTree::ProcessClipping(Frustum *frustum)
{
	currFrustum = frustum;	
	ProcessNodeClipping(0, 0x3f); 

	
}
void QuadTree::UpdateTree()
{		
	int32 count = 0;
	for (List<int32>::iterator it = dirtyZNodes.begin(), e=dirtyZNodes.end(); (it!=e)&&(count<RECALCULATE_Z_PER_FRAME); ++count)
	{
		RecalculateNodeZLimits(*it);
		it = dirtyZNodes.erase(it);
	}
	count = 0;
	for (List<RenderObject *>::iterator it = dirtyObjects.begin(), e=dirtyObjects.end(); (it!=e)&&(count<RECALCULATE_OBJECTS_PER_FRAME); ++count)
	{
		RenderObject *object = *it;		
		it = dirtyObjects.erase(it);		
		if ((object->GetFlags()&RenderObject::CLIPPING_VISIBILITY_CRITERIA)!=RenderObject::CLIPPING_VISIBILITY_CRITERIA) //object is invisible anyway - update it later
		{
			dirtyObjects.push_back(object);
		}
		else
		{
			object->RemoveFlag(RenderObject::TREE_NODE_NEED_UPDATE);
			uint16 startNode = object->GetTreeNodeIndex();
			if ((!startNode)&&(!worldBox.IsInside(object->GetWorldBoundingBox())))
				continue; //object is out of world - leave it in root node

			uint16 targetNode = FindObjectAddNode(startNode, object->GetWorldBoundingBox());
			if (startNode!=targetNode)
			{
				//remove from base
				int32 objectsSize = nodes[startNode].objects.size();
				int32 objIndex = 0;
				for (; objIndex<objectsSize; ++objIndex)
				{
					if (nodes[startNode].objects[objIndex]==object) break;
				}
				DVASSERT(objIndex<objectsSize);
				if (objectsSize>1)
					nodes[startNode].objects[objIndex] = nodes[startNode].objects[objectsSize-1];
				nodes[startNode].objects.resize(objectsSize-1);
				//and add to target
				nodes[targetNode].objects.push_back(object);
				object->SetTreeNodeIndex(targetNode);
			}
		}
	}
}

void QuadTree::DebugDraw()
{
	RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
	RenderManager::Instance()->SetState(RenderState::STATE_COLORMASK_ALL | RenderState::STATE_DEPTH_WRITE | RenderState::STATE_DEPTH_TEST);
	RenderManager::Instance()->SetColor(0.2f, 1.0f, 0.2f, 1.0f);
	DebugDrawNode(0);
	RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void QuadTree::DebugDrawNode(uint16 nodeId)
{	
	RenderManager::Instance()->SetColor(0.2f, 0.2f, 1.0f, 1.0f);	
	for (int32 i = 0, size = nodes[nodeId].objects.size(); i<size; ++i)
	{
		RenderHelper::Instance()->DrawBox(nodes[nodeId].objects[i]->GetWorldBoundingBox());
	}
	RenderManager::Instance()->SetColor(0.2f, 1.0f, 0.2f, 1.0f);
	RenderHelper::Instance()->DrawBox(nodes[nodeId].bbox);	
	for (int32 i=0; i<QuadTreeNode::NODE_NONE; ++i)
	{
		if (nodes[nodeId].children[i]!=INVALID_TREE_NODE_INDEX)
		{			
			DebugDrawNode(nodes[nodeId].children[i]);
		}
	}	
	
}

//traverse without recursive calls ... later - it works but might require some rethink
/*void QuadTree::DebugDraw()
{
	Vector<int32> dirStack;
	dirStack.resize(maxTreeDepth+1, 0);
	int currDepth=0;
	AABBox3 currBox = worldBox;
	int currNodeIndex = 0;
	do
	{

		if (dirStack[currDepth] == QuadTreeNode::NODE_NONE)
		{
			if (currDepth == 0)
			{
				break;
			}
			currDepth--;
			UpdateParentBox(currBox, QuadTreeNode::eNodeType(dirStack[currDepth]-1)); //rethink it
			currNodeIndex = nodes[currNodeIndex].parent;
		}
		else if (nodes[currNodeIndex].children[dirStack[currDepth]]!=INVALID_TREE_NODE_INDEX)
		{
			UpdateChildBox(currBox, QuadTreeNode::eNodeType(dirStack[currDepth]));
			currNodeIndex = nodes[currNodeIndex].children[dirStack[currDepth]];
			dirStack[currDepth]++;
			currDepth++;
			dirStack[currDepth] = 0;			
			//visit
			RenderManager::Instance()->SetColor(0.2f, 0.2f, 1.0f, 1.0f);	
			for (int32 i = 0, size = nodes[currNodeIndex].objects.size(); i<size; ++i)
			{
				RenderHelper::Instance()->DrawBox(nodes[currNodeIndex].objects[i]->GetWorldBoundingBox());
			}
			RenderManager::Instance()->SetColor(0.2f, 1.0f, 0.2f, 1.0f);
			RenderHelper::Instance()->DrawBox(currBox);
			//end visit
		}
		else
		{
			dirStack[currDepth]++;
			
		}

	}while(true);

}*/

} //namespace DAVA