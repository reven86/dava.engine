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


#ifndef __DAVAENGINE_IMPOSTER_NODE_H__
#define __DAVAENGINE_IMPOSTER_NODE_H__

#include "Scene3D/Scene.h"
#include "Render/RenderDataObject.h"
#include "Render/SharedFBO.h"

namespace DAVA
{

class ImposterManager;

class ImposterNode : public Entity
{
public:

	static const float32 TOGGLE_SQUARE_DISTANCE;

	enum eState
	{
		STATE_3D = 0,
		STATE_IMPOSTER,
		STATE_QUEUED,
		STATE_REDRAW_APPROVED
	};
protected:
	virtual ~ImposterNode();
public:
	ImposterNode();

	void UpdateState();
	virtual void Draw();
	virtual Entity* Clone(Entity *dstNode = NULL);

	void UpdateImposter();
	void GeneralDraw();
	void DrawImposter();

	bool IsQueued();

	void SetManager(ImposterManager * manager);

	float32 GetPriority();
	
	void ZeroOutBlock();

private:
	void AskForRedraw();
	void ClearGeometry();
	void CreateGeometry();
	
	bool IsRedrawApproved();
	bool IsImposterReady();

	bool isReady;

	Vector3 imposterVertices[4];
	RenderDataObject * renderData;

	Vector<float32> verts;
	Vector<float32> texCoords;
	Vector3 center;
	Vector3 direction;
	float32 distanceSquaredToCamera;

	eState state;

	ImposterManager * manager;

	SharedFBO::Block * block;

	void RecreateFbo(const Vector2 & size);
	void HierarchicalRemoveCull(Entity * node);

	void UpdatePriority(float32 squaredDistance, float32 dotProduct);
	float32 priority;

	bool IsAngleOrRangeChangedEnough(float32 squareDistance, float32 dotProduct);
	void GetOOBBoxScreenCoords(Entity * node, const Matrix4 & mvp, AABBox3 & screenBounds);

	friend class ImposterNodeComparer;
};

};

#endif //__DAVAENGINE_IMPOSTER_NODE_H__
