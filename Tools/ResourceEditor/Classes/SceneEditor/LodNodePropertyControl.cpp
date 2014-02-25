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



#include "LodNodePropertyControl.h"


LodNodePropertyControl::LodNodePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

LodNodePropertyControl::~LodNodePropertyControl()
{

}


void LodNodePropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    LodComponent *lodComponent = GetLodComponent(sceneNode);
	DVASSERT(lodComponent);

    propertyList->AddSection("property.lodnode", GetHeaderState("property.lodnode", true));
    
    propertyList->AddBoolProperty("property.lodnode.forcedistance");
    propertyList->SetBoolPropertyValue("property.lodnode.forcedistance", false);
    propertyList->AddSliderProperty("property.lodnode.distanceslider", false);
    propertyList->SetSliderPropertyValue("property.lodnode.distanceslider", 0, LodComponent::MAX_LOD_DISTANCE, LodComponent::MIN_LOD_DISTANCE);

    int32 lodCount = lodComponent->GetLodLayersCount();
    if(1 < lodCount)
    {
        propertyList->AddDistanceProperty("property.lodnode.distances");
        float32 *distances = new float32[lodComponent->GetLodLayersCount()];
        int32 *triangles = new int32[lodComponent->GetLodLayersCount()];
        
        
        Vector<LodComponent::LodData*> lodLayers;
        lodComponent->GetLodData(lodLayers);
        
        Vector<LodComponent::LodData*>::const_iterator lodLayerIt = lodLayers.begin();
        for(int32 i = 0; i < lodComponent->GetLodLayersCount(); ++i)
        {
            distances[i] = lodComponent->GetLodLayerDistance(i);
            
            LodComponent::LodData *layer = *lodLayerIt;
            triangles[i] = GetTrianglesForLodLayer(layer);

            ++lodLayerIt;
        }
        
        propertyList->SetDistancePropertyValue("property.lodnode.distances", distances, triangles, lodComponent->GetLodLayersCount());
        
        
        SafeDeleteArray(distances);
        SafeDeleteArray(triangles);
    }
}



