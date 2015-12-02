/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_RENDER_SPATIAL_TREE_H__
#define	__DAVAENGINE_RENDER_SPATIAL_TREE_H__

#include "Base/BaseObject.h"
#include "Math/AABBox3.h"
#include "Render/Highlevel/RenderHierarchy.h"
#include "Render/UniqueStateSet.h"

namespace DAVA
{

const static uint16 INVALID_TREE_NODE_INDEX = (uint16)(-1);
    
class RenderObject;
class Frustum;
class QuadTree : public RenderHierarchy
{	
	struct QuadTreeNode //still basic implementation - later move it to more compact
	{
		enum eNodeType {NODE_LB=0, NODE_RB=1, NODE_LT=2, NODE_RT=3, NODE_NONE = 4};
		uint16 parent;
		uint16 children[4]; //think about allocating and freeing at groups of for
		AABBox3 bbox;
		/*int32 depth;
		int32 numChildNodes; 
		uint32 startClipPlane;
		bool dirtyZ;*/		
		
		const static uint16 NUM_CHILD_NODES_MASK = 0x07;		
		const static uint16 DIRTY_Z_MASK = 0x08;
		const static uint16 NODE_DEPTH_MASK = 0xFF00;				
		const static uint16 NODE_DEPTH_OFFSET = 8;	
		const static uint16 START_CLIP_PLANE_MASK = 0xF0;				
		const static uint16 START_CLIP_PLANE_OFFSET = 4;	
		uint16 nodeInfo; // format : ddddddddddzccñ where c - numChildNodes, z - dirtyZ, d - depth
		//uint8 startClipPlane;
		Vector<RenderObject *>  objects;
		QuadTreeNode();
		void Reset();
	};	
	

	Vector<QuadTreeNode> nodes;
	Vector<uint32> emptyNodes;
	
	AABBox3 worldBox;
	int32 maxTreeDepth;
	Frustum *currFrustum;
	
	Camera *currCamera;
	uint32 currVisibilityCriteria;

	List<int32> dirtyZNodes;    
	List<RenderObject *> dirtyObjects;    	
	
	bool CheckObjectFitNode(const AABBox3& objBox, const AABBox3& nodeBox);
	bool CheckBoxIntersectBranch(const AABBox3& objBox, float32 xmin, float32 ymin, float32 xmax, float32 ymax);		
	bool CheckBoxIntersectChild(const AABBox3& objBox, const AABBox3& nodeBox, QuadTreeNode::eNodeType nodeType); //assuming it already fit parent!
	void UpdateChildBox(AABBox3 &parentBox, QuadTreeNode::eNodeType childType);
    void UpdateParentBox(AABBox3& childBox, QuadTreeNode::eNodeType childType);

    void ProcessNodeClipping(uint16 nodeId, uint8 clippingFlags, Vector<RenderObject*>& visibilityArray);
    void GetObjects(uint16 nodeId, uint8 clippingFlags, const AABBox3& bbox, Vector<RenderObject*>& visibilityArray);

    uint16 FindObjectAddNode(uint16 startNodeId, const AABBox3& objBox);

    static const int32 RECALCULATE_Z_PER_FRAME = 10;
    static const int32 RECALCULATE_OBJECTS_PER_FRAME = 10;
    void RecalculateNodeZLimits(uint16 nodeId);
    void MarkNodeDirty(uint16 nodeId);
    void MarkObjectDirty(RenderObject* object);

    bool worldInitialized;
    List<RenderObject *> worldInitObjects;	


	void DebugDrawNode(uint16 nodeId);
	UniqueHandle debugDrawStateHandle;

protected:
	~QuadTree()
	{}
	
public:
	QuadTree(int32 maxTreeDepth);
	
	virtual void AddRenderObject(RenderObject * renderObject);
	virtual void RemoveRenderObject(RenderObject * renderObject);
	virtual void ObjectUpdated(RenderObject * renderObject);
    virtual void Clip(Camera* camera, Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria);
    virtual void GetAllObjectsInBBox(const AABBox3& bbox, Vector<RenderObject*>& visibilityArray);

    virtual void Initialize();

    virtual void Update();
    virtual void DebugDraw(const Matrix4& cameraMatrix);
};

} //namespace DAVA
#endif