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


#include "WaveSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Scene.h"
#include "Utils/Random.h"
#include "Math/Math2D.h"
#include "Debug/Stats.h"
#include "Render/Renderer.h"

namespace DAVA
{

WaveSystem::WaveInfo::WaveInfo(WaveComponent * _component) :
component(_component),
currentWaveRadius(0.f)
{
    if(component->GetDampingRatio() < EPSILON)
        maxRadius = component->GetInfluenceRadius();
    else
        maxRadius = Min(component->GetInfluenceRadius(), 1.f/component->GetDampingRatio());

    maxRadiusSq = maxRadius * maxRadius;

    center = GetTransformComponent(component->GetEntity())->GetWorldTransform().GetTranslationVector();
}

WaveSystem::WaveSystem(Scene * scene) : 
    SceneSystem(scene)
{
    RenderOptions* options = Renderer::GetOptions();
    options->AddObserver(this);
    HandleEvent(options);

    isVegetationAnimationEnabled = QualitySettingsSystem::Instance()->IsOptionEnabled(QualitySettingsSystem::QUALITY_OPTION_VEGETATION_ANIMATION);

    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WAVE_TRIGGERED);
}

WaveSystem::~WaveSystem()
{
    Renderer::GetOptions()->RemoveObserver(this);

    ClearWaves();
}

void WaveSystem::ImmediateEvent(Component * component, uint32 event)
{
    if(event == EventSystem::WAVE_TRIGGERED)
    {
        if(!isWavesEnabled || !isVegetationAnimationEnabled)
            return;

        WaveComponent * waveComponent = DynamicTypeCheck<WaveComponent*>(component);
        waves.push_back(new WaveInfo(waveComponent));
    }
}

void WaveSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("WaveSystem::Process");

    int32 index = 0;
    int32 size = static_cast<int32>(waves.size());
    while(index < size)
    {
        WaveInfo * info = waves[index];
        info->currentWaveRadius += info->component->GetWaveSpeed() * timeElapsed;

        if(info->currentWaveRadius >= info->maxRadius)
        {
            SafeDelete(info);
            RemoveExchangingWithLast(waves, index);
            size--;
        }
        else
        {
            index++;
        }
    }
}

Vector3 WaveSystem::GetWaveDisturbance(const Vector3 & inPosition) const
{
    Vector3 ret;
    int32 wavesCount = static_cast<int32>(waves.size());
    for(int32 i = 0; i < wavesCount; ++i)
    {
        WaveInfo * info = waves[i];
        Vector3 direction = inPosition - info->center;
        float32 distanceSq = direction.SquareLength();
        if(distanceSq > EPSILON && distanceSq < info->maxRadiusSq)
        {
            WaveComponent * component = info->component;

            float32 damping = 1 - component->GetDampingRatio() * info->currentWaveRadius; //damping function: D = 1 - k * x

            DVASSERT(damping >= 0.f);

            float32 distance = sqrtf(distanceSq);
            direction /= distance;
            float32 dt = Abs(info->currentWaveRadius - distance);
            float32 value = Max(1 - dt / component->GetWaveLenght(), 0.f) * component->GetWaveAmplitude() * component->GetWaveSpeed() * damping; // wave function: A = (1 - x/L) * A0

            DVASSERT(value >= 0.f);

            ret += direction * value;
        }
    }

    return ret;
}

void WaveSystem::ClearWaves()
{
    for(auto& wave : waves)
    {
        SafeDelete(wave);
    }

    waves.clear();
}

void WaveSystem::HandleEvent(Observable * observable)
{
    RenderOptions * options = static_cast<RenderOptions *>(observable);
    isWavesEnabled = options->IsOptionEnabled(RenderOptions::WAVE_DISTURBANCE_PROCESS);

    if(!isWavesEnabled)
        ClearWaves();
}

};