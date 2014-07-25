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


#include "Commands2/LodIndexChangeCommand.h"

LodIndexChangeCommand::LodIndexChangeCommand(DAVA::int32 layerNum,
                                             DAVA::int32 lodIndex,
                                             DAVA::LodComponent * component,
                                             const DAVA::FastName& currentQuality) :
                                             Command2(CMDID_SET_LOD_INDEX, "Set LOD index"),
                                             lodComponent(component),
                                             qualityName(currentQuality),
                                             targetLayerNumber(layerNum),
                                             targetLodIndex(lodIndex),
                                             storedLodIndex(DAVA::LodComponent::INVALID_LOD_LAYER),
                                             oldLayerCount(0)
{
}

LodIndexChangeCommand::~LodIndexChangeCommand()
{
}

void LodIndexChangeCommand::Undo()
{
    if(NULL == lodComponent)
    {
        return;
    }
    
    DAVA::LodComponent::QualityContainer* qualityItem = lodComponent->FindQualityItem(qualityName);
    DAVA::Vector<DAVA::LodComponent::LodDistance>& lodLayersArray = (NULL == qualityItem) ? lodComponent->lodLayersArray : qualityItem->lodLayersArray;
    
    Undo(lodLayersArray);
    
    if(qualityItem)
    {
        lodComponent->SetQuality(qualityName);
    }
}

void LodIndexChangeCommand::Redo()
{
    if(NULL == lodComponent)
    {
        return;
    }
    
    DAVA::LodComponent::QualityContainer* qualityItem = lodComponent->FindQualityItem(qualityName);
    DAVA::Vector<DAVA::LodComponent::LodDistance>& lodLayersArray = (NULL == qualityItem) ? lodComponent->lodLayersArray : qualityItem->lodLayersArray;
    
    Redo(lodLayersArray);
    
    if(qualityItem)
    {
        lodComponent->SetQuality(qualityName);
    }
}

DAVA::Entity* LodIndexChangeCommand::GetEntity() const
{
    if(lodComponent)
		return lodComponent->GetEntity();
    
	return NULL;
}

void LodIndexChangeCommand::Redo(DAVA::Vector<DAVA::LodComponent::LodDistance>& lodLayersArray)
{
    oldLayerCount = lodLayersArray.size();
    
    if(lodLayersArray.size() <= targetLayerNumber)
    {
        lodLayersArray.resize(targetLayerNumber + 1);
    }
    
    DVASSERT(targetLayerNumber >= 0 &&
             targetLayerNumber < lodLayersArray.size());
    
    storedLodIndex = lodLayersArray[targetLayerNumber].lodIndex;
    lodLayersArray[targetLayerNumber].lodIndex = targetLodIndex;
}

void LodIndexChangeCommand::Undo(DAVA::Vector<DAVA::LodComponent::LodDistance>& lodLayersArray)
{
    DVASSERT(targetLayerNumber >= 0 &&
             targetLayerNumber < lodLayersArray.size());
    
    lodLayersArray[targetLayerNumber].lodIndex = storedLodIndex;
    
    if(oldLayerCount < lodLayersArray.size())
    {
        lodLayersArray.resize(oldLayerCount);
    }
}
