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
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{

REGISTER_CLASS(LodComponent)
	
const float32 LodComponent::INVALID_DISTANCE = -1.f;
const float32 LodComponent::MIN_LOD_DISTANCE = 0.f;
const float32 LodComponent::MAX_LOD_DISTANCE = 1000.f;
const float32 LodComponent::INFINITY_LOD_DISTANCE_SQ = 10000.0f * 10000.0f;

LodComponent::LodDistance::LodDistance()
{
	distance = nearDistanceSq = farDistanceSq = (float32) INVALID_DISTANCE;
    lodIndex = -1;
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
		archive->SetUInt32("lc.flags", flags);

        DVASSERT(qualityContainer);
        
        if(NULL == qualityContainer)
        {
            PopulateQualityContainer();
        }
        
        KeyedArchive* qualityContainerArchive = new KeyedArchive();
        
        size_t qualityCount = qualityContainer->size();
        for(size_t qualityIndex = 0; qualityIndex < qualityCount; ++qualityIndex)
        {
            QualityContainer& containerItem = (*qualityContainer)[qualityIndex];
            KeyedArchive* containerItemArchive = new KeyedArchive();
            
            size_t lodLayerCount = containerItem.lodLayersArray.size();
            for (size_t i = 0; i < lodLayerCount; ++i)
            {
                KeyedArchive* lodDistValuesArch = new KeyedArchive();
                lodDistValuesArch->SetFloat("ld.distance", containerItem.lodLayersArray[i].distance);
                lodDistValuesArch->SetFloat("ld.neardistsq", containerItem.lodLayersArray[i].nearDistanceSq);
                lodDistValuesArch->SetFloat("ld.fardistsq", containerItem.lodLayersArray[i].farDistanceSq);
                lodDistValuesArch->SetInt32("ld.lodIndex", (int32)containerItem.lodLayersArray[i].lodIndex);
                
                containerItemArchive->SetArchive(KeyedArchive::GenKeyFromIndex(i), lodDistValuesArch);
                SafeRelease(lodDistValuesArch);
            }
            
            qualityContainerArchive->SetArchive(containerItem.qualityName.c_str(),
                                                containerItemArchive);
            SafeRelease(containerItemArchive);
        }
        
        archive->SetArchive("ld.qualityContainer", qualityContainerArchive);
        SafeRelease(qualityContainerArchive);
	}
}

void LodComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    if(serializationContext->GetVersion() < LODS_WITH_QUALITY_VERSION)
    {
        DeserializeWithoutQuality(archive, serializationContext);
    }
    else
    {
        DeserializeWithQuality(archive, serializationContext);
    }
}

LodComponent::LodComponent()
:	forceLodLayer(INVALID_LOD_LAYER),
	forceDistance(INVALID_DISTANCE),
	forceDistanceSq(INVALID_DISTANCE),
    currentLod(INVALID_LOD_LAYER),
    qualityContainer(NULL)
{
	lodLayersArray.resize(MAX_LOD_LAYERS);

	flags = NEED_UPDATE_AFTER_LOAD;

    uint32 layerCount = lodLayersArray.size();
	for(int32 iLayer = 0; iLayer < layerCount; ++iLayer)
	{
		lodLayersArray[iLayer].SetDistance(GetDefaultDistance(iLayer));
		lodLayersArray[iLayer].SetFarDistance(MAX_LOD_DISTANCE * 2);
	}

	lodLayersArray[0].SetNearDistance(0.0f);
}

LodComponent::~LodComponent()
{
    SafeDelete(qualityContainer);
}

float32 LodComponent::GetDefaultDistance(int32 layer)
{
	float32 distance = MIN_LOD_DISTANCE + ((float32)(MAX_LOD_DISTANCE - MIN_LOD_DISTANCE) / GetDefaultLod()) * layer;
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
    SetLodLayerDistance(layerNum, distance, lodLayersArray);
}

void LodComponent::SetLodLayerDistance(int32 layerNum, float32 distance, Vector<LodDistance>& layers)
{
    DVASSERT(0 <= layerNum && layerNum < layers.size());
    
    if(INVALID_DISTANCE != distance)
    {
        float32 nearDistance = distance * 0.95f;
        float32 farDistance = distance * 1.05f;
        
        if(layers.size() - 1 == layerNum)
        {
            layers[layerNum].SetFarDistance(MAX_LOD_DISTANCE * 1.05f);
        }
        if(layerNum)
        {
            layers[layerNum-1].SetFarDistance(farDistance);
        }
        
        layers[layerNum].SetDistance(distance);
        layers[layerNum].SetNearDistance(nearDistance);
    }
    else
    {
        layers[layerNum].SetDistance(distance);
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
    
    if(fromLOD->qualityContainer != NULL)
    {
        InitQualityContainer();
        
        (*qualityContainer) = *(fromLOD->qualityContainer);
    }
    else
    {
        SafeDelete(qualityContainer);
    }
}

void LodComponent::DeserializeWithQuality(KeyedArchive *archive,
                                          SerializationContext *serializationContext)
{
    if(NULL != archive)
	{
		if(archive->IsKeyExists("lc.flags"))
        {
            flags = archive->GetUInt32("lc.flags");
        }
        
        forceDistance = INVALID_DISTANCE;
        forceDistanceSq = INVALID_DISTANCE;
        forceLodLayer = INVALID_LOD_LAYER;
        
        const FastName& currentLodQuality = QualitySettingsSystem::Instance()->GetCurrentLODQuality();
        DVASSERT(currentLodQuality.IsValid());
        
        KeyedArchive* qualityContainerArchive = archive->GetArchive("ld.qualityContainer");
        DVASSERT(qualityContainerArchive);
        DVASSERT(qualityContainerArchive->Count() > 0);

        KeyedArchive* currentQualityLODArchive = qualityContainerArchive->GetArchive(currentLodQuality.c_str());
        if(NULL == currentQualityLODArchive)
        {
           const Map<String, VariantType*>& allData = qualityContainerArchive->GetArchieveData();
           Map<String, VariantType*>::const_iterator it = allData.begin();
           
           currentQualityLODArchive = qualityContainerArchive->GetArchive(it->first);
        }
        
        LoadDistancesFromArchive(currentQualityLODArchive, lodLayersArray, currentQualityLODArchive->Count());
        
        if(serializationContext->TestSerializationFlags(SerializationContext::EDITOR_MODE))
        {
            const Map<String, VariantType*>& allData = qualityContainerArchive->GetArchieveData();
            Map<String, VariantType*>::const_iterator it = allData.begin();
            Map<String, VariantType*>::const_iterator itEnd = allData.end();
            
            InitQualityContainer();
            
            qualityContainer->resize(allData.size());
            
            size_t curIndex = 0;
            while(it != itEnd)
            {
                KeyedArchive* lodArchive = it->second->AsKeyedArchive();
                QualityContainer& containerItem = (*qualityContainer)[curIndex];
                
                containerItem.qualityName = FastName(it->first);
                LoadDistancesFromArchive(lodArchive, containerItem.lodLayersArray, lodArchive->Count());
                
                ++curIndex;
                ++it;
            }
        }
	}
    
	flags |= NEED_UPDATE_AFTER_LOAD;
	Component::Deserialize(archive, serializationContext);

}
    
void LodComponent::DeserializeWithoutQuality(KeyedArchive *archive,
                                             SerializationContext *serializationContext)
{
    if(NULL != archive)
	{
		if(archive->IsKeyExists("lc.flags"))
        {
            flags = archive->GetUInt32("lc.flags");
        }
        
        forceDistance = INVALID_DISTANCE;
        forceDistanceSq = INVALID_DISTANCE;
        forceLodLayer = INVALID_LOD_LAYER;
        
		KeyedArchive *lodDistArch = archive->GetArchive("lc.loddist");
		if(NULL != lodDistArch)
		{
            LoadDistancesFromArchive(lodDistArch, lodLayersArray, MAX_LOD_LAYERS);
        
            //if(serializationContext->TestSerializationFlags(SerializationContext::EDITOR_MODE))
            //{
            //    PopulateQualityContainer();
            //}
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

bool LodComponent::ApplyQuality(const FastName& qualityName,
                                Vector<QualityContainer>& src,
                                Vector<LodDistance>& dst)
{
    bool qualityApplied = false;
    
    size_t qualityCount = src.size();
    for(size_t i = 0; i < qualityCount; ++i)
    {
        QualityContainer& containerItem = src[i];
        
        if(qualityName == containerItem.qualityName)
        {
            qualityApplied = true;
            
            dst.clear();
            dst = containerItem.lodLayersArray;
        }
    }
    
    return qualityApplied;
}

void LodComponent::PopulateQualityContainer()
{
    InitQualityContainer();
    
    QualitySettingsSystem* qualitySystem = QualitySettingsSystem::Instance();
    int32 qualityCount = qualitySystem->GetLODQualityCount();
    
    qualityContainer->resize(qualityCount);
    for(int32 i = 0; i < qualityCount; ++i)
    {
        QualityContainer& containerItem = (*qualityContainer)[i];
        
        containerItem.qualityName = qualitySystem->GetLODQualityName(i);
        containerItem.lodLayersArray = lodLayersArray;
    }
}

void LodComponent::LoadDistancesFromArchive(KeyedArchive* lodDistArch,
                                            Vector<LodDistance>& lodLayers,
                                            uint32 maxDistanceCount)
{
    uint32 validDistanceCount = 0;
    
    LodDistance prevTestDistance;
    prevTestDistance.distance = -1.0f;
    prevTestDistance.farDistanceSq = -1.0f;
    prevTestDistance.nearDistanceSq = -1.0f;
    
    for(int32 i = 0; i < maxDistanceCount; ++i)
    {
        KeyedArchive *lodDistValuesArch = lodDistArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
        if(NULL != lodDistValuesArch)
        {
            LodDistance testDistance;
            
            testDistance.distance = lodDistValuesArch->GetFloat("ld.distance");
            testDistance.nearDistanceSq = lodDistValuesArch->GetFloat("ld.neardistsq");
            testDistance.farDistanceSq = lodDistValuesArch->GetFloat("ld.fardistsq");
            testDistance.lodIndex = (lodDistValuesArch->IsKeyExists("ld.lodIndex")) ? (int8)lodDistValuesArch->GetInt32("ld.lodIndex") : (int8)i;
            
            if(testDistance.IsValid() &&
               testDistance.IsValidInSequence(prevTestDistance))
            {
                validDistanceCount++;
            }
            
            prevTestDistance = testDistance;
        }
    }
    
    prevTestDistance.distance = -1.0f;
    prevTestDistance.farDistanceSq = -1.0f;
    prevTestDistance.nearDistanceSq = -1.0f;
    
    lodLayers.resize(validDistanceCount);
    uint32 lodLayerIndex = 0;
    for(int32 i = 0; i < maxDistanceCount; ++i)
    {
        KeyedArchive *lodDistValuesArch = lodDistArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
        if(NULL != lodDistValuesArch)
        {
            LodDistance testDistance;
            
            testDistance.distance = lodDistValuesArch->GetFloat("ld.distance");
            testDistance.nearDistanceSq = lodDistValuesArch->GetFloat("ld.neardistsq");
            testDistance.farDistanceSq = lodDistValuesArch->GetFloat("ld.fardistsq");
            testDistance.lodIndex = (lodDistValuesArch->IsKeyExists("ld.lodIndex")) ? (int8)lodDistValuesArch->GetInt32("ld.lodIndex") : (int8)i;
            
            if(testDistance.IsValid() &&
               testDistance.IsValidInSequence(prevTestDistance))
            {
                lodLayers[lodLayerIndex] = testDistance;
                lodLayerIndex++;
            }
            
            prevTestDistance = testDistance;
        }
    }
    
    if(validDistanceCount > 0)
    {
        lodLayers[lodLayers.size() - 1].farDistanceSq = LodComponent::INFINITY_LOD_DISTANCE_SQ;
    }
}

LodComponent::QualityContainer* LodComponent::FindQualityItem(const FastName& qualityName)
{
    LodComponent::QualityContainer* item = NULL;
    
    if(qualityContainer)
    {
        size_t qualityCount = qualityContainer->size();
        for(size_t i = 0; i < qualityCount; ++i)
        {
            QualityContainer& containerItem = (*qualityContainer)[i];
            
            if(qualityName == containerItem.qualityName)
            {
                item = &containerItem;
                break;
            }
        }
    }
    
    return item;
}


};
