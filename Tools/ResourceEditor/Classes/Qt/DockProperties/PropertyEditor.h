#pragma once

#include "PropertyEditorStateHelper.h"
#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Scene/SceneSignals.h"
#include "Scene/ActiveSceneHolder.h"

namespace Ui
{
class MainWindow;
}

namespace DAVA
{
namespace TArc
{
class FieldBinder;
}
}

class GlobalOperations;
class LazyUpdater;
class RECommandNotificationObject;
struct PropEditorUserData : public QtPropertyData::UserData
{
    enum PropertyType : DAVA::uint32
    {
        ORIGINAL,
        COPY
    };

    PropEditorUserData(PropertyType _type, QtPropertyData* _associatedData = NULL, bool _isFavorite = false, DAVA::Entity* _entity = NULL)
        : associatedData(_associatedData)
        , entity(_entity)
        , type(_type)
        , isFavorite(_isFavorite)
    {
    }

    QString realPath;
    QtPropertyData* associatedData;
    DAVA::Entity* entity;
    PropertyType type;
    bool isFavorite;
};

class PropertyEditor : public QtPropertyEditor
{
    Q_OBJECT

public:
    enum eViewMode : DAVA::uint32
    {
        VIEW_NORMAL,
        VIEW_ADVANCED,
        VIEW_FAVORITES_ONLY
    };

    PropertyEditor(QWidget* parent = 0, bool connectToSceneSignals = true);
    ~PropertyEditor();

    void Init(Ui::MainWindow* mainWindowUi, const std::shared_ptr<GlobalOperations>& globalOperations);

    virtual void SetEntities(const SelectableGroup* selected);

    void SetViewMode(eViewMode mode);
    eViewMode GetViewMode() const;

    void SetFavoritesEditMode(bool set);
    bool GetFavoritesEditMode() const;

    bool IsFavorite(QtPropertyData* data) const;
    void SetFavorite(QtPropertyData* data, bool favorite);

    void LoadScheme(const DAVA::FilePath& path);
    void SaveScheme(const DAVA::FilePath& path);

public slots:
    void sceneActivated(SceneEditor2* scene);
    void sceneDeactivated(SceneEditor2* scene);
    void CommandExecuted(SceneEditor2* scene, const RECommandNotificationObject& commandNotification);

    void ActionEditComponent();
    void ActionEditMaterial();
    void ActionEditSoundComponent();
    void OnAddActionComponent();
    void OnAddLodComponent();
    void OnAddStaticOcclusionComponent();
    void OnAddSoundComponent();
    void OnAddWaveComponent();
    void OnAddModelTypeComponent();
    void OnAddSkeletonComponent();
    void OnAddPathComponent();
    void OnAddRotationControllerComponent();
    void OnAddSnapToLandscapeControllerComponent();
    void OnAddWASDControllerComponent();
    void OnAddVisibilityComponent();
    void OnRemoveComponent();
    void OnTriggerWaveComponent();
    void OnConvertRenderObjectToBillboard();

    void ConvertToShadow();

    void DeleteRenderBatch();

    void RebuildTangentSpace();

    void CloneRenderBatchesToFixSwitchLODs();

    void ResetProperties();

private:
    QtPropertyData* CreateInsp(const DAVA::FastName& name, void* object, const DAVA::InspInfo* info);
    QtPropertyData* CreateInspMember(const DAVA::FastName& name, void* object, const DAVA::InspMember* member);
    QtPropertyData* CreateInspCollection(const DAVA::FastName& name, void* object, const DAVA::InspColl* collection);
    QtPropertyData* CreateClone(QtPropertyData* original);

    void ClearCurrentNodes();
    void ApplyModeFilter(QtPropertyData* parent);
    void ApplyFavorite(QtPropertyData* data);
    void ApplyCustomExtensions(QtPropertyData* data);

    void OnAddComponent(DAVA::Component::eType type);
    void OnAddComponent(DAVA::Component* component);

    void AddFavoriteChilds(QtPropertyData* parent);
    void RemFavoriteChilds(QtPropertyData* parent);

    bool IsInspViewAllowed(const DAVA::InspInfo* info) const;

    void OnItemEdited(const QModelIndex& index) override;
    void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    bool IsParentFavorite(const QtPropertyData* data) const;
    PropEditorUserData* GetUserData(QtPropertyData* data) const;

    QtPropertyToolButton* CreateButton(QtPropertyData* data, const QIcon& icon, const QString& tooltip);

    QString GetDefaultFilePath(bool withScenePath = true);

    void AddEntityProperties(DAVA::Entity* node, std::unique_ptr<QtPropertyData>& root,
                             std::unique_ptr<QtPropertyData>& curEntityData, bool isFirstInList);

    void UpdateSelectionLazy();
    SelectableGroup ExtractEntities(const SelectableGroup* selection);

private:
    LazyUpdater* propertiesUpdater = nullptr;
    LazyUpdater* selectionUpdater = nullptr;
    QtPosSaver posSaver;
    QSet<QString> scheme;
    QtPropertyData* favoriteGroup;
    SelectableGroup curNodes;
    PropertyEditorStateHelper treeStateHelper;
    DAVA::Vector<std::unique_ptr<QtPropertyData>> favoriteList;
    eViewMode viewMode = VIEW_NORMAL;
    bool favoritesEditMode;
    ActiveSceneHolder sceneHolder;
    std::shared_ptr<GlobalOperations> globalOperations;

    std::unique_ptr<DAVA::TArc::FieldBinder> selectionFieldBinder;
};
