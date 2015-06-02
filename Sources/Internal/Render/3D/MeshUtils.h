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


#ifndef __DAVAENGINE_MESH_UTILS_H__
#define __DAVAENGINE_MESH_UTILS_H__

#include "PolygonGroup.h"
#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{

class RenderBatch;
class RenderObject;
class Entity;
class SkinnedMesh;
namespace MeshUtils
{
    void RebuildMeshTangentSpace(PolygonGroup *group, bool precomputeBinormal=true);
    void CopyVertex(PolygonGroup *srcGroup, uint32 srcPos, PolygonGroup *dstGroup, uint32 dstPos);
    void CopyGroupData(PolygonGroup *srcGroup, PolygonGroup *dstGroup);

    SkinnedMesh * CreateSkinnedMesh(Entity * fromEntity, Vector<SkeletonComponent::JointConfig> & outJoints);
    PolygonGroup * CreateShadowPolygonGroup(PolygonGroup * source);

    uint32 ReleaseGeometryDataRecursive(Entity * forEntity);

    struct FaceWork
    {
        int32 indexOrigin[3];
        Vector3 tangent, binormal;
    };

    struct VertexWork
    {
        Vector<int32> refIndices;
        Vector3 tangent, binormal;
        int32 tbRatio;
        int32 refIndex;
        int32 resultGroup;
    };

    struct SkinnedMeshWorkKey
    {
        SkinnedMeshWorkKey(int32 _lod, int32 _switch, NMaterial * _materialParent) :
            lodIndex(_lod), switchIndex(_switch), materialParent(_materialParent) {}

        bool operator==(const SkinnedMeshWorkKey & data) const
        {
            return (lodIndex == data.lodIndex) 
                && (switchIndex == data.switchIndex) 
                && (materialParent == data.materialParent);
        }

        bool operator<(const SkinnedMeshWorkKey & data) const
        {
            if (materialParent != data.materialParent)
            {
                return materialParent < data.materialParent;
            }
            else if (lodIndex != data.lodIndex)
            {
                return lodIndex < data.lodIndex;
            }
            else
            {
                return switchIndex < data.switchIndex;
            }
        }

        int32 lodIndex;
        int32 switchIndex;
        NMaterial * materialParent;
    };

    struct SkinnedMeshJointWork
    {
        SkinnedMeshJointWork(RenderBatch * _batch, uint32 _jointIndex) :
            batch(_batch), jointIndex(_jointIndex) {}

        RenderBatch * batch;
        uint32 jointIndex;
    };

    struct EdgeMappingWork
    {
        int32 oldEdge[2];
        int32 newEdge[2][2];

        EdgeMappingWork()
        {
            Memset(oldEdge, -1, sizeof(oldEdge));
            Memset(newEdge, -1, sizeof(newEdge));
        }
    };

    int32 FindEdgeInMappingTable(int32 nV1, int32 nV2, EdgeMappingWork * mapping, int32 count);
};

};

#endif

