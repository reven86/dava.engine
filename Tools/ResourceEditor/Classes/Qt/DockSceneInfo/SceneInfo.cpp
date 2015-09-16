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


#include "DAVAEngine.h"
#include "DockSceneInfo/SceneInfo.h"
#include "../Qt/Settings/SettingsManager.h"
#include "CommandLine/CommandLineManager.h"
#include "Project/ProjectManager.h"

#include "Main/QtUtils.h"

#include "Render/TextureDescriptor.h"

#include "ImageTools/ImageTools.h"

#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneHelper.h"
#include "Scene/System/EditorLODSystem.h"
#include "Main/mainwindow.h"

#include <QHeaderView>
#include <QTimer>
#include <QPalette>

#include "Render/Material/NMaterialNames.h"

#include "Scene3D/Components/ComponentHelpers.h"


using namespace DAVA;

SceneInfo::SceneInfo(QWidget *parent /* = 0 */)
	: QtPropertyEditor(parent)
	, treeStateHelper(this, curModel)
	, activeScene(NULL)
	, landscape(NULL)
	, isUpToDate(false)
{
	// global scene manager signals
    connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), SLOT(SceneActivated(SceneEditor2 *)));
    connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), SLOT(SceneDeactivated(SceneEditor2 *)));
    connect(SceneSignals::Instance(), SIGNAL(StructureChanged(SceneEditor2 *, DAVA::Entity *)), SLOT(SceneStructureChanged(SceneEditor2 *, DAVA::Entity *)));
    connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), SLOT(SceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));

	// MainWindow actions
	posSaver.Attach(this, "DockSceneInfo");
	
	VariantType v = posSaver.LoadValue("splitPos");
	if(v.GetType() == VariantType::TYPE_INT32) header()->resizeSection(0, v.AsInt32());

    ClearData();
    
    InitializeInfo();
    
    viewport()->setBackgroundRole(QPalette::Window);
}

SceneInfo::~SceneInfo()
{
	VariantType v(header()->sectionSize(0));
	posSaver.SaveValue("splitPos", v);
}

void SceneInfo::LODInfo::AddTriangles(int32 index, int32 count)
{
    if (index==-1)
        trianglesOnObjects += count;
    else
        trianglesOnLod[index] += count;
}

void SceneInfo::InitializeInfo()
{
    RemovePropertyAll();
    
    InitializeGeneralSection();
    Initialize3DDrawSection();
    InitializeLODSectionInFrame();
    InitializeLODSectionForSelection();
    InitializeSpeedTreeInfoSelection();
    InitializeLayersSection();
    
    InitializeVegetationInfoSection();
}

void SceneInfo::InitializeGeneralSection()
{
    QtPropertyData* header = CreateInfoHeader("General Scene Info");
  
    AddChild("Entities Count", header);
    AddChild("Emitters Count", header);

    AddChild("All Textures Size", header);
	AddChild("Material Textures Size", header);
    AddChild("Particles Textures Size", header);
    
    AddChild("Sprites Count", header);
    AddChild("Particle Textures Count", header);
}

void SceneInfo::RefreshSceneGeneralInfo()
{
    QtPropertyData* header = GetInfoHeader("General Scene Info");

    SetChild("Entities Count", (uint32)nodesAtScene.size(), header);
    SetChild("Emitters Count", emittersCount, header);

    SetChild("All Textures Size", QString::fromStdString(SizeInBytesToString((float32)(sceneTexturesSize + particleTexturesSize))), header);
	SetChild("Material Textures Size", QString::fromStdString(SizeInBytesToString((float32)sceneTexturesSize)), header);
    SetChild("Particles Textures Size", QString::fromStdString(SizeInBytesToString((float32)particleTexturesSize)), header);
    
    SetChild("Sprites Count", spritesCount, header);
    SetChild("Particle Textures Count", (uint32)particleTextures.size(), header);
}

void SceneInfo::Initialize3DDrawSection()
{
    QtPropertyData* header = CreateInfoHeader("DrawInfo");

    AddChild("Visible Render Object Count", header);
    AddChild("Occluded Object Count",  header);
    AddChild("ArraysCalls", header);
    AddChild("ElementsCalls",  header);
    AddChild("PointsList", header);
    AddChild("LineList", header);
    AddChild("LineStrip", header);
    AddChild("TriangleList", header);
    AddChild("TriangleStrip", header);
    AddChild("TriangleFan", header);

    QtPropertyData* header2 = CreateInfoHeader("Bind Info");
    AddChild("Dynamic Param Bind Count", header2);
    AddChild("Material Param Bind Count", header2);
    
}

void SceneInfo::Refresh3DDrawInfo()
{
    if(!activeScene) return;
#if RHI_COMPLETE_EDITOR
    QtPropertyData* header = GetInfoHeader("DrawInfo");
    
    const RenderManager::Stats & renderStats = activeScene->GetRenderStats();

    
    SetChild("Visible Render Object Count", renderStats.visibleRenderObjectCount, header);
    SetChild("Occluded Object Count", renderStats.occludedRenderObjectCount, header);

    
    SetChild("ArraysCalls", renderStats.drawArraysCalls, header);
    SetChild("ElementsCalls", renderStats.drawElementsCalls, header);
    SetChild("PointsList", renderStats.primitiveCount[PRIMITIVETYPE_POINTLIST], header);
    SetChild("LineList", renderStats.primitiveCount[PRIMITIVETYPE_LINELIST], header);
    SetChild("LineStrip", renderStats.primitiveCount[PRIMITIVETYPE_LINESTRIP], header);
    SetChild("TriangleList", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLELIST], header);
    SetChild("TriangleStrip", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLESTRIP], header);
    SetChild("TriangleFan", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLEFAN], header);
    
    QtPropertyData* header2 = GetInfoHeader("Bind Info");

    SetChild("Dynamic Param Bind Count", renderStats.dynamicParamUniformBindCount, header2);
    SetChild("Material Param Bind Count", renderStats.materialParamUniformBindCount, header2);
#endif // RHI_COMPLETE_EDITOR

}


void SceneInfo::InitializeSpeedTreeInfoSelection()
{
    QtPropertyData* header = CreateInfoHeader("SpeedTree Info");
    
    AddChild("SpeedTree Leafs Square", header);
    AddChild("SpeedTree Leafs Square Div X", header);
    AddChild("SpeedTree Leafs Square Div Y", header);
}

void SceneInfo::RefreshSpeedTreeInfoSelection()
{
    QtPropertyData* header = GetInfoHeader("SpeedTree Info");
    
    float32 speedTreeLeafSquare = 0.f, speedTreeLeafSquareDivX = 0.f, speedTreeLeafSquareDivY = 0.f;
    int32 infoCount = speedTreeLeafInfo.size();
    for(int32 i = 0; i < infoCount; i++)
    {
        SpeedTreeInfo & info = speedTreeLeafInfo[i];
        speedTreeLeafSquare += info.leafsSquare;
        speedTreeLeafSquareDivX += info.leafsSquareDivX;
        speedTreeLeafSquareDivY += info.leafsSquareDivY;
    }

    SetChild("SpeedTree Leafs Square", speedTreeLeafSquare, header);
    SetChild("SpeedTree Leafs Square Div X", speedTreeLeafSquareDivX, header);
    SetChild("SpeedTree Leafs Square Div Y", speedTreeLeafSquareDivY, header);
}

void SceneInfo::InitializeLODSectionInFrame()
{
    QtPropertyData* header = CreateInfoHeader("LOD in Frame");
    
    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        AddChild(Format("Objects LOD%d Triangles", i).c_str(), header);
    }
    
    AddChild("All LOD Triangles", header);
    AddChild("Objects without LOD Triangles", header);
    AddChild("Landscape Triangles", header);
    AddChild("All Triangles", header);
}

void SceneInfo::InitializeLODSectionForSelection()
{
    QtPropertyData* header = CreateInfoHeader("LOD Info for Selected Entities");
    
    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        AddChild(Format("Objects LOD%d Triangles", i).c_str(), header);
    }
    
    AddChild("All LOD Triangles", header);
    AddChild("Objects without LOD Triangles", header);
    AddChild("All Triangles", header);
}


void SceneInfo::RefreshLODInfoInFrame()
{
    QtPropertyData* header = GetInfoHeader("LOD in Frame");

    uint32 lodTriangles = 0;
    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        SetChild(Format("Objects LOD%d Triangles", i).c_str(), lodInfoInFrame.trianglesOnLod[i], header);
        
        lodTriangles += lodInfoInFrame.trianglesOnLod[i];
    }
    
    int32 landTriangles = (landscape) ? landscape->GetDrawIndices() : 0;
    landTriangles /= 3;
    SetChild("All LOD Triangles", lodTriangles, header);
    SetChild("Objects without LOD Triangles", lodInfoInFrame.trianglesOnObjects, header);
    SetChild("Landscape Triangles", landTriangles, header);
    SetChild("All Triangles", lodInfoInFrame.trianglesOnObjects + landTriangles + lodTriangles, header);
}

void SceneInfo::RefreshLODInfoForSelection()
{
    QtPropertyData* header = GetInfoHeader("LOD Info for Selected Entities");
    
    uint32 lodTriangles = 0;
    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        SetChild(Format("Objects LOD%d Triangles", i).c_str(), lodInfoSelection.trianglesOnLod[i], header);
        
        lodTriangles += lodInfoSelection.trianglesOnLod[i];
    }
    
    SetChild("All LOD Triangles", lodTriangles, header);
    
    SetChild("Objects without LOD Triangles", lodInfoSelection.trianglesOnObjects, header);
    SetChild("All Triangles", lodInfoSelection.trianglesOnObjects + lodTriangles, header);
}


uint32 SceneInfo::CalculateTextureSize(const TexturesMap &textures)
{
	String projectPath = ProjectManager::Instance()->CurProjectPath().GetAbsolutePathname();
    uint32 textureSize = 0;

    eGPUFamily requestedGPU = static_cast<eGPUFamily>(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32());

    TexturesMap::const_iterator endIt = textures.end();
    for(TexturesMap::const_iterator it = textures.begin(); it != endIt; ++it)
    {
        FilePath pathname = it->first;
        Texture *tex = it->second;
        DVASSERT(tex);

		if (tex->IsPinkPlaceholder())
		{
			continue;
		}

        if(String::npos == pathname.GetAbsolutePathname().find(projectPath))
        {
            Logger::Warning("[SceneInfo::CalculateTextureSize] Path (%s) doesn't belong to project.", pathname.GetAbsolutePathname().c_str());
            continue;
        }
        
        auto baseMipmap = tex->GetBaseMipMap();

		auto descriptor = tex->GetDescriptor();
		eGPUFamily gpu = descriptor->IsCompressedFile() ? static_cast<eGPUFamily>(descriptor->exportedAsGpuFamily) : requestedGPU;
        textureSize += ImageTools::GetTexturePhysicalSize(tex->GetDescriptor(), gpu, baseMipmap);
    }

    return textureSize;
}



void SceneInfo::CollectSceneData(SceneEditor2 *scene)
{
    ClearData();

    if(scene)
    {
        scene->GetChildNodes(nodesAtScene);

        SceneHelper::EnumerateSceneTextures(activeScene, sceneTextures, SceneHelper::TexturesEnumerateMode::EXCLUDE_NULL);
        sceneTexturesSize = CalculateTextureSize(sceneTextures);

        CollectParticlesData();
        particleTexturesSize = CalculateTextureSize(particleTextures);
        
        CollectLODDataInFrame();        
        
    }
}

void SceneInfo::ClearData()
{
    nodesAtScene.clear();
    sceneTextures.clear();
    particleTextures.clear();
    
    lodInfoInFrame.Clear();
    
    ClearSelectionData();
    
    sceneTexturesSize = 0;
    particleTexturesSize = 0;
    emittersCount = 0;
    spritesCount = 0;
    
    speedTreeLeafInfo.clear();
}

void SceneInfo::ClearSelectionData()
{
    lodInfoSelection.Clear();
    selectedRenderObjects.clear();
}

void SceneInfo::CollectParticlesData()
{
    Set<Sprite *>sprites;

    emittersCount = 0;
    for(uint32 n = 0; n < nodesAtScene.size(); ++n)
    {
        ParticleEffectComponent *effect = GetEffectComponent(nodesAtScene[n]);
        if(!effect) continue;
        
		for (int32 i=0, sz=effect->GetEmittersCount(); i<sz; ++i)
		{
			++emittersCount;
			Vector<ParticleLayer*> &layers = effect->GetEmitter(i)->layers;
			for(uint32 lay = 0; lay < layers.size(); ++lay)
			{
				Sprite *spr = layers[lay]->sprite;
				if(spr)
				{
					sprites.insert(spr);

					for(int32 fr = 0; fr < spr->GetFrameCount(); ++fr)
					{
						Texture *tex = spr->GetTexture(fr);
						CollectTexture(particleTextures, tex->GetPathname(), tex);
					}
				}
			}
		}
    }
    
    spritesCount = (uint32)sprites.size();
}

void SceneInfo::CollectLODDataInFrame()
{
    lodInfoInFrame.Clear();
    lodInfoSelection.Clear();

    if(!activeScene||!activeScene->renderSystem||!activeScene->renderSystem->IsRenderHierarchyInitialized()||!activeScene->GetCurrentCamera())
        return;

    visibilityArray.clear();
    activeScene->renderSystem->GetRenderHierarchy()->Clip(activeScene->GetCurrentCamera(), visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA);

    uint32 size = (uint32)visibilityArray.size();
    for (uint32 ro = 0; ro < size; ++ro)
    {
        RenderObject * renderObject = visibilityArray[ro];
        uint32 batchCount = renderObject->GetActiveRenderBatchCount();
        int32 indexCount = 0;
        for(uint32 i = 0; i < batchCount; ++i)
        {            
            RenderBatch *rb = renderObject->GetActiveRenderBatch(i);
            if(IsPointerToExactClass<RenderBatch>(rb))
            {                
                PolygonGroup *pg = rb->GetPolygonGroup();
                if(pg)                
                    indexCount += pg->GetIndexCount();                
            }
        }
        int32 currLodIndex = renderObject->GetLodIndex();
        int32 polygonCount = indexCount/3;
        lodInfoInFrame.AddTriangles(currLodIndex, polygonCount);        

        if (selectedRenderObjects.find(renderObject)!=selectedRenderObjects.end())
        {
            lodInfoSelection.AddTriangles(currLodIndex, polygonCount);                    
        }
    }    
}

void SceneInfo::CollectLODDataInScene()
{
    lodInfoInFrame.Clear();
    if(!activeScene)
        return;

    CollectLODDataInEntityRecursive(activeScene);
    lodInfoInFrame.trianglesOnObjects += GetTrianglesForNotLODEntityRecursive(activeScene, true);
}

void SceneInfo::CollectLODDataInEntityRecursive(DAVA::Entity *entity)
{
    DAVA::LodComponent *lod = GetLodComponent(entity);
    
    if(lod)
    {
        EditorLODSystem::AddTrianglesInfo(lodInfoInFrame.trianglesOnLod, lod, true);
    }
    
    DAVA::int32 count = entity->GetChildrenCount();
    for(DAVA::int32 i = 0; i < count; ++i)
    {
        CollectLODDataInEntityRecursive(entity->GetChild(i));
    }
}



void SceneInfo::CollectLODTriangles(const DAVA::Vector<DAVA::LodComponent *> &lods, LODInfo &info)
{
    uint32 count = (uint32)lods.size();
    for(uint32 i = 0; i < count; ++i)
    {
        EditorLODSystem::AddTrianglesInfo(info.trianglesOnLod, lods[i], false);
    }
}

DAVA::uint32 SceneInfo::GetTrianglesForNotLODEntityRecursive(DAVA::Entity *entity, bool onlyVisibleBatches)
{
    if(GetLodComponent(entity))
        return 0;
    
    DAVA::uint32 triangles = 0;
    
    RenderObject * ro = GetRenderObject(entity);
    if(ro && ro->GetType() != RenderObject::TYPE_PARTICLE_EMTITTER)
    {
        uint32 batchCount = (onlyVisibleBatches) ? ro->GetActiveRenderBatchCount() : ro->GetRenderBatchCount();
        for(uint32 i = 0; i < batchCount; ++i)
        {
            RenderBatch *rb = (onlyVisibleBatches) ? ro->GetActiveRenderBatch(i) : ro->GetRenderBatch(i);
            PolygonGroup *pg = rb->GetPolygonGroup();
            if(pg)
            {
                triangles += (pg->GetIndexCount() / 3);
            }
        }
    }

    DAVA::uint32 count = entity->GetChildrenCount();
    for(DAVA::uint32 i = 0; i < count; ++i)
    {
        triangles += GetTrianglesForNotLODEntityRecursive(entity->GetChild(i), onlyVisibleBatches);
    }
    
    return triangles;
}

void SceneInfo::CollectTexture(TexturesMap &textures, const FilePath &name, Texture *tex)
{
    if(!name.IsEmpty() && tex)
	{
		textures[FILEPATH_MAP_KEY(name)] = tex;
	}
}


QtPropertyData * SceneInfo::CreateInfoHeader(const QString &key)
{
    QtPropertyData* headerData = new QtPropertyData("");
    headerData->SetEditable(false);
	headerData->SetBackground(QBrush(QColor(Qt::lightGray)));
    AppendProperty(key, headerData);
	return headerData;
}

QtPropertyData * SceneInfo::GetInfoHeader(const QString &key)
{
	QtPropertyData *header = NULL;
    QtPropertyData *root = GetRootProperty();
	if(NULL != root)
	{
		header = root->ChildGet(key);
	}
	return header;
}

void SceneInfo::AddChild(const QString & key, QtPropertyData *parent)
{
    QtPropertyData *propData = new QtPropertyData(0);
	propData->SetEditable(false);
    parent->ChildAdd(key, propData);
}

void SceneInfo::AddChild(const QString & key, const QString& toolTip, QtPropertyData *parent)
{
    QtPropertyData *propData = new QtPropertyData(0);
	propData->SetEditable(false);
    propData->SetToolTip(toolTip);
    parent->ChildAdd(key, propData);
}

void SceneInfo::SetChild(const QString & key, const QVariant &value, QtPropertyData *parent)
{
	if(NULL != parent)
	{
		QtPropertyData *propData = parent->ChildGet(key);
		if(NULL != propData)
		{
			propData->SetValue(value);
		}
	}
}

bool SceneInfo::HasChild(const QString & key, QtPropertyData *parent)
{
    bool hasChild = false;
    if(NULL != parent)
	{
		QtPropertyData *propData = parent->ChildGet(key);
		hasChild = (propData != NULL);
	}
    
    return hasChild;
}

void SceneInfo::SaveTreeState()
{
    // Store the current Property Editor Tree state before switching to the new node.
	// Do not clear the current states map - we are using one storage to share opened
	// Property Editor nodes between the different Scene Nodes.
	treeStateHelper.SaveTreeViewState(false);
}

void SceneInfo::RestoreTreeState()
{
    // Restore back the tree view state from the shared storage.
	if (!treeStateHelper.IsTreeStateStorageEmpty())
	{
		treeStateHelper.RestoreTreeViewState();
	}
	else
	{
		// Expand the root elements as default value.
		expandToDepth(0);
	}
}

void SceneInfo::showEvent ( QShowEvent * event )
{
	if(!isUpToDate)
	{
		isUpToDate = true;
		RefreshAllData(activeScene);
	}

    QtPropertyEditor::showEvent(event);
}

void SceneInfo::UpdateInfoByTimer()
{
    if(!isVisible()) return;

    Refresh3DDrawInfo();
    
    CollectLODDataInFrame();
    RefreshLODInfoInFrame();
    RefreshLODInfoForSelection();
    
    RefreshVegetationInfoSection();
    
    RefreshLayersSection();
}

void SceneInfo::RefreshAllData(SceneEditor2 *scene)
{
	CollectSceneData(scene);

	SaveTreeState();

	RefreshSceneGeneralInfo();
	Refresh3DDrawInfo();
	RefreshLODInfoInFrame();
    RefreshLODInfoForSelection();
    RefreshSpeedTreeInfoSelection();
    
    RefreshVegetationInfoSection();
    
    RefreshLayersSection();

	RestoreTreeState();
}

void SceneInfo::SceneActivated(SceneEditor2 *scene)
{
    activeScene = scene;
    landscape = FindLandscape(activeScene);
	
	isUpToDate = isVisible();
	if(isUpToDate)
	{
		RefreshAllData(scene);
	}
}

void SceneInfo::SceneDeactivated(SceneEditor2 *scene)
{
    if(activeScene == scene)
    {
        activeScene = NULL;
        landscape = NULL;
        RefreshAllData(NULL);
    }
}

void SceneInfo::SceneStructureChanged(SceneEditor2 *scene, DAVA::Entity *parent)
{
    if(activeScene == scene)
    {
        landscape = FindLandscape(activeScene);

		isUpToDate = isVisible();
		if(isUpToDate)
		{
			RefreshAllData(scene);
		}
    }
}


void SceneInfo::SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    ClearSelectionData();
        
    CollectSelectedRenderObjects(selected);

    CollectLODDataInFrame();
    RefreshLODInfoForSelection();

    CollectSpeedTreeLeafsSquare(selected);
    RefreshSpeedTreeInfoSelection();
}


void SceneInfo::CollectSelectedRenderObjects(const EntityGroup *selected)
{
    for (int32 i = 0, sz = selected->Size(); i<sz; ++i)
    {
        CollectSelectedRenderObjectsRecursivly(selected->GetEntity(i));
    }
}
void SceneInfo::CollectSelectedRenderObjectsRecursivly(Entity * entity)
{
    RenderObject *renderObject = GetRenderObject(entity);
    if (renderObject)    
        selectedRenderObjects.insert(renderObject);    

    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        CollectSelectedRenderObjectsRecursivly(entity->GetChild(i));
}

void SceneInfo::CollectSpeedTreeLeafsSquare(const EntityGroup * forGroup)
{
    speedTreeLeafInfo.clear();

    int32 entitiesCount = forGroup->Size();
    for(int32 i = 0; i < entitiesCount; i++)
    {
        RenderObject * ro = GetRenderObject(forGroup->GetEntity(i));
        if(ro && ro->GetType() == RenderObject::TYPE_SPEED_TREE)
            speedTreeLeafInfo.push_back(GetSpeedTreeLeafsSquare(ro));
    }
}

SceneInfo::SpeedTreeInfo SceneInfo::GetSpeedTreeLeafsSquare(DAVA::RenderObject *renderObject)
{
    SpeedTreeInfo info;
#if RHI_COMPLETE_EDITOR
    if(renderObject)
    {
        bool hasLeafsGeometry = false;

        Vector3 bboxSize = renderObject->GetBoundingBox().GetSize();
        int32 rbCount = renderObject->GetRenderBatchCount();
        for(int32 i = 0; i < rbCount; ++i)
        {
            RenderBatch * rb = renderObject->GetRenderBatch(i);
            if(rb->GetMaterial() && rb->GetMaterial()->GetMaterialTemplate()->name == NMaterialName::SPEEDTREE_LEAF)
            {
                PolygonGroup * pg = rb->GetPolygonGroup();
                int32 triangleCount = pg->GetIndexCount() / 3;
                for(int32 t = 0; t < triangleCount; t++)
                {
                    int32 i1, i2, i3;
                    int32 baseVertexIndex = t * 3;
                    pg->GetIndex(baseVertexIndex, i1);
                    pg->GetIndex(baseVertexIndex + 1, i2);
                    pg->GetIndex(baseVertexIndex + 2, i3);
                    
                    Vector3 v1, v2, v3;
                    pg->GetCoord(i1, v1);
                    pg->GetCoord(i2, v2);
                    pg->GetCoord(i3, v3);
                    
                    v1.z = 0; v2.z = 0; v3.z = 0; //ortho projection in XY-plane
                    
                    v2 -= v1;
                    v3 -= v1;
                    Vector3 vec = v2.CrossProduct(v3);
                    float32 len = vec.Length() / 2;

                    info.leafsSquare += len;
                    hasLeafsGeometry = true;
                }
            }
        }

        if (hasLeafsGeometry)
        {
            info.leafsSquareDivX = info.leafsSquare / (bboxSize.x * bboxSize.z);
            info.leafsSquareDivY = info.leafsSquare / (bboxSize.y * bboxSize.z);
        }
    }
#endif // RHI_COMPLETE_EDITOR
    return info;
}

void SceneInfo::TexturesReloaded()
{
    sceneTextures.clear();
    SceneHelper::EnumerateSceneTextures(activeScene, sceneTextures, SceneHelper::TexturesEnumerateMode::EXCLUDE_NULL);
    sceneTexturesSize = CalculateTextureSize(sceneTextures);
    
    RefreshSceneGeneralInfo();
}

void SceneInfo::SpritesReloaded()
{
    particleTextures.clear();

    CollectParticlesData();
    particleTexturesSize = CalculateTextureSize(particleTextures);
    
    RefreshSceneGeneralInfo();
}

void SceneInfo::OnQualityChanged()
{
    RefreshAllData(activeScene);
}

void SceneInfo::InitializeVegetationInfoSection()
{
    QtPropertyData* header = CreateInfoHeader("Vegetation Info");
    
    AddChild("Poly count", "Visible triangles count", header);
    AddChild("Instance count", "Visible vegetation instance count", header);
    
    AddChild("Poly count in LOD #0", "Visible triangles count in LOD #0", header);
    AddChild("Poly count in LOD #1", "Visible triangles count in LOD #1", header);
    AddChild("Poly count in LOD #2", "Visible triangles count in LOD #2", header);
    
    AddChild("Instance count in LOD #0", "Visible vegetation instance count in LOD #0", header);
    AddChild("Instance count in LOD #1", "Visible vegetation instance count in LOD #1", header);
    AddChild("Instance count in LOD #2", "Visible vegetation instance count in LOD #2", header);
    
    AddChild("Poly count in layer #0", "Visible triangles count in layer #0", header);
    AddChild("Poly count in layer #1", "Visible triangles count in layer #1", header);
    AddChild("Poly count in layer #2", "Visible triangles count in layer #2", header);
    AddChild("Poly count in layer #3", "Visible triangles count in layer #3", header);
    
    AddChild("Instance count in layer #0", "Visible vegetation instance count in layer #0", header);
    AddChild("Instance count in layer #1", "Visible vegetation instance count in layer #1", header);
    AddChild("Instance count in layer #2", "Visible vegetation instance count in layer #2", header);
    AddChild("Instance count in layer #3", "Visible vegetation instance count in layer #3", header);
    
    AddChild("Poly count in LODs in layer #0", "Vegetation model info for layer_0 entity. LOD 0 / LOD 1 / LOD 2", header);
    AddChild("Poly count in LODs in layer #1", "Vegetation model info for layer_1 entity. LOD 0 / LOD 1 / LOD 2", header);
    AddChild("Poly count in LODs in layer #2", "Vegetation model info for layer_2 entity. LOD 0 / LOD 1 / LOD 2", header);
    AddChild("Poly count in LODs in layer #3", "Vegetation model info for layer_3 entity. LOD 0 / LOD 1 / LOD 2", header);
    
    AddChild("Quadtree leaf count", "Number of quad tree leafs covering visible vegetation surface", header);
    
    AddChild("Quadtree leaf count in LOD #0", "Number of quad tree leafs covering visible vegetation surface for LOD #0", header);
    AddChild("Quadtree leaf count in LOD #1", "Number of quad tree leafs covering visible vegetation surface for LOD #1", header);
    AddChild("Quadtree leaf count in LOD #2", "Number of quad tree leafs covering visible vegetation surface for LOD #2", header);
    
    AddChild("RenderBatch count", "Number of renderbatches used to render visible vegetation", header);
}

void SceneInfo::RefreshVegetationInfoSection()
{
    if(activeScene != NULL)
    {
        VegetationRenderObject* renderObj = FindVegetation(activeScene);
        
        if(renderObj != NULL)
        {
            QtPropertyData* header = GetInfoHeader("Vegetation Info");
            
            VegetationMetrics metrics;
            renderObj->CollectMetrics(metrics);
            
            if(metrics.isValid)
            {
                
                static const char* INSTANCE_PER_LOD_HEADER[] =
                {
                    "Instance count in LOD #0",
                    "Instance count in LOD #1",
                    "Instance count in LOD #2"
                };
                
                static const char* INSTANCE_PER_LAYER_HEADER[] =
                {
                    "Instance count in layer #0",
                    "Instance count in layer #1",
                    "Instance count in layer #2",
                    "Instance count in layer #3"
                };
                
                static const char* POLY_PER_LOD_HEADER[] =
                {
                    "Poly count in LOD #0",
                    "Poly count in LOD #1",
                    "Poly count in LOD #2"
                };
                
                static const char* POLY_PER_LAYER_HEADER[] =
                {
                    "Poly count in layer #0",
                    "Poly count in layer #1",
                    "Poly count in layer #2",
                    "Poly count in layer #3"
                };
                
                static const char* QUADTREELEAF_PER_LOD_HEADER[] =
                {
                    "Quadtree leaf count in LOD #0",
                    "Quadtree leaf count in LOD #1",
                    "Quadtree leaf count in LOD #2"
                };
                
                static const char* POLY_PER_LOD_PER_LAYER_HEADER[] =
                {
                    "Poly count in LODs in layer #0",
                    "Poly count in LODs in layer #1",
                    "Poly count in LODs in layer #2",
                    "Poly count in LODs in layer #3"
                };
                
                uint32 totalInstanceCount = 0;
                for(uint32 lodIndex = 0; lodIndex < COUNT_OF(INSTANCE_PER_LOD_HEADER); ++lodIndex)
                {
                    if(metrics.visibleInstanceCountPerLOD.size() > lodIndex)
                    {
                        totalInstanceCount += metrics.visibleInstanceCountPerLOD[lodIndex];
                    }
                    
                    SetChild(INSTANCE_PER_LOD_HEADER[lodIndex], metrics.visibleInstanceCountPerLOD[lodIndex], header);
                }
                SetChild("Instance count", totalInstanceCount, header);
                
                for(uint32 layerIndex = 0; layerIndex < COUNT_OF(INSTANCE_PER_LAYER_HEADER); ++layerIndex)
                {
                    if(metrics.visibleInstanceCountPerLayer.size() > layerIndex)
                    {
                        SetChild(INSTANCE_PER_LAYER_HEADER[layerIndex], metrics.visibleInstanceCountPerLayer[layerIndex], header);
                    }
                }
                
                uint32 totalPolyCount = 0;
                for(uint32 lodIndex = 0; lodIndex < COUNT_OF(POLY_PER_LOD_HEADER); ++lodIndex)
                {
                    if(metrics.visiblePolyCountPerLOD.size() > lodIndex)
                    {
                        totalPolyCount += metrics.visiblePolyCountPerLOD[lodIndex];
                    }
                    
                    SetChild(POLY_PER_LOD_HEADER[lodIndex], metrics.visiblePolyCountPerLOD[lodIndex], header);
                }
                SetChild("Poly count", totalPolyCount, header);
                
                for(uint32 layerIndex = 0; layerIndex < COUNT_OF(POLY_PER_LAYER_HEADER); ++layerIndex)
                {
                    if(metrics.visiblePolyCountPerLayer.size() > layerIndex)
                    {
                        SetChild(POLY_PER_LAYER_HEADER[layerIndex], metrics.visiblePolyCountPerLayer[layerIndex], header);
                    }
                }
                
                uint32 totalLeafCount = 0;
                for(uint32 lodIndex = 0; lodIndex < COUNT_OF(QUADTREELEAF_PER_LOD_HEADER); ++lodIndex)
                {
                    if(metrics.quadTreeLeafCountPerLOD.size() > lodIndex)
                    {
                        totalLeafCount += metrics.quadTreeLeafCountPerLOD[lodIndex];
                    }
                    
                    SetChild(QUADTREELEAF_PER_LOD_HEADER[lodIndex], metrics.quadTreeLeafCountPerLOD[lodIndex], header);
                }
                SetChild("Quadtree leaf count", totalLeafCount, header);
                
                SetChild("RenderBatch count", metrics.renderBatchCount, header);
                
                for(uint32 layerIndex = 0; layerIndex < COUNT_OF(POLY_PER_LOD_PER_LAYER_HEADER); ++layerIndex)
                {
                    if(metrics.polyCountPerLayerPerLod.size() > layerIndex)
                    {
                        String str = Format("%d / %d / %d", metrics.polyCountPerLayerPerLod[layerIndex][0], metrics.polyCountPerLayerPerLod[layerIndex][1], metrics.polyCountPerLayerPerLod[layerIndex][2]);
                        SetChild(POLY_PER_LOD_PER_LAYER_HEADER[layerIndex], str.c_str(), header);
                    }
                }
            }
        }
    }
}

void SceneInfo::InitializeLayersSection()
{
    CreateInfoHeader("Fragments Info");
}

void SceneInfo::RefreshLayersSection()
{
#if RHI_COMPLETE_EDITOR
    if(activeScene)
    {
        float32 viewportSize = RenderManager::Instance()->frameBufferWidth * RenderManager::Instance()->frameBufferHeight;
        
        QtPropertyData* header = GetInfoHeader("Fragments Info");
    
        Vector<FastName> queriesNames;
        FrameOcclusionQueryManager::Instance()->GetQueriesNames(queriesNames);
        int32 namesCount = queriesNames.size();
        for(int32 i = 0; i < namesCount; i++)
        {
            if(queriesNames[i] == FRAME_QUERY_UI_DRAW)
                continue;

            uint32 fragmentStats = FrameOcclusionQueryManager::Instance()->GetFrameStats(queriesNames[i]);
            String str = Format("%d / %.2f%%", fragmentStats, (fragmentStats * 100.0f) / viewportSize);

            if(!HasChild(queriesNames[i].c_str(), header))
                AddChild(queriesNames[i].c_str(), header);

            SetChild(queriesNames[i].c_str(), str.c_str(), header);
        }
    }
#endif RHI_COMPLETE_EDITOR
}