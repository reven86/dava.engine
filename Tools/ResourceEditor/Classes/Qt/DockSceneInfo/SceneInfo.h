#ifndef __SCENE_INFO_H__
#define __SCENE_INFO_H__

#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "DockProperties/PropertyEditorStateHelper.h"

#include <QShowEvent>

class SceneEditor2;
class SelectableGroup;
class EditorStatisticsSystem;
class Command2;

class SceneInfo : public QtPropertyEditor
{
    Q_OBJECT

protected:
    struct SpeedTreeInfo
    {
        SpeedTreeInfo()
            :
            leafsSquare(0.f)
            ,
            leafsSquareDivX(0.f)
            ,
            leafsSquareDivY(0.f)
        {
        }

        DAVA::float32 leafsSquare;
        DAVA::float32 leafsSquareDivX;
        DAVA::float32 leafsSquareDivY;
    };

public:
    SceneInfo(QWidget* parent = 0);
    ~SceneInfo() override;

public slots:
    void UpdateInfoByTimer();
    void TexturesReloaded();
    void SpritesReloaded();
    void OnQualityChanged();

protected slots:
    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);
    void SceneStructureChanged(SceneEditor2* scene, DAVA::Entity* parent);
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void OnCommmandExecuted(SceneEditor2* scene, const Command2* command, bool isRedo);
    void OnThemeChanged();

private:
    void showEvent(QShowEvent* event) override;

    EditorStatisticsSystem* GetCurrentEditorStatisticsSystem() const;

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

    void RefreshAllData(SceneEditor2* scene);

    void ClearData();
    void ClearSelectionData();

    void SaveTreeState();
    void RestoreTreeState();

    QtPropertyData* CreateInfoHeader(const QString& key);
    QtPropertyData* GetInfoHeader(const QString& key);

    void AddChild(const QString& key, QtPropertyData* parent);
    void AddChild(const QString& key, const QString& toolTip, QtPropertyData* parent);
    void SetChild(const QString& key, const QVariant& value, QtPropertyData* parent);
    bool HasChild(const QString& key, QtPropertyData* parent);

    void CollectSceneData(SceneEditor2* scene);
    void CollectParticlesData();
    void CollectSpeedTreeLeafsSquare(const SelectableGroup* forGroup);
    void CollectSelectedRenderObjects(const SelectableGroup* selected);
    void CollectSelectedRenderObjectsRecursivly(DAVA::Entity* entity);

    void CollectTexture(DAVA::TexturesMap& textures, const DAVA::FilePath& pathname, DAVA::Texture* tex);

    static DAVA::uint32 CalculateTextureSize(const DAVA::TexturesMap& textures);

    static DAVA::uint32 GetTrianglesForNotLODEntityRecursive(DAVA::Entity* entity, bool onlyVisibleBatches);

    static SpeedTreeInfo GetSpeedTreeLeafsSquare(DAVA::RenderObject* forEntity);

protected:
    QtPosSaver posSaver;
    PropertyEditorStateHelper treeStateHelper;

    SceneEditor2* activeScene = nullptr;
    DAVA::Vector<DAVA::Entity*> nodesAtScene;
    DAVA::Landscape* landscape = nullptr;

    DAVA::TexturesMap particleTextures;

    DAVA::Vector<DAVA::DataNode*> dataNodesAtScene;

    DAVA::Vector<SpeedTreeInfo> speedTreeLeafInfo;

    DAVA::uint32 sceneTexturesSize = 0;
    DAVA::uint32 particleTexturesSize = 0;

    DAVA::uint32 emittersCount = 0;
    DAVA::uint32 spritesCount = 0;

    DAVA::Vector<DAVA::RenderObject*> visibilityArray;
    DAVA::Set<DAVA::RenderObject*> selectedRenderObjects;

    bool isUpToDate = false;
};

#endif // __SCENE_INFO_H__
