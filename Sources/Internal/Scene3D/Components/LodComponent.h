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



#ifndef __DAVAENGINE_LOD_COMPONENT_H__
#define __DAVAENGINE_LOD_COMPONENT_H__

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
	static const float32 MIN_LOD_DISTANCE;
	static const float32 MAX_LOD_DISTANCE;
	static const float32 INVALID_DISTANCE;
    
	enum eFlags
	{
		NEED_UPDATE_AFTER_LOAD = 1 << 0,
        RECURSIVE_UPDATE = 1 << 1
	};

	struct LodDistance
	{
		float32 distance;
		float32 nearDistanceSq;
		float32 farDistanceSq;

		LodDistance();
		void SetDistance(const float32 &newDistance);
        float32 GetDistance() const { return distance; };
        
		void SetNearDistance(const float32 &newDistance);
		void SetFarDistance(const float32 &newDistance);
        
		float32 GetNearDistance() const;
		float32 GetFarDistance() const;


        INTROSPECTION(LodDistance,
            PROPERTY("distance", "Distance", GetDistance, SetDistance, I_SAVE | I_VIEW)
            MEMBER(nearDistanceSq, "Near Distance", I_SAVE | I_VIEW)
            MEMBER(farDistanceSq, "Far Distance", I_SAVE | I_VIEW)
        );
	};

	struct LodData
	{
		LodData()
		:	layer(INVALID_LOD_LAYER),
			isDummy(false)
		{ }

		Vector<Entity*> nodes;
		Vector<int32> indexes;
		int32 layer;
		bool isDummy;
	};
protected:
    ~LodComponent(){};
public:
	LodComponent();
	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

	static float32 GetDefaultDistance(int32 layer);

	CC_DEPRECATED(inline int32 GetLodLayersCount() const);
	inline float32 GetLodLayerDistance(int32 layerNum) const;
	inline float32 GetLodLayerNear(int32 layerNum) const;
	inline float32 GetLodLayerFar(int32 layerNum) const;
	inline float32 GetLodLayerNearSquare(int32 layerNum) const;
	inline float32 GetLodLayerFarSquare(int32 layerNum) const;

	CC_DEPRECATED(void GetLodData(Vector<LodData*> &retLodLayers));

	int32 currentLod;
	Vector<LodData> lodLayers;
	Vector<LodDistance> lodLayersArray;
	int32 forceLodLayer;

    void SetForceDistance(const float32 &newDistance);
    float32 GetForceDistance() const;
	float32 forceDistance;
	float32 forceDistanceSq;

	int32 flags;
    
    /**
         \brief Registers LOD layer into the LodComponent.
         \param[in] layerNum is the layer index
         \param[in] distance near view distance for the layer
	 */
    void SetLodLayerDistance(int32 layerNum, float32 distance);

    
    /**
         \brief Sets lod layer thet would be forcely used in the whole scene.
         \param[in] layer layer to set on the for the scene. Use -1 to disable forced lod layer.
	 */
    void SetForceLodLayer(int32 layer);
    int32 GetForceLodLayer() const;

	int32 GetMaxLodLayer() const;

	

    void CopyLODSettings(const LodComponent * fromLOD);	

    inline void EnableRecursiveUpdate();
    inline bool IsRecursiveUpdate();
    
public:
    
    INTROSPECTION_EXTEND(LodComponent, Component,
        COLLECTION(lodLayersArray, "Lod Layers Array", I_SAVE | I_VIEW)
        MEMBER(forceLodLayer, "Force Lod Layer", I_SAVE | I_VIEW)
        PROPERTY("forceDistance", "Force Distance", GetForceDistance, SetForceDistance, I_SAVE | I_VIEW)
        MEMBER(flags, "Flags", I_SAVE | I_VIEW | I_EDIT)
    );
};

int32 LodComponent::GetLodLayersCount() const
{
	return (int32)lodLayers.size();
}

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
    
void LodComponent::EnableRecursiveUpdate()
{
    flags |= RECURSIVE_UPDATE;
}

bool LodComponent::IsRecursiveUpdate()
{
    return (flags & RECURSIVE_UPDATE) != 0;
}
    
};

#endif //__DAVAENGINE_LOD_COMPONENT_H__
