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

#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
class Camera;
class LodComponent;
class ParticleEffectComponent;

class LodSystem : public SceneSystem
{
public:
    LodSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

private:
    struct LodComponentInternal
    {
        LodComponentInternal(Entity* entity_,
                             TransformComponent* transform_,
                             LodComponent* lod_,
                             ParticleEffectComponent* effect_,
                             int32 index_)
        {
            entity = entity_;
            transform = transform_;
            lod = lod_;
            effect = effect_;
            index = index_;
        }
        Entity* entity;
        TransformComponent* transform;
        LodComponent* lod;
        ParticleEffectComponent* effect;
        int32 index;
    };
    UnorderedMap<Entity*, LodComponentInternal*> fastMap;
    Vector<LodComponentInternal> fastVector;
    Vector<LodComponent*> lodComponents;

    static void SetEntityLod(Entity* entity, int32 currentLod);

    Camera* camera = nullptr;
};

}
