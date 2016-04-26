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

    struct LodDistance : public InspBase
    {
        float32 distance = INVALID_DISTANCE;
        float32 nearDistanceSq = INVALID_DISTANCE;
        float32 farDistanceSq = INVALID_DISTANCE;

        void SetDistance(const float32& newDistance);
        float32 GetDistance() const
        {
            return distance;
        };

        void SetNearDistance(const float32& newDistance);
        void SetFarDistance(const float32& newDistance);

        float32 GetNearDistance() const;
        float32 GetFarDistance() const;

        INTROSPECTION(LodDistance,
                      PROPERTY("distance", "Distance", GetDistance, SetDistance, I_SAVE | I_VIEW)
                      MEMBER(nearDistanceSq, "Near Distance", I_SAVE | I_VIEW)
                      MEMBER(farDistanceSq, "Far Distance", I_SAVE | I_VIEW)
                      );
    };

public:
    LodComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    inline float32 GetLodLayerDistance(int32 layerNum) const;
    inline float32 GetLodLayerNearSquare(int32 layerNum) const;
    inline float32 GetLodLayerFarSquare(int32 layerNum) const;

    int32 currentLod;
    Vector<LodDistance> lodLayersArray;

    //force lod - for editors only
    int32 forceLodLayer;
    void SetForceDistance(const float32& newDistance);
    float32 GetForceDistance() const;

    void SetLodLayerDistance(int32 layerNum, float32 distance);

    void SetForceLodLayer(int32 layer);
    int32 GetForceLodLayer() const;

    void CopyLODSettings(const LodComponent* fromLOD);

private:
    static float32 GetDefaultDistance(int32 layer);
    float32 forceDistance;
    float32 forceDistanceSq;

public:
    INTROSPECTION_EXTEND(LodComponent, Component,
                         COLLECTION(lodLayersArray, "Lod Layers Array", I_SAVE | I_VIEW)
                         MEMBER(forceLodLayer, "Force Lod Layer", I_SAVE | I_VIEW)
                         PROPERTY("forceDistance", "Force Distance", GetForceDistance, SetForceDistance, I_SAVE | I_VIEW)
                         );

    friend class LodSystem;
};

REGISTER_CLASS(LodComponent);

float32 LodComponent::GetLodLayerDistance(int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].distance;
}

float32 LodComponent::GetLodLayerNearSquare(int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].nearDistanceSq;
}

float32 LodComponent::GetLodLayerFarSquare(int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].farDistanceSq;
}

};
