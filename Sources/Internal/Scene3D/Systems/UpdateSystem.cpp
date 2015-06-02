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


#include "Scene3D/Systems/UpdateSystem.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Entity.h"
#include "Platform/SystemTimer.h"
#include "Debug/Stats.h"

namespace DAVA
{

UpdateSystem::UpdateSystem(Scene * scene)
:	SceneSystem(scene)
{

}

void UpdateSystem::AddEntity(Entity * entity)
{
	UpdatableComponent * component = (UpdatableComponent*)entity->GetComponent(Component::UPDATABLE_COMPONENT);
	IUpdatable * object = component->GetUpdatableObject();

	if(object)
	{
		IUpdatableBeforeTransform * updateBeforeTransform = dynamic_cast<IUpdatableBeforeTransform*>(object);
		if(updateBeforeTransform)
		{
			updatesBeforeTransform.push_back(updateBeforeTransform);
		}

		IUpdatableAfterTransform * updateAfterTransform = dynamic_cast<IUpdatableAfterTransform*>(object);
		if(updateAfterTransform)
		{
			updatesAfterTransform.push_back(updateAfterTransform);
		}
	}
}

void UpdateSystem::RemoveEntity(Entity * entity)
{
	UpdatableComponent * component = (UpdatableComponent*)entity->GetComponent(Component::UPDATABLE_COMPONENT);
	IUpdatable * object = component->GetUpdatableObject();

	if(object)
	{
		IUpdatableBeforeTransform * updateBeforeTransform = dynamic_cast<IUpdatableBeforeTransform*>(object);
		if(updateBeforeTransform)
		{
			uint32 size = static_cast<uint32>(updatesBeforeTransform.size());
			for(uint32 i = 0; i < size; ++i)
			{
				if(updatesBeforeTransform[i] == updateBeforeTransform)
				{
					updatesBeforeTransform[i] = updatesBeforeTransform[size-1];
					updatesBeforeTransform.pop_back();
					break;
				}
			}
		}

		IUpdatableAfterTransform * updateAfterTransform = dynamic_cast<IUpdatableAfterTransform*>(object);
		if(updateAfterTransform)
		{
			uint32 size = static_cast<uint32>(updatesAfterTransform.size());
			for(uint32 i = 0; i < size; ++i)
			{
				if(updatesAfterTransform[i] == updateAfterTransform)
				{
					updatesAfterTransform[i] = updatesAfterTransform[size-1];
					updatesAfterTransform.pop_back();
					break;
				}
			}
		}
	}
}

void UpdateSystem::Process(float32 timeElapsed)
{

}

void UpdateSystem::UpdatePreTransform(float32 timeElapsed)
{
    TIME_PROFILE("UpdateSystem::UpdatePreTransform");

	uint32 size = static_cast<uint32>(updatesBeforeTransform.size());
	for(uint32 i = 0; i < size; ++i)
	{
		updatesBeforeTransform[i]->UpdateBeforeTransform(timeElapsed);
	}
}

void UpdateSystem::UpdatePostTransform(float32 timeElapsed)
{
    TIME_PROFILE("UpdateSystem::UpdatePostTransform");

	uint32 size = static_cast<uint32>(updatesAfterTransform.size());
	for(uint32 i = 0; i < size; ++i)
	{
		updatesAfterTransform[i]->UpdateAfterTransform(timeElapsed);
	}
}

}