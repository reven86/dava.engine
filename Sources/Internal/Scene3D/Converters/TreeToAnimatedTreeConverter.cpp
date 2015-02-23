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

#include "TreeToAnimatedTreeConverter.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#define LEAF_BASE_ANGLE_DIFFERENCE_FACTOR 30
#define LEAF_AMPLITUDE_USERFRIENDLY_FACTOR 0.01f
#define TRUNK_AMPLITUDE_USERFRIENDLY_FACTOR 0.1f

namespace DAVA
{
void TreeToAnimatedTreeConverter::CalculateAnimationParams(SpeedTreeObject * object)
{
    float32 treeHeight = object->GetBoundingBox().GetSize().z;

    uint32 size = object->GetRenderBatchCount();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * rb = object->GetRenderBatch(k);
        PolygonGroup * pg = rb->GetPolygonGroup();
        if (nullptr != pg)
        {
            int32 vertexFormat = pg->GetFormat();
            DVASSERT((vertexFormat & EVF_FLEXIBILITY) > 0);

            int32 vxCount = pg->GetVertexCount();
            for (int32 i = 0; i < vxCount; ++i)
            {
                Vector3 vxPosition;

                pg->GetCoord(i, vxPosition);
                float32 t0 = vxPosition.Length() * LEAF_BASE_ANGLE_DIFFERENCE_FACTOR;

                float32 x = vxPosition.z / treeHeight;
                float32 flexebility = logf((expf(1.0) - 1) * x + 1);

                pg->SetFlexibility(i, flexebility * TRUNK_AMPLITUDE_USERFRIENDLY_FACTOR);

                if (SpeedTreeObject::IsTreeLeafBatch(rb)) //leafs geometry
                {
                    float32 leafHeightOscillationCoeff = (.5f + x / 2);
                    //leafAngle: x: cos(T0);  y: sin(T0)
                    Vector2 leafAngle(cosf(t0) * leafHeightOscillationCoeff * LEAF_AMPLITUDE_USERFRIENDLY_FACTOR,
                        sinf(t0) * leafHeightOscillationCoeff * LEAF_AMPLITUDE_USERFRIENDLY_FACTOR);

                    pg->SetAngle(i, leafAngle);
                }
            }

            pg->BuildBuffers();
        }
    }
}

void TreeToAnimatedTreeConverter::ConvertTrees(Entity *scene)
{
    uniqLeafPGs.clear();
    uniqTrunkPGs.clear();
    uniqTreeObjects.clear();

    ConvertingPathRecursive(scene);

    Set<PolygonGroup *>::iterator leafPGIt = uniqLeafPGs.begin();
    for (; leafPGIt != uniqLeafPGs.end(); ++leafPGIt)
        ConvertLeafPGForAnimations((*leafPGIt));

    Set<PolygonGroup *>::iterator pgTrunkPGIt = uniqTrunkPGs.begin();
    for (; pgTrunkPGIt != uniqTrunkPGs.end(); ++pgTrunkPGIt)
        ConvertTrunkForAnimations((*pgTrunkPGIt));

    Set<SpeedTreeObject *>::iterator treeIt = uniqTreeObjects.begin();
    for (; treeIt != uniqTreeObjects.end(); ++treeIt)
    {
        (*treeIt)->RecalcBoundingBox();
        CalculateAnimationParams((*treeIt));
    }
}

void TreeToAnimatedTreeConverter::ConvertingPathRecursive(Entity * node)
{
    for (int32 c = 0; c < node->GetChildrenCount(); ++c)
    {
        Entity * childNode = node->GetChild(c);
        ConvertingPathRecursive(childNode);
    }

    RenderComponent *rc = GetRenderComponent(node);
    if (nullptr == rc)
    {
        return;
    }

    RenderObject *ro = rc->GetRenderObject();
    if (nullptr == ro)
    {
        return;
    }

    bool isSpeedTree = false;

    uint32 count = ro->GetRenderBatchCount();
    for (uint32 b = 0; b < count && !isSpeedTree; ++b)
    {
        RenderBatch *renderBatch = ro->GetRenderBatch(b);
        isSpeedTree |= SpeedTreeObject::IsTreeLeafBatch(renderBatch);
    }

    if (!isSpeedTree)
    {
        return;
    }
    SpeedTreeObject * treeObject = cast_if_equal<SpeedTreeObject*>(ro);
    if (nullptr == treeObject)
    {
        treeObject = new SpeedTreeObject();
        ro->Clone(treeObject);
        rc->SetRenderObject(treeObject);
        treeObject->Release();
    }

    node->AddComponent(new SpeedTreeComponent());

    uniqTreeObjects.insert(treeObject);

    uint32 size = treeObject->GetRenderBatchCount();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * rb = treeObject->GetRenderBatch(k);
        PolygonGroup * pg = rb->GetPolygonGroup();
        if (SpeedTreeObject::IsTreeLeafBatch(rb))
            uniqLeafPGs.insert(pg);
        else
            uniqTrunkPGs.insert(pg);
    }
}

void TreeToAnimatedTreeConverter::ConvertLeafPGForAnimations(PolygonGroup * pg)
{
    int32 vertexFormat = pg->GetFormat();
    int32 vxCount = pg->GetVertexCount();
    int32 indCount = pg->GetIndexCount();

    int32 oldLeafFormat = (EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_TANGENT);
    DVASSERT((vertexFormat & oldLeafFormat) == oldLeafFormat); //old tree leaf vertex format

    PolygonGroup * pgCopy = new PolygonGroup();
    pgCopy->AllocateData(vertexFormat, vxCount, indCount);

    Memcpy(pgCopy->meshData, pg->meshData, vxCount*pg->vertexStride);
    Memcpy(pgCopy->indexArray, pg->indexArray, indCount*sizeof(int16));

    pg->ReleaseData();
    pg->AllocateData(EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_PIVOT | EVF_FLEXIBILITY | EVF_ANGLE_SIN_COS, vxCount, indCount);

    //copy indices
    for (int32 i = 0; i < indCount; ++i)
    {
        int32 index;
        pgCopy->GetIndex(i, index);
        pg->SetIndex(i, index);
    }

    //copy vertex data
    for (int32 i = 0; i < vxCount; ++i)
    {
        Vector3 vxPosition;
        uint32 color;
        Vector2 vxTx;
        Vector3 vxPivot;

        pgCopy->GetCoord(i, vxPosition);
        pgCopy->GetColor(i, color);
        pgCopy->GetTexcoord(0, i, vxTx);
        pgCopy->GetTangent(i, vxPivot);

        pg->SetCoord(i, vxPosition);
        pg->SetColor(i, color);
        pg->SetTexcoord(0, i, vxTx);
        pg->SetPivot(i, vxPivot);
    }
    SafeRelease(pgCopy);

    pg->BuildBuffers();
}

void TreeToAnimatedTreeConverter::ConvertTrunkForAnimations(PolygonGroup * pg)
{
    int32 vertexFormat = pg->GetFormat();
    int32 vxCount = pg->GetVertexCount();
    int32 indCount = pg->GetIndexCount();

    int32 oldTrunkFormat = (EVF_VERTEX | EVF_TEXCOORD0);
    DVASSERT((vertexFormat & oldTrunkFormat) == oldTrunkFormat); //old tree trunk vertex format

    PolygonGroup * pgCopy = new PolygonGroup();
    pgCopy->AllocateData(vertexFormat, vxCount, indCount);

    Memcpy(pgCopy->meshData, pg->meshData, vxCount*pg->vertexStride);
    Memcpy(pgCopy->indexArray, pg->indexArray, indCount*sizeof(int16));

    pg->ReleaseData();
    pg->AllocateData(EVF_VERTEX | EVF_TEXCOORD0 | EVF_FLEXIBILITY, vxCount, indCount);

    //copy indices
    for (int32 i = 0; i < indCount; ++i)
    {
        int32 index;
        pgCopy->GetIndex(i, index);
        pg->SetIndex(i, index);
    }

    //copy vertex data
    for (int32 i = 0; i < vxCount; ++i)
    {
        Vector3 vxPosition;
        Vector2 vxTx;

        pgCopy->GetCoord(i, vxPosition);
        pgCopy->GetTexcoord(0, i, vxTx);

        pg->SetCoord(i, vxPosition);
        pg->SetTexcoord(0, i, vxTx);
    }
    SafeRelease(pgCopy);

    pg->BuildBuffers();
}

};