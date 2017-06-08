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
void RebuildMeshTangentSpace(PolygonGroup* group, bool precomputeBinormal = true);
void CopyVertex(PolygonGroup* srcGroup, uint32 srcPos, PolygonGroup* dstGroup, uint32 dstPos);
void CopyGroupData(PolygonGroup* srcGroup, PolygonGroup* dstGroup);

SkinnedMesh* CreateSkinnedMesh(Entity* fromEntity, Vector<SkeletonComponent::JointConfig>& outJoints);
PolygonGroup* CreateShadowPolygonGroup(PolygonGroup* source);

uint32 ReleaseGeometryDataRecursive(Entity* forEntity);

Vector<uint16> BuildSortedIndexBufferData(PolygonGroup* pg, Vector3 direction);

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
    SkinnedMeshWorkKey(int32 _lod, int32 _switch, NMaterial* _materialParent)
        :
        lodIndex(_lod)
        , switchIndex(_switch)
        , materialParent(_materialParent)
    {
    }

    bool operator==(const SkinnedMeshWorkKey& data) const
    {
        return (lodIndex == data.lodIndex)
        && (switchIndex == data.switchIndex)
        && (materialParent == data.materialParent);
    }

    bool operator<(const SkinnedMeshWorkKey& data) const
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
    NMaterial* materialParent;
};

struct SkinnedMeshJointWork
{
    SkinnedMeshJointWork(RenderBatch* _batch, uint32 _jointIndex)
        :
        batch(_batch)
        , jointIndex(_jointIndex)
    {
    }

    RenderBatch* batch;
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

int32 FindEdgeInMappingTable(int32 nV1, int32 nV2, EdgeMappingWork* mapping, int32 count);
};
};

#endif
