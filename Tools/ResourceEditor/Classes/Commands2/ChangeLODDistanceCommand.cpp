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

ChangeLODDistanceCommand::ChangeLODDistanceCommand(DAVA::LodComponent *lod, DAVA::int32 lodLayer, DAVA::float32 distance)
	: Command2(CMDID_LOD_DISTANCE_CHANGE, "Change LOD Distance")
	, lodComponent(lod)
	, layer(lodLayer)
	, newDistance(distance)
	, oldDistance(0)
{

}

void ChangeLODDistanceCommand::Redo()
{
	if(!lodComponent) return;

	oldDistance = lodComponent->GetLodLayerDistance(layer);
	lodComponent->SetLodLayerDistance(layer, newDistance);
}

void ChangeLODDistanceCommand::Undo()
{
	if(!lodComponent) return;

	lodComponent->SetLodLayerDistance(layer, oldDistance);
}

Entity * ChangeLODDistanceCommand::GetEntity() const
{
	if(lodComponent)
		return lodComponent->GetEntity();

	return NULL;
}

CopyLastLODToLod0Command::CopyLastLODToLod0Command(DAVA::LodComponent * lod) 
    : Command2(CMDID_LOD_COPY_LAST_LOD, "Make last LOD to lod0")
    , lodComponent(lod)
{
}

void CopyLastLODToLod0Command::Redo()
{
    if(!lodComponent) return;

    LodComponent::LodData newLayerData = lodComponent->lodLayers.back();
    newLayerData.layer = 0;

    lodComponent->lodLayers.insert(lodComponent->lodLayers.begin(), newLayerData);

    for(int32 i = 0; i < lodComponent->lodLayers.size(); i++)
        lodComponent->lodLayers[i].layer = i;

    for(int32 i = LodComponent::MAX_LOD_LAYERS - 1; i > 0; i--)
    {
        float32 distance = lodComponent->GetLodLayerDistance(i - 1);
        lodComponent->SetLodLayerDistance(i, distance);
    }
    
    lodComponent->SetLodLayerDistance(0, 0.f);
    lodComponent->SetLodLayerDistance(1, 2.f);
}

void CopyLastLODToLod0Command::Undo()
{
    if(!lodComponent) return;

    lodComponent->lodLayers.erase(lodComponent->lodLayers.begin());
    
    for(int32 i = 0; i < lodComponent->lodLayers.size(); i++)
        lodComponent->lodLayers[i].layer = i;

    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS - 1; i++)
        lodComponent->SetLodLayerDistance(i, lodComponent->GetLodLayerDistance(i + 1));
    
    lodComponent->SetLodLayerDistance(0, 0.f);
}

Entity * CopyLastLODToLod0Command::GetEntity() const
{
    if(lodComponent)
        return lodComponent->GetEntity();

    return NULL;
}