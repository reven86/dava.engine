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


#include "Render/Highlevel/SpriteRenderBatch.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/SpriteObject.h"
#include "Render/Highlevel/RenderFastNames.h"

namespace DAVA
{

SpriteRenderBatch::SpriteRenderBatch()
	: RenderBatch()
{
    //SetOwnerLayerName(LAYER_TRANSLUCENT);
}

SpriteRenderBatch::~SpriteRenderBatch()
{
}

void SpriteRenderBatch::Draw(const FastName & ownerRenderPass, Camera * camera)
{
	if(!renderObject)return;
	Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
	if (!worldTransformPtr)
	{
		return;
	}

	uint32 flags = renderObject->GetFlags();
	if ((flags & RenderObject::VISIBILITY_CRITERIA) != RenderObject::VISIBILITY_CRITERIA)
		return;



	SpriteObject *spriteObject = static_cast<SpriteObject *>(renderObject);
	if(!spriteObject || !spriteObject->GetSprite())
	{
		return;
	}


	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

	const Matrix4 & cameraMatrix = camera->GetMatrix();
	Matrix4 finalMatrix;

	switch(spriteObject->GetSpriteType())
	{
	case SpriteObject::SPRITE_OBJECT:
		{
			finalMatrix = (*worldTransformPtr);
			break;
		};
	case SpriteObject::SPRITE_BILLBOARD:
		{
			Matrix4 inverse(Matrix4::IDENTITY);

			inverse._00 = cameraMatrix._00;
			inverse._01 = cameraMatrix._10;
			inverse._02 = cameraMatrix._20;

			inverse._10 = cameraMatrix._01;
			inverse._11 = cameraMatrix._11;
			inverse._12 = cameraMatrix._21;

			inverse._20 = cameraMatrix._02;
			inverse._21 = cameraMatrix._12;
			inverse._22 = cameraMatrix._22;

			finalMatrix = inverse * (*worldTransformPtr);
			break;
		};
	case SpriteObject::SPRITE_BILLBOARD_TO_CAMERA:
		{
 			Vector3 look = camera->GetPosition() - Vector3(0.0f, 0.0f, 0.0f) * (*worldTransformPtr); 
			look.Normalize();
			Vector3 right = CrossProduct(camera->GetUp(), look);
 			Vector3 up = CrossProduct(look, right);

			Matrix4 matrix = Matrix4::IDENTITY;
			matrix._00 = right.x;
			matrix._01 = right.y;
			matrix._02 = right.z;

			matrix._10 = up.x;
			matrix._11 = up.y;
			matrix._12 = up.z;

			matrix._20 = look.x;
			matrix._21 = look.y;
			matrix._22 = look.z;

			finalMatrix = matrix * (*worldTransformPtr);
			break;
		};   
	}

	RenderManager::SetDynamicParam(PARAM_WORLD, &finalMatrix, (pointer_size)worldTransformPtr);

	
	material->BindMaterialTechnique(ownerRenderPass, camera);
	RenderManager::Instance()->SetRenderData(renderDataObject);
	RenderManager::Instance()->AttachRenderData();

	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, spriteObject->GetFrame() * 4, 4);
}

void SpriteRenderBatch::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
    // Don't need to save this batch
}

void SpriteRenderBatch::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    // Don't need to load this batch
}

};

