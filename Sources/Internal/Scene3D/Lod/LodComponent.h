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
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class Entity;
class LodComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(LOD_COMPONENT);

    static const int32 MAX_LOD_LAYERS = 4;
    static const int32 INVALID_LOD_LAYER = -1;
    static const int32 LAST_LOD_LAYER = 0x7fffffff;
    static const float32 MIN_LOD_DISTANCE;
    static const float32 MAX_LOD_DISTANCE;
    static const float32 INVALID_DISTANCE;

    LodComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetLodLayerDistance(int32 layerNum, float32 distance);
    float32 GetLodLayerDistance(int32 layerNum) const;

    int32 GetCurrentLod() const;

private:
    static float32 GetDefaultDistance(int32 layer);
    int32 currentLod;
    Vector<float32> distances;
    void SetCurrentLod(int32 lod);

public:
    INTROSPECTION_EXTEND(LodComponent, Component,
                         COLLECTION(distances, "Lod Distances", I_SAVE | I_VIEW)
                         );

    friend class LodSystem;
};

REGISTER_CLASS(LodComponent);

inline void LodComponent::SetCurrentLod(int32 lod)
{
    currentLod = lod;
}

inline int32 LodComponent::GetCurrentLod() const
{
    return currentLod;
}

inline float32 LodComponent::GetLodLayerDistance(int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return distances[layerNum];
}

};
