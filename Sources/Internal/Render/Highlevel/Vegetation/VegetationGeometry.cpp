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


#include "Render/Highlevel/Vegetation/VegetationGeometry.h"

namespace DAVA
{

VegetationGeometry::VegetationGeometry() : materialTransform(NULL)
{
}

VegetationGeometry::~VegetationGeometry()
{
    SafeDelete(materialTransform);
}


void VegetationGeometry::SetupCameraPositions(const AABBox3& bbox, Vector<Vector3>& positions)
{
    float32 z = bbox.min.z + (bbox.max.z - bbox.min.z) * 0.5f;
    
    positions.push_back(Vector3(bbox.min.x, bbox.min.y, z));
    positions.push_back(Vector3(bbox.min.x + (bbox.max.x - bbox.min.x) * 0.5f, bbox.min.y, z));
    positions.push_back(Vector3(bbox.max.x, bbox.min.y, z));
    positions.push_back(Vector3(bbox.max.x, bbox.min.y + (bbox.max.y - bbox.min.y) * 0.5f, z));
    positions.push_back(Vector3(bbox.max.x, bbox.max.y, z));
    positions.push_back(Vector3(bbox.min.x + (bbox.max.x - bbox.min.x) * 0.5f, bbox.max.y, z));
    positions.push_back(Vector3(bbox.min.x, bbox.max.y, z));
    positions.push_back(Vector3(bbox.min.x, bbox.min.y + (bbox.max.y - bbox.min.y) * 0.5f, z));
}

uint32 VegetationGeometry::GetSortDirectionCount()
{
    return 8;
}

void VegetationGeometry::ReleaseRenderData(Vector<VegetationRenderData*>& renderDataArray)
{
    size_t renderDataCount = renderDataArray.size();
    for(size_t i = 0; i < renderDataCount; ++i)
    {
        renderDataArray[i]->ReleaseRenderData();
        SafeDelete(renderDataArray[i]);
    }
    
    renderDataArray.clear();
}

}