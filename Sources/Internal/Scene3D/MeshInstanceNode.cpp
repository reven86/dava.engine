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


#include "Scene3D/MeshInstanceNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/3D/StaticMesh.h"
#include "Render/RenderHelper.h"
#include "Utils/StringFormat.h"
#include "Scene3D/ShadowVolumeNode.h"
#include "Debug/Stats.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"

namespace DAVA 
{
    
PolygonGroupWithMaterial::~PolygonGroupWithMaterial()
{
    SafeRelease(mesh);
}

void PolygonGroupWithMaterial::Setup(StaticMesh* _mesh, int32 _polygroupIndex, NMaterial* _material, TransformComponent* _transform)
{
    SetMaterial(_material);
    mesh = SafeRetain(_mesh);
    polygroupIndex = _polygroupIndex;
    transform = _transform;
}

StaticMesh * PolygonGroupWithMaterial::GetMesh()
{
    return mesh;
}
    
int32 PolygonGroupWithMaterial::GetPolygroupIndex()
{
    return polygroupIndex;
}

PolygonGroup * PolygonGroupWithMaterial::GetPolygonGroup()
{
    return mesh->GetPolygonGroup(polygroupIndex);
}

uint64 PolygonGroupWithMaterial::GetSortID()
{
    return 0;
}

MeshInstanceNode::MeshInstanceNode()
:	Entity()
{
}
	
MeshInstanceNode::~MeshInstanceNode()
{
	ClearLightmaps();

    for (int32 idx = 0; idx < (int32)polygroups.size(); ++idx)
    {
        SafeRelease(polygroups[idx]);
    }
    polygroups.clear();
}

void MeshInstanceNode::AddPolygonGroup(StaticMesh* mesh, int32 polygonGroupIndex, NMaterial* material)
{
    PolygonGroupWithMaterial * polygroup = new PolygonGroupWithMaterial();
    polygroup->Setup(mesh, polygonGroupIndex, material, (TransformComponent*)GetComponent(Component::TRANSFORM_COMPONENT));
	polygroups.push_back(polygroup);
	
	PolygonGroup * group = polygroup->GetPolygonGroup();
	bbox.AddAABBox(group->GetBoundingBox());
}
    
uint32 MeshInstanceNode::GetRenderBatchCount()
{
    return (uint32)polygroups.size();
}

RenderBatch * MeshInstanceNode::GetRenderBatch(uint32 batchIndex)
{
    return polygroups[batchIndex];
}
    
void MeshInstanceNode::Update(float32 timeElapsed)
{
    //Stats::Instance()->BeginTimeMeasure("Scene.Update.MeshInstanceNode.Update", this);

    bool needUpdateTransformBox = false;
    if (!(flags & NODE_WORLD_MATRIX_ACTUAL)) 
    {
        needUpdateTransformBox = true;
        UpdateLights();
    }
    else
    {
        if(GetScene()->GetFlags() & SCENE_LIGHTS_MODIFIED)
		{
			UpdateLights();
		}
    }
    if (needUpdateTransformBox)
	{
        bbox.GetTransformedBox(GetWorldTransform(), transformedBox);
		//entity->SetData("meshAABox", transformedBox);
	}
	//entity->SetData("meshInstanceNode", this);
	//entity->SetData("transform", worldTransform);

    //Stats::Instance()->EndTimeMeasure("Scene.Update.MeshInstanceNode.Update", this);
}
    
uint64 MeshInstanceNode::GetSortID()
{
    return 0; 
}

    
void MeshInstanceNode::Draw()
{
    //Stats::Instance()->BeginTimeMeasure("Scene.Draw.MeshInstanceNode.Draw", this);

#if 0
    if (!(flags & NODE_VISIBLE) || !(flags & NODE_UPDATABLE) || (flags & NODE_INVALID))return;

//    if (GetFullName() == String("MaxScene->node-Cylinder01->VisualSceneNode14->instance_0"))
//    {
//        RenderSystem2D::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//        RenderHelper::Instance()->DrawBox(transformedBox);
//        RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
//    }    

	//now clipping in entity system
    if (flags & NODE_CLIPPED_THIS_FRAME)
    {
        // !scene->GetClipCamera()->GetFrustum()->IsInside(transformedBox)
        //return;
    }
		
	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	Matrix4 meshFinalMatrix = *(((TransformComponent*)GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransform()) * prevMatrix;
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);

//    /* float32 proj[16];
//    glGetFloatv(GL_MODELVIEW_MATRIX, proj);
//    
//    for (int32 k = 0; k < 16; ++k)
//    {
//        if (proj[k] != prevMatrix.data[k])
//        {
//            printf("k:%d - %0.3f = %0.3f\n", k, proj[k], prevMatrix.data[k]);
//        }
//    } */
//    
    
    //glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    //glMultMatrixf(worldTransform.data);
    
    uint32 meshesSize = (uint32)polygroups.size();

    // RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE & (~RenderStateBlock::STATE_CULL));
    
    for (uint32 k = 0; k < meshesSize; ++k)
    {
        PolygonGroupWithMaterial * polygroup = polygroups[k];
        
        NMaterial * nMaterial = polygroup->GetNMaterial();
        
        if (nMaterial)
        {
            nMaterial->PrepareRenderState(polygroup->GetPolygonGroup(), polygroup->GetNMaterialInstance());
            nMaterial->Draw(polygroup->GetPolygonGroup(), polygroup->GetNMaterialInstance());
        }
        else
        {
            
            if (polygroup->material->type == Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP)
            {
                LightmapData * data = GetLightmapDataForIndex(k);
                if(data && data->lightmap)
                {
                    polygroup->material->SetSetupLightmap(false);
                    polygroup->material->SetTexture(Material::TEXTURE_DECAL, data->lightmap);
                    polygroup->material->SetUvOffset(data->uvOffset);
                    polygroup->material->SetUvScale(data->uvScale);
                }
                else
                {
                    polygroup->material->SetSetupLightmap(true);
                    polygroup->material->SetSetupLightmapSize(GetCustomProperties()->GetInt32(DAVA::Format("#%d.lightmap.size", k), 128));
                }
            }

    //        if (polygroup->material->type == Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP)
    //        {
    //            polygroup->material->type = Material::MATERIAL_UNLIT_TEXTURE;
    //        }
    //        if (polygroup->material->type != Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP)
            polygroup->material->Draw(polygroup->GetPolygonGroup(), materialState);
        }
    }
#if 0
	if (debugFlags != DEBUG_DRAW_NONE)
	{
        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
		uint32 oldState = RenderManager::Instance()->GetState();
        RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST); 
		
		if (debugFlags & DEBUG_DRAW_LOCAL_AXIS)
		{
			RenderSystem2D::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f); 
			RenderHelper::Instance()->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(50.0f, 0.0f, 0.0f));
			
			RenderSystem2D::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
			RenderHelper::Instance()->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 50.0f, 0.0f));
			
			RenderSystem2D::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
			RenderHelper::Instance()->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 50.0f));
		}

//		if (debugFlags & DEBUG_DRAW_AABOX_CORNERS)
//		{
//			RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
//			RenderHelper::Instance()->DrawCornerBox(bbox);
//      }
        
        if (debugFlags & DEBUG_DRAW_NORMALS)
        {
            
            //const Matrix4 & modelView = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
            // const Matrix3 & normalMatrix = RenderManager::Instance()->GetNormalMatrix();
            
            
            const float32 DEBUG_VECTOR_LENGTH = 5.0f;
            
            
            for (uint32 k = 0; k < meshesSize; ++k)
            {
                PolygonGroup * pGroup = polygroups[k]->mesh->GetPolygonGroup(polygroups[k]->polygroupIndex);
                for (int vi = 0; vi < pGroup->GetVertexCount(); ++vi)
                {
                    Vector3 vertex;
                    Vector3 normal;
                    Vector3 tangent;
                    Vector3 binormal;
                    
                    pGroup->GetCoord(vi, vertex);
                    if (pGroup->GetFormat() & EVF_NORMAL)
                    {
                        pGroup->GetNormal(vi, normal);
                    }
                    if (pGroup->GetFormat() & EVF_TANGENT)
                    {
                        pGroup->GetTangent(vi, tangent);
                    }
                    if (pGroup->GetFormat() & EVF_BINORMAL)
                    {
                        pGroup->GetBinormal(vi, binormal);
                    }else
                    {
                        binormal = CrossProduct(normal, tangent);
                    }
                    Vector3 vertex2;
                     vertex2 = vertex + normal * DEBUG_VECTOR_LENGTH;
                    Color color(normal.x * 0.5f + 0.5f, normal.y * 0.5f + 0.5f, normal.z * 0.5f + 0.5f, 1.0f);
                    RenderSystem2D::Instance()->SetColor(color);
                    RenderHelper::Instance()->DrawLine(vertex, vertex2);
                    
                    vertex2 = vertex + tangent * DEBUG_VECTOR_LENGTH;
                    Color tcolor(0.0f, 1.0f, 0.0f, 1.0f);
                    RenderSystem2D::Instance()->SetColor(tcolor);
                    RenderHelper::Instance()->DrawLine(vertex, vertex2);
                    
                    vertex2 = vertex + binormal * DEBUG_VECTOR_LENGTH;
                    Color bcolor(0.0f, 0.0f, 1.0f, 1.0f);
                    RenderSystem2D::Instance()->SetColor(bcolor);
                    RenderHelper::Instance()->DrawLine(vertex, vertex2);
                    
                    
                }

            }
            
        }
		
//      RenderManager::Instance()->EnableDepthTest(true);
//		RenderManager::Instance()->EnableTexturing(true);
        RenderManager::Instance()->SetState(oldState);
        RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
#endif
	//glPopMatrix();
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);

	Entity::Draw();
#endif
    //Stats::Instance()->EndTimeMeasure("Scene.Draw.MeshInstanceNode.Draw", this);
}


Entity* MeshInstanceNode::Clone(Entity *dstNode)
{
    if (!dstNode) 
    {
		DVASSERT_MSG(IsPointerToExactClass<MeshInstanceNode>(this), "Can clone only MeshInstanceNode");
        dstNode = new MeshInstanceNode();
    }

    Entity::Clone(dstNode);
    MeshInstanceNode *nd = (MeshInstanceNode *)dstNode;
    
    nd->polygroups = polygroups;
    for (int32 k = 0; k < (int32) polygroups.size(); ++k)
    {
        nd->polygroups[k]->Retain();
    }
    
    nd->bbox = bbox;
    
    return dstNode;
}

AABBox3 MeshInstanceNode::GetWTMaximumBoundingBoxSlow()
{
	AABBox3 retBBox = transformedBox;
    
    const Vector<Entity*>::iterator & itEnd = children.end();
	for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
    {
        AABBox3 box = (*it)->GetWTMaximumBoundingBoxSlow();
        if(  (AABBOX_INFINITY != box.min.x && AABBOX_INFINITY != box.min.y && AABBOX_INFINITY != box.min.z)
           &&(-AABBOX_INFINITY != box.max.x && -AABBOX_INFINITY != box.max.y && -AABBOX_INFINITY != box.max.z))
        {
            retBBox.AddAABBox(box);
        }
    }
    
    return retBBox;
}
    
void MeshInstanceNode::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
#if 0
    Entity::Save(archive, serializationContext);
//    archive->SetInt32("lodCount", (int32)lodLayers.size());
//    
//    int32 lodIdx = 0;
//    //const List<LodData>::iterator & end = lodLayers.end();
//    //for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
//    {
//        LodData & ld = *it;
//        size_t size = ld.materials.size();
//        size_t sizeMeshes = ld.meshes.size();
//        DVASSERT(size == sizeMeshes);
//        archive->SetInt32(Format("lod%d_cnt", lodIdx), (int32)size);
//        for (size_t idx = 0; idx < size; ++idx)
//        {
//            Material * material = ld.materials[idx];
//            StaticMesh * mesh = ld.meshes[idx];
//            int32 pgIndex = ld.polygonGroupIndexes[idx];
//            
//            archive->SetByteArrayAsType(Format("l%d_%d_matptr", lodIdx, idx), (uint64)material);
//            archive->SetByteArrayAsType(Format("l%d_%d_meshptr", lodIdx, idx), (uint64)mesh);
//            archive->SetInt32(Format("l%d_%d_pg", lodIdx, idx), pgIndex);
//            
//            //DVASSERT(meshIndex != -1 && materialIndex != -1 && pgIndex != -1)
//        }
//        lodIdx++;
//    }

    int32 polygroupCount = (int32)polygroups.size();
    archive->SetInt32("pgcnt", polygroupCount);
    
    for (int idx = 0; idx < polygroupCount; ++idx)
    {
        Material * material = polygroups[idx]->material;
        StaticMesh * mesh = polygroups[idx]->mesh;
        int32 pgIndex = polygroups[idx]->polygroupIndex;
        
        archive->SetByteArrayAsType(Format("pg%d_matptr", idx), (uint64)material);
        archive->SetByteArrayAsType(Format("pg%d_meshptr", idx), (uint64)mesh);
        archive->SetInt32(Format("pg%d_pg", idx), pgIndex);
    }
    
	archive->SetInt32("lightmapsCount", (int32)lightmaps.size());
	int32 lightmapIndex = 0;
	Vector<LightmapData>::iterator lighmapsEnd = lightmaps.end();
	for(Vector<LightmapData>::iterator lightmapsIterator = lightmaps.begin(); lightmapsIterator != lighmapsEnd; ++lightmapsIterator)
	{
		LightmapData & data = *lightmapsIterator;
		String filename = data.lightmapName.GetRelativePathname(serializationContext->GetScenePath());

		archive->SetString(Format("lightmap%d", lightmapIndex), filename.c_str());
		archive->SetFloat(Format("lightmap%duvoX", lightmapIndex), data.uvOffset.x);
		archive->SetFloat(Format("lightmap%duvoY", lightmapIndex), data.uvOffset.y);
		archive->SetFloat(Format("lightmap%duvsX", lightmapIndex), data.uvScale.x);
		archive->SetFloat(Format("lightmap%duvsY", lightmapIndex), data.uvScale.y);
		lightmapIndex++;
	}
#endif
}

void MeshInstanceNode::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    Entity::Load(archive, serializationContext);

    static const int32 errorIdx = -1;

    if(serializationContext->GetVersion() >= 3)
    {
        int32 polygroupCount = archive->GetInt32("pgcnt", 0);
        
        for(int idx = 0; idx < polygroupCount; ++idx)
        {
            uint64 matPtr = archive->GetByteArrayAsType(Format("pg%d_matptr", idx), (uint64)0);
            NMaterial* material = static_cast<NMaterial*>(serializationContext->GetDataBlock(matPtr));
            uint64 meshPtr = archive->GetByteArrayAsType(Format("pg%d_meshptr", idx), (uint64)0);
            StaticMesh * mesh = static_cast<StaticMesh*>(serializationContext->GetDataBlock(meshPtr));
            const int32 pgIndex = archive->GetInt32(Format("pg%d_pg", idx), errorIdx);

            if(material && mesh)
            {
                DVASSERT(pgIndex != errorIdx);

                //if(serializationContext->IsDebugLogEnabled())
                //    Logger::FrameworkDebug("+ assign material: %s", material->GetName().c_str());
                
                AddPolygonGroup(mesh, pgIndex, material);
            }
        }
    }
    else
    {
        //int32 lodCount = archive->GetInt32("lodCount", 0);
        
        //for (int32 lodIdx = 0; lodIdx < lodCount; ++lodIdx)
        int32 lodIdx = 0;
        {
            size_t size = archive->GetInt32(Format("lod%d_cnt", lodIdx), 0);
            for(size_t idx = 0; idx < size; ++idx)
            {
                if(serializationContext->GetVersion() == 2)
                {
                    uint64 matPtr = archive->GetByteArrayAsType(Format("l%d_%d_matptr", lodIdx, idx), (uint64)0);
                    NMaterial* material = static_cast<NMaterial*>(serializationContext->GetDataBlock(matPtr));
                    uint64 meshPtr = archive->GetByteArrayAsType(Format("l%d_%d_meshptr", lodIdx, idx), (uint64)0);
                    StaticMesh * mesh = static_cast<StaticMesh*>(serializationContext->GetDataBlock(meshPtr));
                    const int32 pgIndex = archive->GetInt32(Format("l%d_%d_pg", lodIdx, idx), errorIdx);

                    if(material && mesh)
                    {
                        DVASSERT(pgIndex != errorIdx);

                        //if(serializationContext->IsDebugLogEnabled())
                        //    Logger::FrameworkDebug("+ assign material: %s", material->GetName().c_str());
                        
                        AddPolygonGroup(mesh, pgIndex, material);
                    }
                }

                if(serializationContext->GetVersion() == 1)
                {
                    serializationContext->SetLastError(SceneFileV2::ERROR_VERSION_IS_TOO_OLD);
    //                int32 materialIndex = archive->GetInt32(Format("l%d_%d_mat", lodIdx, idx), -1);
    //                int32 meshIndex = archive->GetInt32(Format("l%d_%d_ms", lodIdx, idx), -1);
    //                int32 pgIndex = archive->GetInt32(Format("l%d_%d_pg", lodIdx, idx), -1);
    //            
    //            
    //            
    //                if ((materialIndex != -1) && (meshIndex != -1) && (pgIndex != -1))
    //                {
    //                    Material * material = sceneFile->GetMaterial(materialIndex);
    //                    StaticMesh * mesh = sceneFile->GetStaticMesh(meshIndex);
    //                    Logger::FrameworkDebug("+ assign material: %s index: %d", material->GetName().c_str(), materialIndex);
    //                    
    //                    AddPolygonGroupForLayer(mesh, pgIndex, material);
    //                }
    //                else
    //                {
    //                    DVASSERT(0 && "Negative element")
    //                }
                }
                
            }
        }
    }

//    if (polygroups[0]->GetMaterial()->type == Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP)
    {
        int32 lightmapsCount = archive->GetInt32("lightmapsCount", 0);
        for(int32 i = 0; i < lightmapsCount; ++i)
        {
			LightmapData data;

            String pathname = archive->GetString(Format("lightmap%d", i), "");
            data.lightmapName = serializationContext->GetScenePath() + pathname;
			data.uvOffset.x = archive->GetFloat(Format("lightmap%duvoX", i));
			data.uvOffset.y = archive->GetFloat(Format("lightmap%duvoY", i));
			data.uvScale.x = archive->GetFloat(Format("lightmap%duvsX", i));
			data.uvScale.y = archive->GetFloat(Format("lightmap%duvsY", i));

            AddLightmap(i, data);
        }
    }
}
    
Vector<PolygonGroupWithMaterial*> & MeshInstanceNode::GetPolygonGroups()
{
    return polygroups;
}

void MeshInstanceNode::AddLightmap(int32 polygonGroupIndex, const LightmapData & lightmapData)
{
	LightmapData data = lightmapData;
	data.lightmap = Texture::CreateFromFile(data.lightmapName);

	if(polygonGroupIndex > ((int32)lightmaps.size()-1))
	{
		lightmaps.resize(polygonGroupIndex+1);
	}

	lightmaps[polygonGroupIndex] = data;
}

void MeshInstanceNode::AddLightmap(const LightmapData & lightmapData)
{
	AddLightmap(static_cast<int32>(lightmaps.size()), lightmapData);
}

void MeshInstanceNode::ClearLightmaps()
{
	Vector<LightmapData>::iterator lighmapsEnd = lightmaps.end();
	for(Vector<LightmapData>::iterator lightmapsIterator = lightmaps.begin(); lightmapsIterator != lighmapsEnd; ++lightmapsIterator)
	{
		LightmapData & data = (*lightmapsIterator);
		SafeRelease(data.lightmap);
	}

	lightmaps.clear();
}

MeshInstanceNode::LightmapData * MeshInstanceNode::GetLightmapDataForIndex(int32 index)
{
	if(index < (int32)lightmaps.size())
	{
		return &(lightmaps[index]);
	}
	else
	{
		return 0;
	}
}

int32 MeshInstanceNode::GetLightmapCount()
{
    return static_cast<int32>(lightmaps.size());
}
    
void MeshInstanceNode::CreateDynamicShadowNode()
{
	ShadowVolumeNode * shadowVolume = new ShadowVolumeNode();
	shadowVolume->SetName("dynamicshadow.shadowvolume");

	shadowVolume->CopyGeometryFrom(this);

	AddNode(shadowVolume);
	shadowVolume->Release();
}

void MeshInstanceNode::DeleteDynamicShadowNode()
{
	ShadowVolumeNode * shadowVolume = (ShadowVolumeNode*)FindByName(FastName("dynamicshadow.shadowvolume"));
	RemoveNode(shadowVolume);
}

void MeshInstanceNode::ConvertToShadowVolume()
{
	ShadowVolumeNode * shadowVolume = new ShadowVolumeNode();
	shadowVolume->SetName("dynamicshadow.shadowvolume");

	shadowVolume->CopyGeometryFrom(this);

	GetParent()->AddNode(shadowVolume);
	shadowVolume->Release();
}

void MeshInstanceNode::GetDataNodes(Set<DataNode*> & dataNodes)
{
//    const List<LodData>::iterator & end = lodLayers.end();
//    for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
//    {
//        LodData & ld = *it;
//        for (int k = 0; k < ld.meshes.size(); ++k)
//        {
//            dataNodes.push_back(ld.meshes[k]->GetPolygonGroup(ld.polygonGroupIndexes[k]));
//        }
        for (int k = 0; k < (int32)polygroups.size(); ++k)
        {
            dataNodes.insert(polygroups[k]->GetMesh());
        }
        for (int k = 0; k < (int32)polygroups.size(); ++k)
        {
            dataNodes.insert(polygroups[k]->GetMaterial());
        }
//    }
    Entity::GetDataNodes(dataNodes);
}
    
void MeshInstanceNode::BakeTransforms()
{
    const Matrix4 & localTransform = GetLocalTransform();

    Set<PolygonGroup*> groupsToBatch;
    
    bool canBakeEverything = true;
    for (int k = 0; k < (int32)polygroups.size(); ++k)
    {
        //Logger::FrameworkDebug("%d - mesh: %d pg: %d", k, polygroups[k]->GetMesh()->GetRetainCount(), polygroups[k]->GetPolygonGroup()->GetRetainCount());
     
        StaticMesh * mesh = polygroups[k]->GetMesh();
        PolygonGroup * polygroup = polygroups[k]->GetPolygonGroup();
        if ((mesh->GetRetainCount() == 1) && (polygroup->GetRetainCount() == 1))
        {
            groupsToBatch.insert(polygroup);
        }
        else
        {
            canBakeEverything = false; 
//            Logger::Warning("WARNING: Can't batch object because it has multiple instances: %s", GetFullName().c_str());
        }
    }   
    if (canBakeEverything)
    {
        bbox = AABBox3(); // reset bbox
        for (Set<PolygonGroup*>::iterator it = groupsToBatch.begin(); it != groupsToBatch.end(); ++it)
        {
            PolygonGroup * polygroup = *it;
            polygroup->ApplyMatrix(localTransform);
            polygroup->BuildBuffers();
            bbox.AddAABBox(polygroup->GetBoundingBox());
        }
        SetLocalTransform(Matrix4::IDENTITY);
        AddFlag(NODE_LOCAL_MATRIX_IDENTITY);
    }
}
    
void MeshInstanceNode::UpdateLights()
{
    Vector3 meshPosition = Vector3() * GetWorldTransform();
    Light * nearestLight = scene->GetNearestDynamicLight(Light::TYPE_COUNT, meshPosition);

    RegisterNearestLight(nearestLight);
}

void MeshInstanceNode::RegisterNearestLight(Light * node)
{
}

bool MeshInstanceNode::HasLightmaps()
{
	return lightmaps.size() > 0;
}


//String MeshInstanceNode::GetDebugDescription()
//{
//    /return Format(": %d ", GetChildrenCount());
//}
    
};
