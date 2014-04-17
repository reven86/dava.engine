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


#include "Render/Highlevel/SpeedTreeObject.h"
#include "Utils/Utils.h"

namespace DAVA 
{

void SpeedTreeObject::RecalcBoundingBox()
{
    bbox = AABBox3();

    uint32 size = (uint32)renderBatchArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * rb = renderBatchArray[k].renderBatch;
        PolygonGroup * pg = rb->GetPolygonGroup();
        if(pg)
        {
            if((pg->GetFormat() & EVF_TANGENT) > 0) //speedtree leaf batch
                bbox.AddAABBox(CalcBBoxForSpeedTreeLeafGeometry(pg));
            else
                bbox.AddAABBox(rb->GetBoundingBox());
        }   
    }
}

RenderObject * SpeedTreeObject::Clone(RenderObject *newObject)
{
    if(!newObject)
    {
        DVASSERT_MSG(IsPointerToExactClass<SpeedTreeObject>(this), "Can clone only SpeedTreeObject");
        newObject = new SpeedTreeObject();
    }

    return Mesh::Clone(newObject);
}

AABBox3 SpeedTreeObject::CalcBBoxForSpeedTreeLeafGeometry(PolygonGroup * pg)
{
    AABBox3 pgBbox;
    if(pg)
    {
        DVASSERT((pg->GetFormat() & EVF_TANGENT) > 0); //non speedtree leaf batch

        int32 vertexCount = pg->GetVertexCount();
        for(int32 vi = 0; vi < vertexCount; vi++)
        {
            Vector3 pivot;
            pg->GetTangent(vi, pivot);

            Vector3 pointX, pointY, pointZ;
            Vector3 offsetX, offsetY, offsetZ;

            pg->GetCoord(vi, pointZ);
            offsetX = offsetY = offsetZ = pointZ - pivot;

            Swap(offsetX.x, offsetX.z);
            Swap(offsetX.y, offsetX.z);
            
            pointX = pivot + offsetX;
            pointY = pivot + offsetY;

            pgBbox.AddPoint(pointX);
            pgBbox.AddPoint(pointY);
            pgBbox.AddPoint(pointZ);
        }
    }

    return pgBbox;
}

};
