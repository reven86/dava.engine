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



#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Particles/ParticleEmitter.h"
#include "Platform/SystemTimer.h"
#include "Base/TemplateHelpers.h"
#include "Render/Highlevel/Camera.h"
#include "Core/PerformanceSettings.h"

namespace DAVA
{


void ParticleEmitterSystem::AddIfEmitter(RenderObject * maybeEmitter)
{
	if(maybeEmitter->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
	{
		ParticleEmitter * emitter = static_cast<ParticleEmitter*>(maybeEmitter);
		emitters.push_back(emitter);
	}
}

void ParticleEmitterSystem::RemoveIfEmitter(RenderObject * maybeEmitter)
{
	if(maybeEmitter->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
	{
		ParticleEmitter * emitter = static_cast<ParticleEmitter*>(maybeEmitter);
		uint32 size = emitters.size();
		for(uint32 i = 0; i < size; ++i)
		{
			if(emitters[i] == emitter)
			{
				emitter->HandleRemoveFromSystem();
				emitters[i] = emitters[size-1];
				emitters.pop_back();
				return;
			}
		}
		DVASSERT(0);
	}
}

void ParticleEmitterSystem::Update(float32 timeElapsed, Camera * camera)
{
	uint32 size = emitters.size();
	Vector<ParticleEmitter*> emittersToBeDeleted;

	float32 currFps = 1.0f/timeElapsed;
	
	float32 currPSValue = (currFps - PerformanceSettings::Instance()->GetPsPerformanceMinFPS())/(PerformanceSettings::Instance()->GetPsPerformanceMaxFPS()-PerformanceSettings::Instance()->GetPsPerformanceMinFPS());
	currPSValue = Clamp(currPSValue, 0.0f, 1.0f);
	float32 speedMult = 1.0f+(PerformanceSettings::Instance()->GetPsPerformanceSpeedMult()-1.0f)*(1-currPSValue);

	for(uint32 i = 0; i < size; ++i)
	{
		// Yuri Coder, 2013/05/15. Visible emitters are always updated, "deferred" update
		// is called for invisible ones. See pls issue #DF-1140.
		uint32 flags = emitters[i]->GetFlags();
		bool requireUpdate = false;

		float32 effectTime = emitters[i]->IsShortEffect()?timeElapsed*speedMult:timeElapsed;

        if ((flags & RenderObject::VISIBILITY_CRITERIA) == RenderObject::VISIBILITY_CRITERIA)
        {
            emitters[i]->Update(effectTime);
			requireUpdate = true;
        }
		else
		{
			requireUpdate = emitters[i]->DeferredUpdate(effectTime);
		}		

		if (emitters[i]->IsToBeDeleted())
		{
			emittersToBeDeleted.push_back(emitters[i]);
		}
		else if (requireUpdate)
		{
			emitters[i]->PrepareRenderData(camera);			
			emitters[i]->GetRenderSystem()->MarkForUpdate(emitters[i]);
		}				
		emitters[i]->RecalcBoundingBox(); //recalculate it anyway just in case
	}

	for(Vector<ParticleEmitter*>::iterator it = emittersToBeDeleted.begin(); it != emittersToBeDeleted.end(); ++it)
	{
		ParticleEmitter* partEmitter = (*it);
		RenderSystem* renderSystem = partEmitter->GetRenderSystem();
	
		renderSystem->RemoveFromRender(partEmitter);
		SafeRelease(partEmitter);
	}
}

}
