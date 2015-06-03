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


#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{

	
const float32 LodComponent::INVALID_DISTANCE = -1.f;
const float32 LodComponent::MIN_LOD_DISTANCE = 0.f;
const float32 LodComponent::MAX_LOD_DISTANCE = 1000.f;
    
const float32 NEAR_DISTANCE_COEFF = 1.05f;
const float32 FAR_DISTANCE_COEFF = 0.95f;

LodComponent::LodDistance::LodDistance()
{
	distance = nearDistanceSq = farDistanceSq = (float32) INVALID_DISTANCE;
}

void LodComponent::LodDistance::SetDistance(const float32 &newDistance)
{
	distance = newDistance;
}

void LodComponent::LodDistance::SetNearDistance(const float32 &newDistance)
{
	nearDistanceSq = newDistance * newDistance;
}

float32 LodComponent::LodDistance::GetNearDistance() const
{
	return sqrtf(nearDistanceSq);
}


void LodComponent::LodDistance::SetFarDistance(const float32 &newDistance)
{
	farDistanceSq = newDistance * newDistance;
}

float32 LodComponent::LodDistance::GetFarDistance() const
{
	return sqrtf(farDistanceSq);
}


Component * LodComponent::Clone(Entity * toEntity)
{
	LodComponent * newLod = new LodComponent();
	newLod->SetEntity(toEntity);

	newLod->lodLayers = lodLayers;
	const Vector<LodData>::const_iterator endLod = newLod->lodLayers.end();
	for (Vector<LodData>::iterator it = newLod->lodLayers.begin(); it != endLod; ++it)
	{
		LodData & ld = *it;
		ld.nodes.clear();
	}

	//Lod values
    newLod->CopyLODSettings(this);

	return newLod;
}

void LodComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(NULL != archive)
	{
		uint32 i;

		archive->SetUInt32("lc.flags", flags);

		KeyedArchive *lodDistArch = new KeyedArchive();
		for (i = 0; i < MAX_LOD_LAYERS; ++i)
		{
			KeyedArchive *lodDistValuesArch = new KeyedArchive();
			lodDistValuesArch->SetFloat("ld.distance", lodLayersArray[i].distance);
			lodDistValuesArch->SetFloat("ld.neardistsq", lodLayersArray[i].nearDistanceSq);
			lodDistValuesArch->SetFloat("ld.fardistsq", lodLayersArray[i].farDistanceSq);

			lodDistArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), lodDistValuesArch);
			lodDistValuesArch->Release();
		}
		archive->SetArchive("lc.loddist", lodDistArch);
		lodDistArch->Release();
	}
}

void LodComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		if(archive->IsKeyExists("lc.flags")) flags = archive->GetUInt32("lc.flags");

        forceDistance = INVALID_DISTANCE;
        forceDistanceSq = INVALID_DISTANCE;
        forceLodLayer = INVALID_LOD_LAYER;
        
		KeyedArchive *lodDistArch = archive->GetArchive("lc.loddist");
		if(NULL != lodDistArch)
		{
			for(uint32 i = 0; i < MAX_LOD_LAYERS; ++i)
			{
				KeyedArchive *lodDistValuesArch = lodDistArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
				if(NULL != lodDistValuesArch)
				{
					lodLayersArray[i].distance = lodDistValuesArch->GetFloat("ld.distance");
					lodLayersArray[i].nearDistanceSq = lodDistValuesArch->GetFloat("ld.neardistsq");
					lodLayersArray[i].farDistanceSq = lodDistValuesArch->GetFloat("ld.fardistsq");
				}
			}
		}

        if(serializationContext->GetVersion() < OLD_LODS_SCENE_VERSION)
        {
            KeyedArchive *lodDataArch = archive->GetArchive("lc.loddata");
            if(NULL != lodDataArch)
            {
                uint32 lodDataCount = archive->GetUInt32("lc.loddatacount");
                lodLayers.reserve(lodDataCount);
                for(uint32 i = 0; i < lodDataCount; ++i)
                {
                    KeyedArchive *lodDataValuesArch = lodDataArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
                    if(NULL != lodDataValuesArch)
                    {
                        LodData data;

                        if(lodDataValuesArch->IsKeyExists("layer")) data.layer = lodDataValuesArch->GetInt32("layer");
                        if(lodDataValuesArch->IsKeyExists("isdummy")) data.isDummy = lodDataValuesArch->GetBool("isdummy");

                        KeyedArchive *lodDataIndexesArch = lodDataValuesArch->GetArchive("indexes");
                        if(NULL != lodDataIndexesArch)
                        {
                            uint32 indexesCount = lodDataValuesArch->GetUInt32("indexescount");
                            data.indexes.reserve(indexesCount);
                            for(uint32 j = 0; j < indexesCount; ++j)
                            {
                                data.indexes.push_back(lodDataIndexesArch->GetInt32(KeyedArchive::GenKeyFromIndex(j)));
                            }
                        }

                        lodLayers.push_back(data);
                    }
                }
            }
        }
	}

	flags |= NEED_UPDATE_AFTER_LOAD;
	Component::Deserialize(archive, serializationContext);
}

LodComponent::LodComponent()
: currentLod(INVALID_LOD_LAYER)
, forceLodLayer(INVALID_LOD_LAYER)
, forceDistance(INVALID_DISTANCE)
, forceDistanceSq(INVALID_DISTANCE)
{
	lodLayersArray.resize(MAX_LOD_LAYERS);

	flags = NEED_UPDATE_AFTER_LOAD;

	for(int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
	{
        SetLodLayerDistance(iLayer, GetDefaultDistance(iLayer));
	}

	lodLayersArray[0].SetNearDistance(0.0f);
}

float32 LodComponent::GetDefaultDistance(int32 layer)
{
	float32 distance = MIN_LOD_DISTANCE + ((float32)(MAX_LOD_DISTANCE - MIN_LOD_DISTANCE) / (MAX_LOD_LAYERS-1)) * layer;
	return distance;
}

void LodComponent::SetForceDistance(const float32 &newDistance)
{
    forceDistance = newDistance;
    forceDistanceSq = forceDistance * forceDistance;
}
    
float32 LodComponent::GetForceDistance() const
{
    return forceDistance;
}

void LodComponent::GetLodData(Vector<LodData*> &retLodLayers)
{
	retLodLayers.clear();
    retLodLayers.reserve(lodLayers.size());

	Vector<LodData>::const_iterator endIt = lodLayers.end();
	for(Vector<LodData>::iterator it = lodLayers.begin(); it != endIt; ++it)
	{
		LodData *ld = &(*it);
		retLodLayers.push_back(ld);
	}
}
    
void LodComponent::SetLodLayerDistance(int32 layerNum, float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    
    if(INVALID_DISTANCE != distance)
    {
        float32 nearDistance = distance * NEAR_DISTANCE_COEFF;
        float32 farDistance = distance * FAR_DISTANCE_COEFF;
        
        if(DAVA::GetLodLayersCount(this) - 1 == static_cast<uint32>(layerNum))
        {
            lodLayersArray[layerNum].SetFarDistance(MAX_LOD_DISTANCE * FAR_DISTANCE_COEFF);
        }
        if(layerNum)
        {
            lodLayersArray[layerNum-1].SetFarDistance(farDistance);
        }
        
        lodLayersArray[layerNum].SetDistance(distance);
        lodLayersArray[layerNum].SetNearDistance(nearDistance);
    }
    else 
    {
        lodLayersArray[layerNum].SetDistance(distance);
    }
}

void LodComponent::SetForceLodLayer(int32 layer)
{
    forceLodLayer = layer;
}
    
int32 LodComponent::GetForceLodLayer() const
{
    return forceLodLayer;
}

int32 LodComponent::GetMaxLodLayer() const
{
	int32 ret = -1;
	const Vector<LodData>::const_iterator &end = lodLayers.end();
	for (Vector<LodData>::const_iterator it = lodLayers.begin(); it != end; ++it)
	{
		const LodData & ld = *it;
		if(ld.layer > ret)
		{
			ret = ld.layer;
		}
	}

	return ret;
}

void LodComponent::CopyLODSettings(const LodComponent * fromLOD)
{
    lodLayersArray = fromLOD->lodLayersArray;

    forceDistance = fromLOD->forceDistance;
    forceDistanceSq = fromLOD->forceDistanceSq;
    forceLodLayer = fromLOD->forceLodLayer;
}


};
