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
#include "Deprecated/EditorSettings.h"

#include "CommandLine/CommandLineManager.h"


#include "Main/QtUtils.h"

#include "Render/TextureDescriptor.h"

#include "../../ImageTools/ImageTools.h"

#include "CubemapEditor/MaterialHelper.h"

#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneHelper.h"
#include "DockLODEditor/EditorLODData.h"
#include "Main/mainwindow.h"

#include <QHeaderView>
#include <QTimer>
#include <QPalette>

using namespace DAVA;

SceneInfo::SceneInfo(QWidget *parent /* = 0 */)
	: QtPropertyEditor(parent)
    , activeScene(NULL)
	, landscape(NULL)
    , treeStateHelper(this, curModel)
	, isUpToDate(false)
{
    SetEditTracking(true);
    
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

void SceneInfo::InitializeInfo()
{
    RemovePropertyAll();
    
    InitializeGeneralSection();
    Initialize3DDrawSection();
    InitializeLODSectionInFrame();
    InitializeLODSectionForSelection();
    InitializeParticlesSection();
}

void SceneInfo::InitializeGeneralSection()
{
    QtPropertyData* header = CreateInfoHeader("General Scene Info");
  
    AddChild("Entities Count", header);
    AddChild("All Textures Size", header);
	AddChild("Material Textures Size", header);
}

void SceneInfo::RefreshSceneGeneralInfo()
{
    QtPropertyData* header = GetInfoHeader("General Scene Info");

    SetChild("Entities Count", (uint32)nodesAtScene.size(), header);
    SetChild("All Textures Size", QString::fromStdString(SizeInBytesToString((float32)(sceneTexturesSize + particleTexturesSize))), header);
	SetChild("Material Textures Size", QString::fromStdString(SizeInBytesToString((float32)sceneTexturesSize)), header);
}

void SceneInfo::Initialize3DDrawSection()
{
    QtPropertyData* header = CreateInfoHeader("DrawInfo");

    AddChild("ArraysCalls", header);
    AddChild("ElementsCalls",  header);
    AddChild("PointsList", header);
    AddChild("LineList", header);
    AddChild("LineStrip", header);
    AddChild("TriangleList", header);
    AddChild("TriangleStrip", header);
    AddChild("TriangleFan", header);
}

void SceneInfo::Refresh3DDrawInfo()
{
    if(!activeScene) return;
    
    QtPropertyData* header = GetInfoHeader("DrawInfo");
    
    const RenderManager::Stats & renderStats = activeScene->GetRenderStats();
    
    SetChild("ArraysCalls", renderStats.drawArraysCalls, header);
    SetChild("ElementsCalls", renderStats.drawElementsCalls, header);
    SetChild("PointsList", renderStats.primitiveCount[PRIMITIVETYPE_POINTLIST], header);
    SetChild("LineList", renderStats.primitiveCount[PRIMITIVETYPE_LINELIST], header);
    SetChild("LineStrip", renderStats.primitiveCount[PRIMITIVETYPE_LINESTRIP], header);
    SetChild("TriangleList", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLELIST], header);
    SetChild("TriangleStrip", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLESTRIP], header);
    SetChild("TriangleFan", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLEFAN], header);
}


void SceneInfo::InitializeLODSectionInFrame()
{
    QtPropertyData* header = CreateInfoHeader("LOD in Frame");
    
    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        AddChild(Format("Objects LOD%d Triangles", i), header);
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
        AddChild(Format("Objects LOD%d Triangles", i), header);
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
        SetChild(Format("Objects LOD%d Triangles", i), lodInfoInFrame.trianglesOnLod[i], header);
        
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
        SetChild(Format("Objects LOD%d Triangles", i), lodInfoSelection.trianglesOnLod[i], header);
        
        lodTriangles += lodInfoSelection.trianglesOnLod[i];
    }
    
    SetChild("All LOD Triangles", lodTriangles, header);
    
    SetChild("Objects without LOD Triangles", lodInfoSelection.trianglesOnObjects, header);
    SetChild("All Triangles", lodInfoSelection.trianglesOnObjects + lodTriangles, header);
}


void SceneInfo::InitializeParticlesSection()
{
    QtPropertyData* header = CreateInfoHeader("Particles");

    AddChild("Emitters Count", header);
    AddChild("Sprites Count", header);
    AddChild("Textures Count", header);
    AddChild("Textures Size", header);
}

void SceneInfo::RefreshParticlesInfo()
{
    QtPropertyData* header = GetInfoHeader("Particles");

    SetChild("Emitters Count", emittersCount, header);
    SetChild("Sprites Count", spritesCount, header);
    SetChild("Textures Count", (uint32)particleTextures.size(), header);
    SetChild("Textures Size", QString::fromStdString(SizeInBytesToString((float32)particleTexturesSize)), header);
}

uint32 SceneInfo::CalculateTextureSize(const TexturesMap &textures)
{
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings();
    String projectPath = settings->GetString("ProjectPath");

    uint32 textureSize = 0;
    
    TexturesMap::const_iterator endIt = textures.end();
    for(TexturesMap::const_iterator it = textures.begin(); it != endIt; ++it)
    {
        FilePath pathname = it->first;
        Texture *tex = it->second;
        DVASSERT(tex);
        
        if(String::npos == pathname.GetAbsolutePathname().find(projectPath))
        {
            Logger::Warning("[SceneInfo::CalculateTextureSize] Path (%s) doesn't belong to project.", pathname.GetAbsolutePathname().c_str());
            continue;
        }
        
        TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(pathname);
        if(!descriptor)
        {
            Logger::Error("[SceneInfo::CalculateTextureSize] Can't create descriptor for texture %s", pathname.GetAbsolutePathname().c_str());
            continue;
        }
        
        textureSize += ImageTools::GetTexturePhysicalSize(descriptor, EditorSettings::Instance()->GetTextureViewGPU());
        
        SafeRelease(descriptor);
    }

    return textureSize;
}



void SceneInfo::CollectSceneData(SceneEditor2 *scene)
{
    ClearData();

    if(scene)
    {
        scene->GetChildNodes(nodesAtScene);
		//VI: remove skybox materials so they not to appear in the lists
		//MaterialHelper::FilterMaterialsByType(materialsAtScene, DAVA::Material::MATERIAL_SKYBOX);

        SceneHelper::EnumerateTextures(activeScene, sceneTextures);
        sceneTexturesSize = CalculateTextureSize(sceneTextures);

        CollectParticlesData();
        particleTexturesSize = CalculateTextureSize(particleTextures);
        
        CollectLODDataInFrame();
        CollectLODDataForSelection();
        
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
}

void SceneInfo::ClearSelectionData()
{
    lodInfoSelection.Clear();
}

void SceneInfo::CollectParticlesData()
{
    Set<Sprite *>sprites;

    emittersCount = 0;
    for(uint32 n = 0; n < nodesAtScene.size(); ++n)
    {
        ParticleEmitter *emitter = GetEmitter(nodesAtScene[n]);
        if(!emitter) continue;
        
        ++emittersCount;
        
        Vector<ParticleLayer*> &layers = emitter->GetLayers();
        
        for(uint32 lay = 0; lay < layers.size(); ++lay)
        {
            Sprite *spr = layers[lay]->GetSprite();
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
    
    spritesCount = (uint32)sprites.size();
}

void SceneInfo::CollectLODDataForSelection()
{
    if(!activeScene) return;
    
    Vector<LodComponent *>lods;
    for(size_t i = 0; i < activeScene->selectionSystem->GetSelectionCount(); ++i)
    {
        EditorLODData::EnumerateLODsRecursive(activeScene->selectionSystem->GetSelectionEntity(i), lods);
        lodInfoSelection.trianglesOnObjects += GetTrianglesForNotLODEntityRecursive(activeScene->selectionSystem->GetSelectionEntity(i), false);
    }
    
    CollectLODTriangles(lods, lodInfoSelection);
}

void SceneInfo::CollectLODDataInFrame()
{
    lodInfoInFrame.Clear();

    if(!activeScene) return;

    CollectLODDataInFrameRecursive(activeScene);
    lodInfoInFrame.trianglesOnObjects += GetTrianglesForNotLODEntityRecursive(activeScene, true);
}

void SceneInfo::CollectLODDataInFrameRecursive(DAVA::Entity *entity)
{
    DAVA::LodComponent *lod = GetLodComponent(entity);
    
    if(lod)
    {
        Vector<LodComponent::LodData*> lodLayers;
        lod->GetLodData(lodLayers);
        Vector<LodComponent::LodData*>::const_iterator lodLayerIt = lodLayers.begin();
        DAVA::uint32 layersCount = lod->GetForceLodLayer();
        for(DAVA::uint32 layer = 0; layer < layersCount && lodLayerIt != lodLayers.end(); ++layer, ++lodLayerIt)
        {
            lodInfoInFrame.trianglesOnLod[layer] += EditorLODData::GetTrianglesForLodLayer(*lodLayerIt, true);
        }
    }
    
    DAVA::int32 count = entity->GetChildrenCount();
    for(DAVA::int32 i = 0; i < count; ++i)
    {
        CollectLODDataInFrameRecursive(entity->GetChild(i));
    }
}



void SceneInfo::CollectLODTriangles(const DAVA::Vector<DAVA::LodComponent *> &lods, LODInfo &info)
{
    uint32 count = (uint32)lods.size();
    for(uint32 i = 0; i < count; ++i)
    {
        Vector<LodComponent::LodData*> lodLayers;
        lods[i]->GetLodData(lodLayers);

        int32 layersCount = lods[i]->GetLodLayersCount();
        Vector<LodComponent::LodData*>::const_iterator lodLayerIt = lodLayers.begin();
        for(int32 layer = 0; layer < layersCount; ++layer, ++lodLayerIt)
        {
            info.trianglesOnLod[layer] += EditorLODData::GetTrianglesForLodLayer(*lodLayerIt, false);
        }
    }
}

DAVA::uint32 SceneInfo::GetTrianglesForNotLODEntityRecursive(DAVA::Entity *entity, bool checkVisibility)
{
    if(GetLodComponent(entity))
        return 0;
    
    DAVA::uint32 triangles = EditorLODData::GetTrianglesForEntity(entity, checkVisibility);
    
    DAVA::uint32 count = entity->GetChildrenCount();
    for(DAVA::uint32 i = 0; i < count; ++i)
    {
        triangles += GetTrianglesForNotLODEntityRecursive(entity->GetChild(i), checkVisibility);
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
}

void SceneInfo::RefreshAllData(SceneEditor2 *scene)
{
	CollectSceneData(scene);

	SaveTreeState();

	RefreshSceneGeneralInfo();
	Refresh3DDrawInfo();
	RefreshLODInfoInFrame();
    RefreshLODInfoForSelection();
	RefreshParticlesInfo();

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

		isUpToDate = !isVisible();
		if(isUpToDate)
		{
			RefreshAllData(scene);
		}
    }
}


void SceneInfo::SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    ClearSelectionData();
    CollectLODDataForSelection();
    RefreshLODInfoForSelection();
}

void SceneInfo::TexturesReloaded()
{
    sceneTextures.clear();
    SceneHelper::EnumerateTextures(activeScene, sceneTextures);
    sceneTexturesSize = CalculateTextureSize(sceneTextures);
    
    RefreshSceneGeneralInfo();
}

void SceneInfo::SpritesReloaded()
{
    particleTextures.clear();

    CollectParticlesData();
    particleTexturesSize = CalculateTextureSize(particleTextures);
    
    RefreshSceneGeneralInfo();
    RefreshParticlesInfo();
}
