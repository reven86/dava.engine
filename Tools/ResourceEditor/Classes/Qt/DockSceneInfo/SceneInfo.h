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


#ifndef __SCENE_INFO_H__
#define __SCENE_INFO_H__

#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "DockProperties/PropertyEditorStateHelper.h"

#include <QShowEvent>

class SceneEditor2;
class EntityGroup;

class SceneInfo : public QtPropertyEditor
{
	Q_OBJECT

protected:
    
    struct LODInfo
    {
		std::array<DAVA::uint32, DAVA::LodComponent::MAX_LOD_LAYERS> trianglesOnLod;
        DAVA::uint32 trianglesOnObjects;
        
        void Clear()
        {
            for(DAVA::int32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
            {
                trianglesOnLod[i] = 0;
            }
            trianglesOnObjects = 0;
        }

        void AddTriangles(DAVA::int32 index, DAVA::int32 count);
        
    };
    
    struct SpeedTreeInfo
    {
        SpeedTreeInfo() :
        leafsSquare(0.f),
        leafsSquareDivX(0.f),
        leafsSquareDivY(0.f)
        {}
        
        DAVA::float32 leafsSquare;
        DAVA::float32 leafsSquareDivX;
        DAVA::float32 leafsSquareDivY;
    };

public:
	SceneInfo(QWidget *parent = 0);
	~SceneInfo();

public slots:
    void UpdateInfoByTimer();
    void TexturesReloaded();
    void SpritesReloaded();
    void OnQualityChanged();
    

protected slots:
    void SceneActivated(SceneEditor2 *scene);
	void SceneDeactivated(SceneEditor2 *scene);
    void SceneStructureChanged(SceneEditor2 *scene, DAVA::Entity *parent);
	void SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected);

protected:
    
    virtual void showEvent ( QShowEvent * event );
    
protected:

    void InitializeInfo();
    void InitializeGeneralSection();
    void Initialize3DDrawSection();
    void InitializeLODSectionInFrame();
    void InitializeLODSectionForSelection();

    void InitializeSpeedTreeInfoSelection();

    void InitializeVegetationInfoSection();
    
    void InitializeLayersSection();

    void RefreshSceneGeneralInfo();
    void Refresh3DDrawInfo();
    void RefreshLODInfoInFrame();
    void RefreshLODInfoForSelection();

    void RefreshSpeedTreeInfoSelection();
    
    void RefreshVegetationInfoSection();
    
    void RefreshLayersSection();
    
	void RefreshAllData(SceneEditor2 *scene);

    void ClearData();
    void ClearSelectionData();
    
    void SaveTreeState();
    void RestoreTreeState();
    
    
    QtPropertyData * CreateInfoHeader(const QString &key);
    QtPropertyData * GetInfoHeader(const QString &key);
    
    void AddChild(const QString & key, QtPropertyData *parent);
    void AddChild(const QString & key, const QString& toolTip, QtPropertyData *parent);
    void SetChild(const QString & key, const QVariant &value, QtPropertyData *parent);
    bool HasChild(const QString & key, QtPropertyData *parent);
    
    void CollectSceneData(SceneEditor2 *scene);
    void CollectParticlesData();
    void CollectLODDataInFrame();
    void CollectLODDataInScene();
    void CollectLODDataInEntityRecursive(DAVA::Entity *entity);    
    void CollectSpeedTreeLeafsSquare(const EntityGroup * forGroup);
    void CollectSelectedRenderObjects(const EntityGroup *selected);
    void CollectSelectedRenderObjectsRecursivly(DAVA::Entity * entity);
    static void CollectLODTriangles(const DAVA::Vector<DAVA::LodComponent *> &lods, LODInfo &info);
    
    void CollectTexture(DAVA::TexturesMap &textures, const DAVA::FilePath &pathname, DAVA::Texture *tex);
    
    static DAVA::uint32 CalculateTextureSize(const DAVA::TexturesMap &textures);

    static DAVA::uint32 GetTrianglesForNotLODEntityRecursive(DAVA::Entity *entity, bool onlyVisibleBatches);
    
    static SpeedTreeInfo GetSpeedTreeLeafsSquare(DAVA::RenderObject *forEntity);
    
protected:
    
	QtPosSaver posSaver;
    PropertyEditorStateHelper treeStateHelper;
    
    SceneEditor2 * activeScene;
    DAVA::Vector<DAVA::Entity *> nodesAtScene;
    DAVA::Landscape *landscape;
    
	DAVA::TexturesMap sceneTextures;
	DAVA::TexturesMap particleTextures;

    DAVA::Vector<DAVA::DataNode *>dataNodesAtScene;
    
    DAVA::Vector<SpeedTreeInfo> speedTreeLeafInfo;
    
    DAVA::uint32 sceneTexturesSize;
    DAVA::uint32 particleTexturesSize;
    
    DAVA::uint32 emittersCount;
    DAVA::uint32 spritesCount;
    
    LODInfo lodInfoSelection;
    LODInfo lodInfoInFrame;

    DAVA::Vector<DAVA::RenderObject*> visibilityArray;
    DAVA::Set<DAVA::RenderObject *> selectedRenderObjects;

	bool isUpToDate;
};

#endif // __SCENE_INFO_H__
