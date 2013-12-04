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

#include "ConvertToShadowCommand.h"

using namespace DAVA;


ConvertToShadowCommand::ConvertToShadowCommand(DAVA::Entity * entity)
    : Command2(CMDID_CONVERT_TO_SHADOW, "Convert To Shadow")
{
	affectedEntity = entity;
    changedRenderBatch = 0;
}

ConvertToShadowCommand::~ConvertToShadowCommand()
{
    SafeRelease(changedRenderBatch);
    affectedEntity = NULL;
}

void ConvertToShadowCommand::Redo()
{
    DVASSERT(affectedEntity);

	changedRenderBatch = ConvertToShadowVolume(affectedEntity);
}

void ConvertToShadowCommand::Undo()
{
    if(changedRenderBatch && affectedEntity)
    {
		if(IsEntityWithShadowVolume(affectedEntity))
		{
			RenderObject * ro = GetRenderObject(affectedEntity);

			RenderBatch * shadowVolume = ro->GetRenderBatch(0);
			ro->RemoveRenderBatch(shadowVolume);

			ro->AddRenderBatch(changedRenderBatch);
			SafeRelease(changedRenderBatch);

			affectedEntity->SetLocalTransform(affectedEntity->GetLocalTransform());
		}
    }
}

DAVA::Entity* ConvertToShadowCommand::GetEntity() const
{
    return affectedEntity;
}

bool ConvertToShadowCommand::IsEntityWithShadowVolume(const DAVA::Entity *entity)
{
	RenderObject * ro = GetRenderObject(entity);
	return (ro && ro->GetRenderBatchCount() == 1 && typeid(*(ro->GetRenderBatch(0))) == typeid(DAVA::ShadowVolume));
}

bool ConvertToShadowCommand::IsAvailableForConvertionToShadowVolume(const DAVA::Entity *entity)
{
    RenderObject * ro = GetRenderObject(entity);
    return (    (!ro || (ro->GetRenderBatchCount() != 1) ||
                 (typeid(*(ro->GetRenderBatch(0))) != typeid(DAVA::RenderBatch)) ||
                 (ro->GetType() == RenderObject::TYPE_SKYBOX)) == false);
}


DAVA::RenderBatch * ConvertToShadowCommand::ConvertToShadowVolume( DAVA::Entity *entity )
{
	RenderObject * ro = GetRenderObject(entity);
	if(ro && (ro->GetRenderBatchCount() == 1) && (typeid(*(ro->GetRenderBatch(0))) == typeid(DAVA::RenderBatch)))
	{
		ShadowVolume * shadowVolume = ro->CreateShadow();

		RenderBatch * oldBatch = ro->GetRenderBatch(0);
		oldBatch->Retain();

		ro->RemoveRenderBatch(oldBatch);
		ro->AddRenderBatch(shadowVolume);
		shadowVolume->Release();

		entity->SetLocalTransform(entity->GetLocalTransform());//just forced update of worldTransform

		return oldBatch;
	}

	return NULL;
}
