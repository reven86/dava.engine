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


#include "Base/BaseMath.h"
#include "WindSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/WindComponent.h"
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

const static float32 WIND_PERIOD = 2 * PI;

WindSystem::WindInfo::WindInfo(WindComponent * c) :
component(c)
{
    timeValue = (float32)Random::Instance()->RandFloat(1000.f);
}

WindSystem::WindSystem(Scene * scene) : 
    SceneSystem(scene)
{
    RenderOptions* options = Renderer::GetOptions();
    options->AddObserver(this);
    HandleEvent(options);

    isVegetationAnimationEnabled = QualitySettingsSystem::Instance()->IsOptionEnabled(QualitySettingsSystem::QUALITY_OPTION_VEGETATION_ANIMATION);

    for(int32 i = 0; i < WIND_TABLE_SIZE; i++)
    {
        float32 t = WIND_PERIOD * i / (float32)WIND_TABLE_SIZE;
        windValuesTable[i] = (2.f + sinf(t) * 0.7f + cosf(t * 10) * 0.3f);
    }
}

WindSystem::~WindSystem()
{
    DVASSERT(winds.size() == 0);

    Renderer::GetOptions()->RemoveObserver(this);
}

void WindSystem::AddEntity(Entity * entity)
{
    WindComponent * wind = GetWindComponent(entity);
    winds.push_back(new WindInfo(wind));
}

void WindSystem::RemoveEntity(Entity * entity)
{
    int32 windsCount = static_cast<int32>(winds.size());
    for(int32 i = 0; i < windsCount; ++i)
    {
        WindInfo * info = winds[i];
        if(info->component->GetEntity() == entity)
        {
            SafeDelete(info);
            RemoveExchangingWithLast(winds, i);
            break;
        }
    }
}

void WindSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("WindSystem::Process")
    
    if(!isAnimationEnabled || !isVegetationAnimationEnabled)
        return;

    int32 windCount = static_cast<int32>(winds.size());
    for(int32 i = 0; i < windCount; ++i)
    {
        winds[i]->timeValue += timeElapsed * winds[i]->component->GetWindSpeed();
    }
}

Vector3 WindSystem::GetWind(const Vector3 & inPosition) const
{
    Vector3 ret;
    int32 windCount = static_cast<int32>(winds.size());
    for(int32 i = 0; i < windCount; ++i)
    {
        WindInfo * info = winds[i];
        if(info->component->GetInfluenceBBox().IsInside(inPosition))
        {
            ret += info->component->GetDirection() * info->component->GetWindForce() * GetWindValueFromTable(inPosition, info) * winds[i]->component->GetWindSpeed();
        }
    }

    return ret;
}

float32 WindSystem::GetWindValueFromTable(const Vector3 & inPosition, const WindInfo * info) const
{
    Vector3 dir = info->component->GetDirection();
    Vector3 projPt = dir * (inPosition.DotProduct(dir));
    float32 t = projPt.Length() + info->timeValue;

    float32 tMod = fmodf(t, WIND_PERIOD);
    int32 i = (int32)floorf(tMod / WIND_PERIOD * WIND_TABLE_SIZE);

    DVASSERT(i >= 0 && i < WIND_TABLE_SIZE);
    return windValuesTable[i];
}

void WindSystem::HandleEvent(Observable * observable)
{
    RenderOptions * options = static_cast<RenderOptions *>(observable);
    isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);
}

};