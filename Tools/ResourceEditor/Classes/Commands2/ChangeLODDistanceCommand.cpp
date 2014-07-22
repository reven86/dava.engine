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



#include "ChangeLODDistanceCommand.h"

using namespace DAVA;

ChangeLODDistanceCommand::ChangeLODDistanceCommand(DAVA::LodComponent *lod, DAVA::int32 lodLayer, DAVA::float32 distance, DAVA::FastName& qualityName)
	: Command2(CMDID_LOD_DISTANCE_CHANGE, "Change LOD Distance")
	, lodComponent(lod)
	, layer(lodLayer)
	, newDistance(distance)
	, oldDistance(0)
    , quality(qualityName)
    , oldLayerCount(0)
{

}

void ChangeLODDistanceCommand::Redo()
{
	if(!lodComponent) return;
    
    DVASSERT(lodComponent->qualityContainer);
    
    LodComponent::QualityContainer* qualityItem = lodComponent->FindQualityItem(quality);
    
    DVASSERT(qualityItem);
    
    oldLayerCount = qualityItem->lodLayersArray.size();
    
    if(qualityItem->lodLayersArray.size() <= layer)
    {
        qualityItem->lodLayersArray.resize(layer + 1);
    }

	oldDistance = qualityItem->lodLayersArray[layer].distance;
	lodComponent->SetLodLayerDistance(layer, newDistance, qualityItem->lodLayersArray);
    
    lodComponent->SetQuality(quality);
}

void ChangeLODDistanceCommand::Undo()
{
	if(!lodComponent) return;
    
    LodComponent::QualityContainer* qualityItem = lodComponent->FindQualityItem(quality);
    
    DVASSERT(qualityItem);
    
	lodComponent->SetLodLayerDistance(layer, oldDistance, qualityItem->lodLayersArray);
    
    if(oldLayerCount < qualityItem->lodLayersArray.size())
    {
       qualityItem->lodLayersArray.resize(oldLayerCount);
    }
    
    lodComponent->SetQuality(quality);
}

Entity * ChangeLODDistanceCommand::GetEntity() const
{
	if(lodComponent)
		return lodComponent->GetEntity();

	return NULL;
}


