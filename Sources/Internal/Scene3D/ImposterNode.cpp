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



#include "ImposterNode.h"
#include "ImposterManager.h"
#include "Utils/Utils.h"
#include "Render/Image/Image.h"
#include "Platform/SystemTimer.h"
#include "Scene3D/MeshInstanceNode.h"

namespace DAVA
{

const float32 ImposterNode::TOGGLE_SQUARE_DISTANCE = 2500.f;


ImposterNode::ImposterNode()
:	Entity()
{
	state = STATE_3D;
	renderData = 0;
	manager = 0;
	distanceSquaredToCamera = 0;
	isReady = false;
	block = 0;
	priority = 0;
}

ImposterNode::~ImposterNode()
{
	if(manager)
	{
		SharedFBO * fbo = manager->GetFBO();
		if(block && fbo)
		{
			fbo->ReleaseBlock(block);
		}
	}
	block = 0;
	SafeRelease(renderData);
}

void ImposterNode::UpdateState()
{
	if(GetChildrenCount() > 0)
	{
		DVASSERT(GetChildrenCount() == 1);
		AABBox3 bbox = GetChild(0)->GetWTMaximumBoundingBoxSlow();
		Vector3 bboxCenter = bbox.GetCenter();
		float32 distanceSquare = (scene->GetCurrentCamera()->GetPosition() - bboxCenter).SquareLength();
		distanceSquare *= scene->GetCurrentCamera()->GetZoomFactor() * scene->GetCurrentCamera()->GetZoomFactor();
		
		Vector3 newDirection = scene->GetCurrentCamera()->GetPosition()-center;
		newDirection.Normalize();
		float32 dotProduct = newDirection.DotProduct(direction);

		switch(state)
		{
		case STATE_3D:
		{
			if(distanceSquare > TOGGLE_SQUARE_DISTANCE)
			{
				UpdatePriority(distanceSquare, 0);
				AskForRedraw();
			}
			else
			{
				isReady = false;
			}
		}
		break;

		case STATE_QUEUED:
		{
			if(IsAngleOrRangeChangedEnough(distanceSquare, dotProduct))
			{
				UpdatePriority(distanceSquare, dotProduct);
				manager->UpdateQueue(this);
			}
		}
		break;

		case STATE_IMPOSTER:
		{
			if(distanceSquare < TOGGLE_SQUARE_DISTANCE)
			{
				isReady = false;
				state = STATE_3D;
				manager->RemoveFromQueue(this);
				break;
			}

			if(IsAngleOrRangeChangedEnough(distanceSquare, dotProduct))
			{
				UpdatePriority(distanceSquare, dotProduct);
				AskForRedraw();
			}
		}
		break;

		case STATE_REDRAW_APPROVED:
		{
		}
		break;
		}
	}
}


bool ImposterNode::IsAngleOrRangeChangedEnough(float32 squareDistance, float32 dotProduct)
{
	bool result = false;

	float32 distanceDelta = squareDistance/distanceSquaredToCamera;
	if((dotProduct < 0.9999f) 
		|| (distanceDelta < 0.25f/*0.5^2*/) //if distance is doubled or halved
		|| (distanceDelta > 4.f/*2^2*/))
	{
		result = true;
	}

	return result;
}

void ImposterNode::Draw()
{
	DVASSERT(GetChildrenCount() == 1);
	GetChild(0)->Draw();
}

void ImposterNode::GeneralDraw()
{
	if(IsImposterReady())
	{
		DrawImposter();
	}
	else
	{
		Entity::Draw();
	}
}

void ImposterNode::GetOOBBoxScreenCoords(Entity * node, const Matrix4 & mvp, AABBox3 & screenBounds)
{
	const Rect & viewport = RenderManager::Instance()->GetViewport();
	MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode*>(node);
	if (mesh)
	{
		Vector3 corners[8];
		Vector3 screenVertices[8];

		mesh->GetBoundingBox().GetCorners(corners);
		const Matrix4 & worldTransform = mesh->GetWorldTransform();

		for (int32 k = 0; k < 8; ++k)
		{
			Vector4 pv(corners[k]);
			pv = pv * worldTransform;
			pv = pv * mvp;
			pv.x = (viewport.dx * 0.5f) * (1.f + pv.x/pv.w) + viewport.x;
			pv.y = (viewport.dy * 0.5f) * (1.f + pv.y/pv.w) + viewport.y;
			pv.z = (pv.z/pv.w + 1.f) * 0.5f;

			screenVertices[k] = Vector3(pv.x, pv.y, pv.z);
			screenBounds.AddPoint(screenVertices[k]);

		}
	}

	int32 count = node->GetChildrenCount();
	for (int32 i = 0; i < count; ++i)
	{
		GetOOBBoxScreenCoords(node->GetChild(i), mvp, screenBounds);
	}
}


void ImposterNode::UpdateImposter()
{
	Camera * camera = scene->GetCurrentCamera();

	Camera * imposterCamera = new Camera();
	Vector3 cameraPos = camera->GetPosition();

	Entity * child = GetChild(0);
	AABBox3 bbox = child->GetWTMaximumBoundingBoxSlow();
	Vector3 bboxCenter = bbox.GetCenter();

	imposterCamera->SetupPerspective(camera->GetFOV(), camera->GetAspect(), camera->GetZNear(), camera->GetZFar());
	imposterCamera->SetTarget(bbox.GetCenter());
	imposterCamera->SetPosition(cameraPos);
	imposterCamera->SetUp(camera->GetUp());
	imposterCamera->SetLeft(camera->GetLeft());

	Rect viewport = RenderManager::Instance()->GetViewport();
	
	const Matrix4 & mvp = imposterCamera->GetViewProjMatrix();

	AABBox3 screenBounds;
	GetOOBBoxScreenCoords(child, mvp, screenBounds);

	Vector4 pv(bboxCenter);
	pv = pv*mvp;
	pv.z = (pv.z/pv.w + 1.f) * 0.5f;
	float32 bboxCenterZ = pv.z;

	Vector2 screenSize = Vector2(screenBounds.max.x-screenBounds.min.x, screenBounds.max.y-screenBounds.min.y);

	Vector3 screenBillboardVertices[4];
	screenBillboardVertices[0] = Vector3(screenBounds.min.x, screenBounds.min.y, screenBounds.min.z);
	screenBillboardVertices[1] = Vector3(screenBounds.max.x, screenBounds.min.y, screenBounds.min.z);
	screenBillboardVertices[2] = Vector3(screenBounds.min.x, screenBounds.max.y, screenBounds.min.z);
	screenBillboardVertices[3] = Vector3(screenBounds.max.x, screenBounds.max.y, screenBounds.min.z);

	center = Vector3();
	Matrix4 invMvp = mvp;
	invMvp.Inverse();
	for(int32 i = 0; i < 4; ++i)
	{
		//unproject
		Vector4 out;
		out.x = 2.f*(screenBillboardVertices[i].x-viewport.x)/viewport.dx-1.f;
		out.y = 2.f*(screenBillboardVertices[i].y-viewport.y)/viewport.dy-1.f;
		out.z = 2.f*screenBillboardVertices[i].z-1.f;
		out.w = 1.f;

		out = out*invMvp;
		DVASSERT(out.w != 0.f);

		out.x /= out.w;
		out.y /= out.w;
		out.z /= out.w;

		imposterVertices[i] = Vector3(out.x, out.y, out.z);
		center += imposterVertices[i];
	}
	center /= 4.f;


	//draw
	RecreateFbo(screenSize);
	//Logger::Info("%f, %f", screenSize.x, screenSize.y);
	if(!block)
	{
		return;
	}

	direction = camera->GetPosition()-center;
	direction.Normalize();

	distanceSquaredToCamera = (center-cameraPos).SquareLength();

	float32 nearPlane = sqrtf(distanceSquaredToCamera);
	//float32 farPlane = nearPlane + (bbox.max.z-bbox.min.z);
	float32 w = (imposterVertices[1]-imposterVertices[0]).Length();
	float32 h = (imposterVertices[2]-imposterVertices[0]).Length();
	
	//TODO: calculate instead of +50
	imposterCamera->Setup(-w/2.f, w/2.f, -h/2.f, h/2.f, nearPlane, nearPlane+50.f);

	Rect oldViewport = RenderManager::Instance()->GetViewport();
	
	//Texture * target = fbo->GetTexture();

	//RenderManager::Instance()->AppendState(RenderState::STATE_SCISSOR_TEST);
	//RenderManager::Instance()->State()->SetScissorRect(Rect(block->offset.x, block->offset.y, block->size.dx, block->size.dy));
	RenderManager::Instance()->FlushState();
	//TODO: use one "clear" function instead of two
	//if(block->size.x == 512.f)
	//{
	//	RenderManager::Instance()->ClearWithColor(0.f, .8f, 0.f, 1.f);
	//}
	//else if(block->size.x == 256.f)
	//{
	//	RenderManager::Instance()->ClearWithColor(0.f, .3f, 0.f, 1.f);
	//}
	//else if(block->size.x == 128.f)
	//{
	//	RenderManager::Instance()->ClearWithColor(.3f, .3f, 0.f, 1.f);
	//}
	//else
	//{
	//	RenderManager::Instance()->ClearWithColor(.3f, 0.f, 0.f, 1.f);
	//}
    
	RenderManager::Instance()->ClearWithColor(.0f, .0f, 0.f, .0f);
	RenderManager::Instance()->ClearDepthBuffer();
	//RenderManager::Instance()->RemoveState(RenderState::STATE_SCISSOR_TEST);

	RenderManager::Instance()->SetViewport(Rect(block->offset.x, block->offset.y, block->size.dx, block->size.dy));


	imposterCamera->SetTarget(center);
	imposterCamera->SetupDynamicParameters();

	//TODO: remove this call
	HierarchicalRemoveCull(child);
	RenderManager::Instance()->FlushState();
	child->Draw();

	RenderManager::Instance()->SetViewport(oldViewport);

	isReady = true;
	state = STATE_IMPOSTER;

	//unproject
	screenBillboardVertices[0] = Vector3(screenBounds.min.x, screenBounds.min.y, bboxCenterZ);
	screenBillboardVertices[1] = Vector3(screenBounds.max.x, screenBounds.min.y, bboxCenterZ);
	screenBillboardVertices[2] = Vector3(screenBounds.min.x, screenBounds.max.y, bboxCenterZ);
	screenBillboardVertices[3] = Vector3(screenBounds.max.x, screenBounds.max.y, bboxCenterZ);
	for(int32 i = 0; i < 4; ++i)
	{
		//unproject
		Vector4 out;
		out.x = 2.f*(screenBillboardVertices[i].x-viewport.x)/viewport.dx-1.f;
		out.y = 2.f*(screenBillboardVertices[i].y-viewport.y)/viewport.dy-1.f;
		out.z = 2.f*screenBillboardVertices[i].z-1.f;
		out.w = 1.f;

		out = out*invMvp;
		DVASSERT(out.w != 0.f);

		out.x /= out.w;
		out.y /= out.w;
		out.z /= out.w;

		imposterVertices[i] = Vector3(out.x, out.y, out.z);
	}

	SafeRelease(imposterCamera);

	ClearGeometry();
	CreateGeometry();
}

void ImposterNode::HierarchicalRemoveCull(Entity * node)
{
	//TODO: remove this function
	node->RemoveFlag(NODE_CLIPPED_THIS_FRAME);
	uint32 size = (uint32)(node->GetChildrenCount());
	for(uint32 c = 0; c < size; ++c)
	{
		HierarchicalRemoveCull(node->GetChild(c));
	}
}

void ImposterNode::DrawImposter()
{
#if 0
	if(!block)
	{
		return;
	}

	Matrix4 modelViewMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
    uint32 matrixCache = RenderManager::Instance()->GetModelViewMatrixCache();
	const Matrix4 & cameraMatrix = scene->GetCurrentCamera()->GetMatrix();
	Matrix4 meshFinalMatrix;

	meshFinalMatrix = /*worldTransform **/ cameraMatrix;

	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST);

	RenderManager::Instance()->SetColor(1.f, 1.f, 1.f, 1.f);

	//RenderManager::Instance()->RemoveState(RenderState::STATE_CULL);
	//eBlendMode src = RenderManager::Instance()->GetSrcBlend();
	//eBlendMode dst = RenderManager::Instance()->GetDestBlend();
	//RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

	SharedFBO * fbo = manager->GetFBO();
	RenderManager::Instance()->SetTextureState(fbo->GetTextureHandle());

	RenderManager::Instance()->SetRenderData(renderData);

	//RenderManager::Instance()->FlushState();
	//RenderManager::Instance()->AttachRenderData();
	RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);

	//RenderManager::Instance()->AppendState(RenderStateBlock::STATE_DEPTH_WRITE);
	//RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);
	//RenderManager::Instance()->SetBlendMode(src, dst);

	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, modelViewMatrix, matrixCache);
#endif 
}

Entity* ImposterNode::Clone(Entity *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		DVASSERT_MSG(IsPointerToExactClass<ImposterNode>(this), "Can clone only ImposterNode");
		dstNode = new ImposterNode();
	}

	Entity::Clone(dstNode);

	return dstNode;
}

void ImposterNode::ClearGeometry()
{
	SafeRelease(renderData);
	verts.clear();
	texCoords.clear();
}

void ImposterNode::CreateGeometry()
{
	DVASSERT(verts.empty() && texCoords.empty() && !renderData);

	verts.push_back(imposterVertices[0].x);
	verts.push_back(imposterVertices[0].y);
	verts.push_back(imposterVertices[0].z);

	verts.push_back(imposterVertices[1].x);
	verts.push_back(imposterVertices[1].y);
	verts.push_back(imposterVertices[1].z);

	verts.push_back(imposterVertices[2].x);
	verts.push_back(imposterVertices[2].y);
	verts.push_back(imposterVertices[2].z);

	verts.push_back(imposterVertices[3].x);
	verts.push_back(imposterVertices[3].y);
	verts.push_back(imposterVertices[3].z);

	SharedFBO * fbo = manager->GetFBO();
	float32 fboWidth = (float32)fbo->GetTexture()->GetWidth();
	float32 fboHeight = (float32)fbo->GetTexture()->GetHeight();
	float32 uMin, uMax, vMin, vMax;
	uMin = block->offset.x/fboWidth;
	vMin = block->offset.y/fboHeight;
	uMax = (block->offset.x+block->size.dx)/fboWidth;
	vMax = (block->offset.y+block->size.dy)/fboHeight;

	texCoords.push_back(uMin);
	texCoords.push_back(vMin);

	texCoords.push_back(uMax);
	texCoords.push_back(vMin);

	texCoords.push_back(uMin);
	texCoords.push_back(vMax);

	texCoords.push_back(uMax);
	texCoords.push_back(vMax);

	renderData = new RenderDataObject();
	renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &(verts[0]));
	renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &(texCoords[0]));
}


void ImposterNode::AskForRedraw()
{
	manager->AddToQueue(this);
	state = STATE_QUEUED;
}

bool ImposterNode::IsRedrawApproved()
{
	return (STATE_REDRAW_APPROVED == state);
}

bool ImposterNode::IsImposterReady()
{
	return isReady;
}

void ImposterNode::RecreateFbo(const Vector2 & size)
{
	SharedFBO * fbo = manager->GetFBO();
	if(block)
	{
		fbo->ReleaseBlock(block);
	}
	block = fbo->AcquireBlock(size);
}

bool ImposterNode::IsQueued()
{
	return (STATE_QUEUED == state);
}

void ImposterNode::SetManager(ImposterManager * _manager)
{
	manager = _manager;
}

void ImposterNode::UpdatePriority(float32 squaredDistance, float32 dotProduct)
{
	if(dotProduct)
	{
		priority = squaredDistance*dotProduct;
	}
	else
	{
		priority = squaredDistance;
	}
}

float32 ImposterNode::GetPriority()
{
	return priority;
}

void ImposterNode::ZeroOutBlock()
{
	block = 0;
}


}
