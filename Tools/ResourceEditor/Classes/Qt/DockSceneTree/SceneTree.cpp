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

#include "DockSceneTree/SceneTree.h"
#include "Main/mainwindow.h"
#include "StringConstants.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QMenu>
#include <QDebug>

#include "Qt/Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Main/QTUtils.h"
#include "Project/ProjectManager.h"
#include "Scene/SceneEditor2.h"
#include "Scene/System/SelectionSystem.h"

#include "QtTools/FileDialog/FileDialog.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

// commands
#include "Commands2/ParticleEditorCommands.h"
#include "Commands2/SaveEntityAsAction.h"
#include "Commands2/ConvertToShadowCommand.h"
#include "QtTools/ConsoleWidget/PointerSerializer.h"
#include "FileSystem/VariantType.h"

#include "QtTools/LazyUpdater/LazyUpdater.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

namespace SceneTreeDetails
{
class SyncGuard
{
public:
    SyncGuard(bool& isSync_)
        : isSync(isSync_)
    {
        isSync = true;
    }

    ~SyncGuard()
    {
        isSync = false;
    }

private:
    bool& isSync;
};
} // namespace SceneTreeDetails

SceneTree::SceneTree(QWidget* parent /*= 0*/)
    : QTreeView(parent)
    , isInSync(false)
{
    Function<void()> fn(this, &SceneTree::UpdateTree);
    treeUpdater = new LazyUpdater(fn, this);

    CleanupParticleEditorSelectedItems();

    treeModel = new SceneTreeModel();
    filteringProxyModel = new SceneTreeFilteringModel(treeModel);

    setModel(filteringProxyModel);

    treeDelegate = new SceneTreeDelegate();
    setItemDelegate(treeDelegate);

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    // scene signals
    QObject::connect(SceneSignals::Instance(), &SceneSignals::Activated, this, &SceneTree::SceneActivated);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::Deactivated, this, &SceneTree::SceneDeactivated);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::StructureChanged, this, &SceneTree::SceneStructureChanged);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::SelectionChanged, this, &SceneTree::SceneSelectionChanged);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::CommandExecuted, this, &SceneTree::CommandExecuted);

    // particles signals
    QObject::connect(SceneSignals::Instance(), &SceneSignals::ParticleLayerValueChanged, this, &SceneTree::ParticleLayerValueChanged);

    // this widget signals
    QObject::connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &SceneTree::TreeSelectionChanged);
    QObject::connect(this, &QTreeView::clicked, this, &SceneTree::TreeItemClicked);
    QObject::connect(this, &QTreeView::doubleClicked, this, &SceneTree::TreeItemDoubleClicked);
    QObject::connect(this, &QTreeView::collapsed, this, &SceneTree::TreeItemCollapsed);
    QObject::connect(this, &QTreeView::expanded, this, &SceneTree::TreeItemExpanded);

    QObject::connect(this, &QTreeView::customContextMenuRequested, this, &SceneTree::ShowContextMenu);
}

void SceneTree::SetFilter(const QString& filter)
{
    treeModel->SetFilter(filter);
    filteringProxyModel->invalidate();
    SyncSelectionToTree();

    if (!filter.isEmpty())
    {
        ExpandFilteredItems();
    }
}

void SceneTree::GetDropParams(const QPoint& pos, QModelIndex& index, int& row, int& col)
{
    row = -1;
    col = -1;
    index = indexAt(pos);

    switch (dropIndicatorPosition())
    {
    case QAbstractItemView::AboveItem:
        row = index.row();
        col = index.column();
        index = index.parent();
        break;
    case QAbstractItemView::BelowItem:
        row = index.row() + 1;
        col = index.column();
        index = index.parent();
        break;
    case QAbstractItemView::OnItem:
    case QAbstractItemView::OnViewport:
        break;
    }
}

void SceneTree::dropEvent(QDropEvent* event)
{
    QTreeView::dropEvent(event);

    if (treeModel->DropAccepted())
    {
        int row, col;
        QModelIndex parent;

        GetDropParams(event->pos(), parent, row, col);
        expand(parent);
    }

    // after processing don't allow this event to go higher
    // so no body will decide to remove/insert drag&dropped items into treeview
    // except our model. Model will do this when scene entity remove/move signals catched
    event->setDropAction(Qt::IgnoreAction);
    event->accept();
}

void SceneTree::dragMoveEvent(QDragMoveEvent* event)
{
    QTreeView::dragMoveEvent(event);

    if (SettingsManager::GetValue(Settings::Scene_DragAndDropWithShift).AsBool() && ((event->keyboardModifiers() & Qt::SHIFT) != Qt::SHIFT))
    {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
        return;
    }

    {
        int row, col;
        QModelIndex parent;

        GetDropParams(event->pos(), parent, row, col);
        if (treeModel->DropCanBeAccepted(event->mimeData(), event->dropAction(), row, col, filteringProxyModel->mapToSource(parent)))
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->setDropAction(Qt::IgnoreAction);
            event->accept();
        }
    }
}

void SceneTree::dragEnterEvent(QDragEnterEvent* event)
{
    QTreeView::dragEnterEvent(event);

    {
        int row, col;
        QModelIndex parent;

        GetDropParams(event->pos(), parent, row, col);
        if (treeModel->DropCanBeAccepted(event->mimeData(), event->dropAction(), row, col, filteringProxyModel->mapToSource(parent)))
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->setDropAction(Qt::IgnoreAction);
            event->accept();
        }
    }
}

void SceneTree::SceneActivated(SceneEditor2* scene)
{
    treeModel->SetScene(scene);
    selectionModel()->clear();
    SyncSelectionToTree();
    filteringProxyModel->invalidate();

    PropagateSolidFlag();
}

void SceneTree::SceneDeactivated(SceneEditor2* scene)
{
    if (treeModel->GetScene() == scene)
    {
        selectionModel()->clear();
        treeModel->SetScene(NULL);
    }
}

void SceneTree::SceneSelectionChanged(SceneEditor2* scene, const SelectableObjectGroup* selected, const SelectableObjectGroup* deselected)
{
    if (scene == treeModel->GetScene())
    {
        SyncSelectionToTree();
    }
}

void SceneTree::SceneStructureChanged(SceneEditor2* scene, DAVA::Entity* parent)
{
    if (scene == treeModel->GetScene())
    {
        bool selectionWasBlocked = selectionModel()->blockSignals(true);
        treeModel->ResyncStructure(treeModel->invisibleRootItem(), treeModel->GetScene());
        treeModel->ReloadFilter();
        filteringProxyModel->invalidate();

        SyncSelectionToTree();
        EmitParticleSignals(QItemSelection());

        if (treeModel->IsFilterSet())
        {
            ExpandFilteredItems();
        }

        selectionModel()->blockSignals(selectionWasBlocked);
    }
}

void SceneTree::CommandExecuted(SceneEditor2* scene, const Command2* command, bool redo)
{
    auto commandID = command->GetId();

    switch (commandID)
    {
    case CMDID_COMPONENT_ADD:
    case CMDID_COMPONENT_REMOVE:
    case CMDID_INSP_MEMBER_MODIFY:
    case CMDID_INSP_DYNAMIC_MODIFY:
    case CMDID_ENTITY_LOCK:
    case CMDID_PARTICLE_EMITTER_ADD:
    case CMDID_PARTICLE_EMITTER_MOVE:
    case CMDID_PARTICLE_EMITTER_REMOVE:
    case CMDID_PARTICLE_LAYER_REMOVE:
    case CMDID_PARTICLE_LAYER_MOVE:
    case CMDID_PARTICLE_FORCE_REMOVE:
    case CMDID_PARTICLE_FORCE_MOVE:
    case CMDID_META_OBJ_MODIFY:
    case CMDID_PARTICLE_EMITTER_LAYER_ADD:
    case CMDID_PARTICLE_EMITTER_LAYER_REMOVE:
    case CMDID_PARTICLE_EMITTER_LAYER_CLONE:
    case CMDID_PARTICLE_EMITTER_FORCE_ADD:
    case CMDID_PARTICLE_EMITTER_FORCE_REMOVE:
    {
        treeUpdater->Update();
    }
    break;

    default:
        break;
    }
}

void SceneTree::TreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (isInSync)
        return;

    SyncSelectionFromTree();

    // emit some signal about particles
    EmitParticleSignals(selected);
}

void SceneTree::TreeItemClicked(const QModelIndex& index)
{
}

void SceneTree::ParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer)
{
    QModelIndexList indexList = selectionModel()->selection().indexes();
    if (indexList.empty())
    {
        return;
    }

    QModelIndex realIndex = filteringProxyModel->mapToSource(indexList[0]);
    SceneTreeItem* item = treeModel->GetItem(realIndex);
    if (item->ItemType() != SceneTreeItem::EIT_Layer)
    {
        return;
    }

    ParticleLayer* selectedLayer = SceneTreeItemParticleLayer::GetLayer(item);
    if (selectedLayer != layer)
    {
        return;
    }

    // Update the "isEnabled" flag, if it is changed.
    bool sceneTreeItemChecked = item->checkState() == Qt::Checked;
    if (layer->isDisabled == sceneTreeItemChecked)
    {
        blockSignals(true);
        item->setCheckState(sceneTreeItemChecked ? Qt::Unchecked : Qt::Checked);
        blockSignals(false);
    }

    //check if we need to resync tree for superemmiter
    SceneTreeItemParticleLayer* itemLayer = (SceneTreeItemParticleLayer*)item;
    bool needEmmiter = selectedLayer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES;
    if (itemLayer->hasInnerEmmiter != needEmmiter)
    {
        itemLayer->hasInnerEmmiter = needEmmiter;
        treeModel->ResyncStructure(treeModel->invisibleRootItem(), treeModel->GetScene());
    }
}

void SceneTree::TreeItemDoubleClicked(const QModelIndex& index)
{
    LookAtSelection();
}

void SceneTree::ShowContextMenu(const QPoint& pos)
{
    CleanupParticleEditorSelectedItems();
    QModelIndex index = filteringProxyModel->mapToSource(indexAt(pos));
    SceneTreeItem* item = treeModel->GetItem(index);

    if (NULL != item)
    {
        switch (item->ItemType())
        {
        case SceneTreeItem::EIT_Entity:
            ShowContextMenuEntity(SceneTreeItemEntity::GetEntity(item), treeModel->GetCustomFlags(index), mapToGlobal(pos));
            break;

        case SceneTreeItem::EIT_Emitter:
            ShowContextMenuEmitter(((SceneTreeItemParticleEmitter*)item)->effect, ((SceneTreeItemParticleEmitter*)item)->emitter, mapToGlobal(pos));
            break;

        case SceneTreeItem::EIT_Layer:
            ShowContextMenuLayer(((SceneTreeItemParticleLayer*)item)->emitter, SceneTreeItemParticleLayer::GetLayer(item), mapToGlobal(pos));
            break;

        case SceneTreeItem::EIT_InnerEmitter:
            ShowContextMenuInnerEmitter(((SceneTreeItemParticleEmitter*)item)->effect, ((SceneTreeItemParticleInnerEmitter*)item)->emitter, ((SceneTreeItemParticleInnerEmitter*)item)->parent, mapToGlobal(pos));
            break;

        case SceneTreeItem::EIT_Force:
        {
            // We have to know both Layer and Force.
            QStandardItem* parentItem = item->parent();
            if (!parentItem)
            {
                DVASSERT(false);
                return;
            }

            SceneTreeItem* layerItem = treeModel->GetItem(parentItem->index());
            DVASSERT(layerItem->ItemType() == SceneTreeItem::EIT_Layer);
            ShowContextMenuForce(SceneTreeItemParticleLayer::GetLayer(layerItem),
                                 SceneTreeItemParticleForce::GetForce(item), mapToGlobal(pos));
            break;
        }

        default:
            break;
        }
    }
}

void SceneTree::ShowContextMenuEntity(DAVA::Entity* entity, int entityCustomFlags, const QPoint& pos)
{
    if (entity == nullptr)
        return;

    // Get selection size to show different menues
    const SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    SceneSelectionSystem* selSystem = scene->selectionSystem;
    size_t selectionSize = selSystem->GetSelectionCount();

    Camera* camera = GetCamera(entity);

    QMenu contextMenu;
    if (entityCustomFlags & SceneTreeModel::CF_Disabled)
    {
        if (selectionSize == 1 && camera)
        {
            AddCameraActions(contextMenu);
            contextMenu.addSeparator();
        }

        if ((camera != scene->GetCurrentCamera()) && (entity->GetNotRemovable() == false))
        {
            contextMenu.addAction(SharedIcon(":/QtIcons/remove.png"), "Remove entity", this, SLOT(RemoveSelection()));
        }
    }
    else
    {
        // look at
        contextMenu.addAction(SharedIcon(":/QtIcons/zoom.png"), "Look at", this, SLOT(LookAtSelection()));

        // look from
        if (NULL != camera)
        {
            AddCameraActions(contextMenu);
        }

        // add/remove
        contextMenu.addSeparator();
        if (entity->GetLocked() == false && (camera != scene->GetCurrentCamera()) && (entity->GetNotRemovable() == false))
        {
            contextMenu.addAction(SharedIcon(":/QtIcons/remove.png"), "Remove entity", this, SLOT(RemoveSelection()));
        }

        // lock/unlock
        contextMenu.addSeparator();
        QAction* lockAction = contextMenu.addAction(SharedIcon(":/QtIcons/lock_add.png"), "Lock", QtMainWindow::Instance(), SLOT(OnLockTransform()));
        QAction* unlockAction = contextMenu.addAction(SharedIcon(":/QtIcons/lock_delete.png"), "Unlock", QtMainWindow::Instance(), SLOT(OnUnlockTransform()));
        if (entity->GetLocked())
        {
            lockAction->setDisabled(true);
        }
        else
        {
            unlockAction->setDisabled(true);
        }

        // show save as/reload/edit for regular entity
        // save model as
        contextMenu.addSeparator();
        contextMenu.addAction(SharedIcon(":/QtIcons/save_as.png"), "Save Entity As...", this, SLOT(SaveEntityAs()));

        DAVA::KeyedArchive* customProp = GetCustomPropertiesArchieve(entity);
        if (NULL != customProp)
        {
            DAVA::FilePath ownerRef = customProp->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
            if (!ownerRef.IsEmpty())
            {
                if (selectionSize == 1)
                {
                    contextMenu.addAction("Edit Model", this, SLOT(EditModel()));
                }

                contextMenu.addAction("Reload Model...", this, SLOT(ReloadModel()));
            }
        }
        // Reload for every entity at scene
        contextMenu.addAction("Reload Model As...", this, SLOT(ReloadModelAs()));

        // particle effect
        DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(entity);
        if (NULL != effect)
        {
            contextMenu.addSeparator();
            QMenu* particleEffectMenu = contextMenu.addMenu("Particle Effect");

            particleEffectMenu->addAction(SharedIcon(":/QtIcons/emitter_particle.png"), "Add Emitter", this, SLOT(AddEmitter()));
            particleEffectMenu->addAction(SharedIcon(":/QtIcons/savescene.png"), "Save Effect Emitters", this, SLOT(SaveEffectEmitters()));
            particleEffectMenu->addAction(SharedIcon(":/QtIcons/savescene.png"), "Save Effect Emitters As...", this, SLOT(SaveEffectEmittersAs()));
            particleEffectMenu->addSeparator();
            particleEffectMenu->addAction(SharedIcon(":/QtIcons/play.png"), "Start", this, SLOT(StartEffect()));
            particleEffectMenu->addAction(SharedIcon(":/QtIcons/stop.png"), "Stop", this, SLOT(StopEffect()));
            particleEffectMenu->addAction(SharedIcon(":/QtIcons/restart.png"), "Restart", this, SLOT(RestartEffect()));
        }

        if (selectionSize == 1)
        {
            contextMenu.addSeparator();
            contextMenu.addAction(SharedIcon(":/QtIconsTextureDialog/filter.png"), "Set name as filter", this, SLOT(SetEntityNameAsFilter()));
        }
    }

    contextMenu.exec(pos);
}

void SceneTree::ShowContextMenuLayer(DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer, const QPoint& pos)
{
    selectedEmitter = emitter;
    selectedLayer = layer;

    if (NULL == layer)
    {
        return;
    }

    QMenu contextMenu;
    contextMenu.addAction(SharedIcon(":/QtIcons/clone.png"), "Clone Layer", this, SLOT(CloneLayer()));
    contextMenu.addAction(SharedIcon(":/QtIcons/remove_layer.png"), "Remove Layer", this, SLOT(RemoveLayer()));
    contextMenu.addSeparator();
    contextMenu.addAction(SharedIcon(":/QtIcons/force.png"), "Add Force", this, SLOT(AddForce()));

    contextMenu.exec(pos);
}

void SceneTree::ShowContextMenuForce(DAVA::ParticleLayer* layer, DAVA::ParticleForce* force, const QPoint& pos)
{
    this->selectedLayer = layer;
    this->selectedForce = force;

    QMenu contextMenu;
    contextMenu.addAction(SharedIcon(":/QtIcons/remove_force.png"), "Remove Force", this, SLOT(RemoveForce()));
    contextMenu.exec(pos);
}

void SceneTree::ShowContextMenuEmitter(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, const QPoint& pos)
{
    selectedEffect = effect;
    selectedEmitter = emitter;

    QMenu contextMenu;
    contextMenu.addAction(SharedIcon(":/QtIcons/remove.png"), "Remove emitter", this, SLOT(RemoveEmitter()));
    contextMenu.addSeparator();
    contextMenu.addAction(SharedIcon(":/QtIcons/layer_particle.png"), "Add Layer", this, SLOT(AddLayer()));
    contextMenu.addSeparator();
    contextMenu.addAction(SharedIcon(":/QtIcons/openscene.png"), "Load Emitter from Yaml", this, SLOT(LoadEmitterFromYaml()));
    contextMenu.addAction(SharedIcon(":/QtIcons/savescene.png"), "Save Emitter to Yaml", this, SLOT(SaveEmitterToYaml()));
    contextMenu.addAction(SharedIcon(":/QtIcons/save_as.png"), "Save Emitter to Yaml As...", this, SLOT(SaveEmitterToYamlAs()));

    contextMenu.exec(pos);
}

void SceneTree::ShowContextMenuInnerEmitter(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* parentLayer, const QPoint& pos)
{
    selectedEffect = effect;
    selectedEmitter = emitter;
    selectedLayer = parentLayer;
    QMenu contextMenu;
    contextMenu.addAction(SharedIcon(":/QtIcons/layer_particle.png"), "Add Layer", this, SLOT(AddLayer()));
    contextMenu.addSeparator();
    contextMenu.addAction(SharedIcon(":/QtIcons/openscene.png"), "Load Emitter from Yaml", this, SLOT(LoadInnerEmitterFromYaml()));
    contextMenu.addAction(SharedIcon(":/QtIcons/savescene.png"), "Save Emitter to Yaml", this, SLOT(SaveInnerEmitterToYaml()));
    contextMenu.addAction(SharedIcon(":/QtIcons/save_as.png"), "Save Emitter to Yaml As...", this, SLOT(SaveInnerEmitterToYamlAs()));
    contextMenu.exec(pos);
}

void SceneTree::LookAtSelection()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (sceneEditor != nullptr)
    {
        sceneEditor->cameraSystem->MoveToSelection();
    }
}

void SceneTree::RemoveSelection()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (NULL != sceneEditor)
    {
        const SelectableObjectGroup& selection = sceneEditor->selectionSystem->GetSelection();

        SelectableObjectGroup objectToRemove;
        for (const auto& item : selection.GetContent())
        {
            auto entity = item.AsEntity();
            if ((entity != nullptr) && entity->GetLocked())
            {
                DAVA::StringStream ss;
                ss << "Can not remove entity "
                   << entity->GetName().c_str()
                   << ": entity is locked!"
                   << PointerSerializer::FromPointer(entity);
                Logger::Warning("%s", ss.str().c_str());
            }
            else
            {
                objectToRemove.Add(item.GetContainedObject(), item.GetBoundingBox());
            }
        }

        if (!objectToRemove.IsEmpty())
        {
            sceneEditor->structureSystem->Remove(objectToRemove);
        }
    }
}

void SceneTree::CollapseSwitch()
{
    QModelIndexList indexList = selectionModel()->selection().indexes();
    for (int i = 0; i < indexList.size(); ++i)
    {
        QModelIndex index = indexList.at(i);

        if (isExpanded(index))
        {
            collapse(index);
        }
        else
        {
            expand(index);
        }
    }
}

void SceneTree::EditModel()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (sceneEditor == nullptr)
        return;

    SceneSelectionSystem* ss = sceneEditor->selectionSystem;
    for (auto entity : ss->GetSelection().ObjectsOfType<DAVA::Entity>())
    {
        DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(entity);
        if (archive != nullptr)
        {
            DAVA::FilePath entityRefPath = archive->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
            if (FileSystem::Instance()->Exists(entityRefPath))
            {
                QtMainWindow::Instance()->OpenScene(entityRefPath.GetAbsolutePathname().c_str());
            }
            else
            {
                ShowErrorDialog(ResourceEditor::SCENE_TREE_WRONG_REF_TO_OWNER + entityRefPath.GetAbsolutePathname());
            }
        }
    }
}

void SceneTree::ReloadModel()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (sceneEditor == nullptr)
        return;

    QDialog dlg(this);

    QVBoxLayout* dlgLayout = new QVBoxLayout();
    dlgLayout->setMargin(10);

    dlg.setWindowTitle("Reload Model options");
    dlg.setLayout(dlgLayout);

    QCheckBox* lightmapsChBox = new QCheckBox("Leave lightmap settings", &dlg);
    dlgLayout->addWidget(lightmapsChBox);
    lightmapsChBox->setCheckState(Qt::Checked);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    dlgLayout->addWidget(buttons);

    QObject::connect(buttons, SIGNAL(accepted()), &dlg, SLOT(accept()));
    QObject::connect(buttons, SIGNAL(rejected()), &dlg, SLOT(reject()));

    if (QDialog::Accepted == dlg.exec())
    {
        const SelectableObjectGroup& selection = sceneEditor->selectionSystem->GetSelection();
        String wrongPathes;
        for (auto entity : selection.ObjectsOfType<DAVA::Entity>())
        {
            DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(entity);
            if (archive != nullptr)
            {
                DAVA::FilePath pathToReload(archive->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER));
                if (!FileSystem::Instance()->Exists(pathToReload))
                {
                    wrongPathes += Format("\r\n%s : %s", entity->GetName().c_str(),
                                          pathToReload.GetAbsolutePathname().c_str());
                }
            }
        }
        if (!wrongPathes.empty())
        {
            ShowErrorDialog(ResourceEditor::SCENE_TREE_WRONG_REF_TO_OWNER + wrongPathes);
        }
        SelectableObjectGroup newSelection = sceneEditor->structureSystem->ReloadEntities(selection, lightmapsChBox->isChecked());
        sceneEditor->selectionSystem->SetSelection(newSelection);
    }
}

void SceneTree::ReloadModelAs()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (NULL != sceneEditor)
    {
        DAVA::Entity* entity = sceneEditor->selectionSystem->GetFirstSelectionEntity();
        DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(entity);
        if (NULL != archive)
        {
            DAVA::String ownerPath = archive->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
            if (ownerPath.empty())
            {
                FilePath p = sceneEditor->GetScenePath().GetDirectory();
                if (FileSystem::Instance()->Exists(p) && sceneEditor->IsLoaded())
                {
                    ownerPath = p.GetAbsolutePathname();
                }
                else
                {
                    ownerPath = ProjectManager::Instance()->GetDataSourcePath().GetAbsolutePathname();
                }
            }

            QString filePath = FileDialog::getOpenFileName(NULL, QString("Open scene file"), ownerPath.c_str(), QString("DAVA SceneV2 (*.sc2)"));
            if (!filePath.isEmpty())
            {
                SelectableObjectGroup newSelection = sceneEditor->structureSystem->ReloadEntitiesAs(sceneEditor->selectionSystem->GetSelection(), filePath.toStdString());
                sceneEditor->selectionSystem->SetSelection(newSelection);
            }
        }
    }
}

void SceneTree::SaveEntityAs()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (sceneEditor == nullptr)
        return;

    const SelectableObjectGroup& selection = sceneEditor->selectionSystem->GetSelection();
    if (selection.ContainsObjectsOfType<DAVA::Entity>())
    {
        DAVA::FilePath scenePath = sceneEditor->GetScenePath().GetDirectory();
        if (!FileSystem::Instance()->Exists(scenePath) || !sceneEditor->IsLoaded())
        {
            scenePath = ProjectManager::Instance()->GetDataSourcePath();
        }

        QString filePath = FileDialog::getSaveFileName(nullptr, QString("Save scene file"), QString(scenePath.GetDirectory().GetAbsolutePathname().c_str()), QString("DAVA SceneV2 (*.sc2)"));
        if (!filePath.isEmpty())
        {
            sceneEditor->Exec(new SaveEntityAsAction(&selection, filePath.toStdString()));
        }
    }
}

void SceneTree::CollapseAll()
{
    QTreeView::collapseAll();
    bool needSync = false;
    {
        SceneTreeDetails::SyncGuard guard(isInSync);

        QModelIndexList indexList = selectionModel()->selection().indexes();
        for (int i = 0; i < indexList.size(); ++i)
        {
            QModelIndex childIndex = indexList[i];
            if (childIndex.parent().isValid())
            {
                selectionModel()->select(childIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
                needSync = true;
            }
        }
    }

    if (needSync)
    {
        TreeSelectionChanged(selectionModel()->selection(), QItemSelection());
    }

    PropagateSolidFlag();
}

void SceneTree::TreeItemCollapsed(const QModelIndex& index)
{
    treeModel->SetSolid(filteringProxyModel->mapToSource(index), true);

    bool needSync = false;
    {
        SceneTreeDetails::SyncGuard guard(isInSync);

        // if selected items were inside collapsed item, remove them from selection
        QModelIndexList indexList = selectionModel()->selection().indexes();
        for (int i = 0; i < indexList.size(); ++i)
        {
            QModelIndex childIndex = indexList[i];
            QModelIndex childParent = childIndex.parent();
            while (childParent.isValid())
            {
                if (childParent == index)
                {
                    selectionModel()->select(childIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
                    needSync = true;
                    break;
                }

                childParent = childParent.parent();
            }
        }
    }

    if (needSync)
    {
        TreeSelectionChanged(selectionModel()->selection(), QItemSelection());
    }
}

void SceneTree::TreeItemExpanded(const QModelIndex& index)
{
    QModelIndex mappedIndex = filteringProxyModel->mapToSource(index);
    treeModel->SetSolid(mappedIndex, false);
    QStandardItem* item = treeModel->itemFromIndex(mappedIndex);
    PropagateSolidFlagRecursive(item);
}

void SceneTree::SyncSelectionToTree()
{
    SceneEditor2* curScene = treeModel->GetScene();
    if (isInSync || (curScene == nullptr))
    {
        return;
    }

    SceneTreeDetails::SyncGuard guard(isInSync);

    using TSelectionMap = DAVA::Map<QModelIndex, DAVA::Vector<QModelIndex>>;
    TSelectionMap toSelect;

    QModelIndex lastValidIndex;
    const auto& selection = curScene->selectionSystem->GetSelection();
    for (const auto& item : selection.GetContent())
    {
        QModelIndex sIndex = filteringProxyModel->mapFromSource(treeModel->GetIndex(item.GetContainedObject()));
        if (sIndex.isValid())
        {
            lastValidIndex = sIndex;
            toSelect[sIndex.parent()].push_back(sIndex);
        }
    }

    QItemSelectionModel* selectModel = selectionModel();
    selectModel->clear();

    if (toSelect.empty())
        return;

    for (TSelectionMap::value_type& selectionNode : toSelect)
    {
        DAVA::Vector<QModelIndex>& indexes = selectionNode.second;
        sort(indexes.begin(), indexes.end(), [](const QModelIndex& left, const QModelIndex& right) {
            DVASSERT(left.parent() == right.parent());
            return left.row() < right.row();
        });

        int startIndex = 0;
        int lastIndex = startIndex;
        int lastRow = indexes[lastIndex].row();
        for (size_t i = 1; i < indexes.size(); ++i)
        {
            int currentRow = indexes[i].row();
            if (currentRow - lastRow < 2)
            {
                DVASSERT(currentRow - lastRow > 0);
                lastRow = currentRow;
                lastIndex = i;
            }
            else
            {
                QItemSelection selection(indexes[startIndex], indexes[lastIndex]);
                selectModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                startIndex = i;
                lastIndex = startIndex;
                lastRow = indexes[lastIndex].row();
            }
        }
        QItemSelection selection(indexes[startIndex], indexes[lastIndex]);
        selectModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    scrollTo(lastValidIndex, QAbstractItemView::PositionAtCenter);
}

void SceneTree::SyncSelectionFromTree()
{
    if (!isInSync)
    {
        SceneTreeDetails::SyncGuard guard(isInSync);

        SceneEditor2* curScene = treeModel->GetScene();
        if (NULL != curScene)
        {
            // select items in scene
            SelectableObjectGroup group;

            QModelIndexList indexList = selectionModel()->selection().indexes();
            for (int i = 0; i < indexList.size(); ++i)
            {
                auto item = treeModel->GetItem(filteringProxyModel->mapToSource(indexList[i]));
                switch (item->ItemType())
                {
                case SceneTreeItem::eItemType::EIT_Entity:
                {
                    auto entity = SceneTreeItemEntity::GetEntity(item);
                    group.Add(entity, curScene->selectionSystem->GetUntransformedBoundingBox(entity));
                    break;
                }

                case SceneTreeItem::eItemType::EIT_Emitter:
                case SceneTreeItem::eItemType::EIT_InnerEmitter:
                {
                    auto emitter = SceneTreeItemParticleEmitter::GetEmitter(item);
                    group.Add(emitter, curScene->selectionSystem->GetUntransformedBoundingBox(emitter));
                    break;
                }

                case SceneTreeItem::eItemType::EIT_Layer:
                {
                    auto layer = SceneTreeItemParticleLayer::GetLayer(item);
                    group.Add(layer, curScene->selectionSystem->GetUntransformedBoundingBox(layer));
                    break;
                }

                case SceneTreeItem::eItemType::EIT_Force:
                {
                    auto force = SceneTreeItemParticleForce::GetForce(item);
                    group.Add(force, curScene->selectionSystem->GetUntransformedBoundingBox(force));
                    break;
                }

                default:
                    DVASSERT_MSG(0, "INVALID OR UNKNOWN OBJECT SELECTED IN SCENE TREE");
                }
            }
            curScene->selectionSystem->SetSelection(group);

            // force selection system emit signals about new selection
            // this should be done until we are inSync mode, to prevent unnecessary updates
            // when signals from selection system will be emitted on next frame
            curScene->selectionSystem->ForceEmitSignals();
        }
    }
}

void SceneTree::EmitParticleSignals(const QItemSelection& selected)
{
    SceneEditor2* curScene = treeModel->GetScene();
    bool isParticleElements = false;
    bool emitterSelected = false;

    // allow only single selected entities
    if (selected.size() == 1)
    {
        QModelIndexList indexList = selectionModel()->selection().indexes();
        if (indexList.size())
        {
            SceneTreeItem* item = treeModel->GetItem(filteringProxyModel->mapToSource(indexList[0]));
            if (NULL != item)
            {
                switch (item->ItemType())
                {
                case SceneTreeItem::EIT_Entity:
                {
                    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(item);
                    if (NULL != DAVA::GetEffectComponent(entity))
                    {
                        SceneSignals::Instance()->EmitEffectSelected(curScene, GetEffectComponent(entity));
                        isParticleElements = true;
                    }
                }
                break;
                case SceneTreeItem::EIT_Emitter:
                    curScene->particlesSystem->SetEmitterSelected(((SceneTreeItemParticleEmitter*)item)->effect->GetEntity(), ((SceneTreeItemParticleEmitter*)item)->emitter);
                    emitterSelected = true;
                case SceneTreeItem::EIT_InnerEmitter:
                    SceneSignals::Instance()->EmitEmitterSelected(curScene, ((SceneTreeItemParticleEmitter*)item)->effect, ((SceneTreeItemParticleEmitter*)item)->emitter);
                    isParticleElements = true;
                    break;
                case SceneTreeItem::EIT_Layer:
                {
                    SceneTreeItemParticleLayer* itemLayer = (SceneTreeItemParticleLayer*)item;
                    if (NULL != itemLayer->emitter && NULL != itemLayer->layer)
                    {
                        SceneSignals::Instance()->EmitLayerSelected(curScene, itemLayer->effect, itemLayer->emitter, itemLayer->layer, false);
                        isParticleElements = true;
                    }
                }
                break;
                case SceneTreeItem::EIT_Force:
                {
                    SceneTreeItemParticleForce* itemForce = (SceneTreeItemParticleForce*)item;
                    DAVA::ParticleLayer* layer = itemForce->layer;
                    if (NULL != layer)
                    {
                        for (int i = 0; i < (int)layer->forces.size(); ++i)
                        {
                            if (layer->forces[i] == itemForce->force)
                            {
                                SceneSignals::Instance()->EmitForceSelected(curScene, layer, i);
                                isParticleElements = true;

                                break;
                            }
                        }
                    }
                }
                break;
                }
            }
        }
    }

    if (!emitterSelected)
        curScene->particlesSystem->SetEmitterSelected(NULL, NULL);
    if (!isParticleElements)
    {
        SceneSignals::Instance()->EmitEmitterSelected(NULL, NULL, NULL);
    }
}

void SceneTree::AddEmitter()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr != sceneEditor)
    {
        QModelIndex realIndex = filteringProxyModel->mapToSource(currentIndex());
        DAVA::Entity* curEntity = SceneTreeItemEntity::GetEntity(treeModel->GetItem(realIndex));
        if (nullptr != curEntity && DAVA::GetEffectComponent(curEntity))
        {
            ExecuteModifyingCommand(new CommandAddParticleEmitter(curEntity));
        }
    }
}

void SceneTree::SaveEffectEmitters()
{
    PerformSaveEffectEmitters(false);
}

void SceneTree::SaveEffectEmittersAs()
{
    PerformSaveEffectEmitters(true);
}

void SceneTree::StartEffect()
{
    ExecuteCommandForSelectedEffects<CommandStartStopParticleEffect>(true);
}

void SceneTree::StopEffect()
{
    ExecuteCommandForSelectedEffects<CommandStartStopParticleEffect>(false);
}

void SceneTree::RestartEffect()
{
    ExecuteCommandForSelectedEffects<CommandRestartParticleEffect>();
}

void SceneTree::RemoveEmitter()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor)
        return;

    DVASSERT((nullptr != selectedEffect) && (nullptr != selectedEmitter));
    ExecuteModifyingCommand(new CommandRemoveParticleEmitter(selectedEffect, selectedEmitter));
}

void SceneTree::AddLayer()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor)
        return;

    SceneTreeItem* curItem = treeModel->GetItem(filteringProxyModel->mapToSource(currentIndex()));
    if ((curItem->ItemType() == SceneTreeItem::EIT_Emitter) || (curItem->ItemType() == SceneTreeItem::EIT_InnerEmitter))
    {
        auto emitter = ((SceneTreeItemParticleEmitter*)curItem)->emitter;
        ExecuteModifyingCommand(new CommandAddParticleEmitterLayer(emitter));
    }
}

void SceneTree::LoadEmitterFromYaml()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor || nullptr == selectedEmitter)
        return;

    QString filePath = FileDialog::getOpenFileName(nullptr, QString("Open Particle Emitter Yaml file"), GetParticlesConfigPath(), QString("YAML File (*.yaml)"));
    if (filePath.isEmpty() == false)
    {
        ExecuteModifyingCommand(new CommandLoadParticleEmitterFromYaml(selectedEffect, selectedEmitter, filePath.toStdString()));
    }
}

void SceneTree::SaveEmitterToYaml()
{
    PerformSaveEmitter(selectedEffect, selectedEmitter, false, QString());
}

void SceneTree::SaveEmitterToYamlAs()
{
    PerformSaveEmitter(selectedEffect, selectedEmitter, true, QString());
}

void SceneTree::LoadInnerEmitterFromYaml()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor || nullptr == selectedEmitter || nullptr == selectedLayer)
    {
        return;
    }

    QString filePath = FileDialog::getOpenFileName(NULL, QString("Open Particle Emitter Yaml file"),
                                                   GetParticlesConfigPath(), QString("YAML File (*.yaml)"));
    if (filePath.isEmpty())
    {
        return;
    }

    selectedLayer->innerEmitterPath = filePath.toStdString();
    ExecuteModifyingCommand(new CommandLoadInnerParticleEmitterFromYaml(selectedEmitter, filePath.toStdString()));
}

void SceneTree::SaveInnerEmitterToYaml()
{
    PerformSaveInnerEmitter(false);
}

void SceneTree::SaveInnerEmitterToYamlAs()
{
    PerformSaveInnerEmitter(true);
}

void SceneTree::PerformSaveInnerEmitter(bool forceAskFileName)
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor || nullptr == selectedEmitter)
    {
        return;
    }

    forceAskFileName |= selectedEmitter->configPath.IsEmpty();

    FilePath yamlPath = selectedEmitter->configPath;
    if (forceAskFileName)
    {
        QString particlesConfigPath = ProjectManager::Instance()->GetParticlesConfigPath().GetAbsolutePathname().c_str();
        QString filePath = FileDialog::getSaveFileName(nullptr, QString("Save Particle Emitter YAML file"),
                                                       particlesConfigPath, QString("YAML File (*.yaml)"));

        if (filePath.isEmpty())
        {
            return;
        }

        yamlPath = FilePath(filePath.toStdString());
    }

    selectedLayer->innerEmitterPath = yamlPath;
    CommandSaveInnerParticleEmitterToYaml* command = new CommandSaveInnerParticleEmitterToYaml(selectedEmitter, yamlPath);
    sceneEditor->Exec(command);
    if (forceAskFileName)
    {
        sceneEditor->MarkAsChanged();
    }
}

void SceneTree::CloneLayer()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor)
    {
        return;
    }

    DVASSERT((nullptr != selectedEmitter) && (nullptr != selectedLayer));
    ExecuteModifyingCommand(new CommandCloneParticleEmitterLayer(selectedEmitter, selectedLayer));
}

void SceneTree::RemoveLayer()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor)
        return;

    DVASSERT(selectedLayer != nullptr);
    ExecuteModifyingCommand(new CommandRemoveParticleEmitterLayer(selectedEmitter, selectedLayer));
}

void SceneTree::AddForce()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor)
    {
        return;
    }

    DVASSERT(selectedLayer != nullptr);
    ExecuteModifyingCommand(new CommandAddParticleEmitterForce(selectedLayer));
}

void SceneTree::RemoveForce()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor)
    {
        return;
    }

    DVASSERT((nullptr != selectedLayer) && (nullptr != selectedForce));
    ExecuteModifyingCommand(new CommandRemoveParticleEmitterForce(selectedLayer, selectedForce));
}

void SceneTree::PerformSaveEmitter(ParticleEffectComponent* effect, ParticleEmitter* emitter, bool forceAskFileName, const QString& defaultName)
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr == sceneEditor || nullptr == emitter)
    {
        return;
    }

    // Verify whether we have to ask about the file name. If emitter
    // does not have emitter path - treat this as "force ask".
    forceAskFileName |= (emitter->configPath.IsEmpty());

    FilePath yamlPath = emitter->configPath;
    if (forceAskFileName)
    {
        FilePath defaultPath = SettingsManager::GetValue(Settings::Internal_ParticleLastEmitterDir).AsFilePath();
        QString particlesPath = defaultPath.IsEmpty() ? ProjectManager::Instance()->GetParticlesConfigPath().GetAbsolutePathname().c_str() : defaultPath.GetAbsolutePathname().c_str();

        FileSystem::Instance()->CreateDirectory(FilePath(particlesPath.toStdString()), true); //to ensure that folder is created

        QString emitterPathname = particlesPath + defaultName;
        QString filePath = FileDialog::getSaveFileName(NULL, QString("Save Particle Emitter ") + QString(emitter->name.c_str()),
                                                       emitterPathname, QString("YAML File (*.yaml)"));

        if (filePath.isEmpty())
        {
            return;
        }

        yamlPath = FilePath(filePath.toStdString());

        SettingsManager::SetValue(Settings::Internal_ParticleLastEmitterDir, VariantType(yamlPath.GetDirectory()));
    }

    CommandSaveParticleEmitterToYaml* command = new CommandSaveParticleEmitterToYaml(effect, emitter, yamlPath);
    sceneEditor->Exec(command);
    if (forceAskFileName)
    {
        sceneEditor->MarkAsChanged();
    }
}

void SceneTree::PerformSaveEffectEmitters(bool forceAskFileName)
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (!sceneEditor)
        return;
    QModelIndex realIndex = filteringProxyModel->mapToSource(currentIndex());
    Entity* entity = SceneTreeItemEntity::GetEntity(treeModel->GetItem(realIndex));
    ParticleEffectComponent* effect = DAVA::GetEffectComponent(entity);
    if (!effect)
        return;

    QString effectName(entity->GetName().c_str());
    for (int32 i = 0, sz = effect->GetEmittersCount(); i != sz; ++i)
    {
        ParticleEmitter* emitter = effect->GetEmitter(i);
        QString defName = effectName + "_" + QString::number(i + 1) + "_" + QString(emitter->name.c_str()) + ".yaml";
        PerformSaveEmitter(effect, emitter, forceAskFileName, defName);
    }
}

QString SceneTree::GetParticlesConfigPath()
{
    return ProjectManager::Instance()->GetParticlesConfigPath().GetAbsolutePathname().c_str();
}

void SceneTree::CleanupParticleEditorSelectedItems()
{
    this->selectedLayer = NULL;
    this->selectedForce = NULL;
    this->selectedEmitter = NULL;
    this->selectedEffect = NULL;
}

void SceneTree::SetEntityNameAsFilter()
{
    SceneEditor2* scene = treeModel->GetScene();
    if (scene == nullptr)
        return;

    const SelectableObjectGroup& selection = scene->selectionSystem->GetSelection();
    if (selection.GetSize() == 1)
    {
        Entity* entity = selection.GetFirst().AsEntity();
        DVASSERT(entity != nullptr);

        QtMainWindow::Instance()->GetUI()->sceneTreeFilterEdit->setText(entity->GetName().c_str());
    }
}

void SceneTree::AddCameraActions(QMenu& menu)
{
    menu.addAction(SharedIcon(":/QtIcons/eye.png"), "Look from", this, SLOT(SetCurrentCamera()));
    menu.addAction(SharedIcon(":/QtIcons/camera.png"), "Set custom draw camera", this, SLOT(SetCustomDrawCamera()));
}

void SceneTree::ExpandFilteredItems()
{
    QSet<QModelIndex> indexSet;
    BuildExpandItemsSet(indexSet);

    for (auto i = indexSet.begin(); i != indexSet.end(); ++i)
    {
        expand(*i);
    }
}

void SceneTree::BuildExpandItemsSet(QSet<QModelIndex>& indexSet, const QModelIndex& parent)
{
    const int n = filteringProxyModel->rowCount(parent);
    for (int i = 0; i < n; i++)
    {
        const QModelIndex _index = filteringProxyModel->index(i, 0, parent);
        SceneTreeItem* item = treeModel->GetItem(filteringProxyModel->mapToSource(_index));
        if (item->IsHighlighed())
        {
            indexSet << _index.parent();
        }
        BuildExpandItemsSet(indexSet, _index);
    }
}

void SceneTree::SetCurrentCamera()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (NULL != sceneEditor)
    {
        DAVA::Camera* camera = GetCamera(sceneEditor->selectionSystem->GetFirstSelectionEntity());
        if (NULL != camera)
        {
            sceneEditor->SetCurrentCamera(camera);
        }
    }
}

void SceneTree::SetCustomDrawCamera()
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (NULL != sceneEditor)
    {
        DAVA::Camera* camera = GetCamera(sceneEditor->selectionSystem->GetFirstSelectionEntity());
        if (NULL != camera)
        {
            sceneEditor->SetCustomDrawCamera(camera);
        }
    }
}

void SceneTree::UpdateTree()
{
    dataChanged(QModelIndex(), QModelIndex());
}

void SceneTree::PropagateSolidFlag()
{
    QStandardItem* root = treeModel->invisibleRootItem();
    for (int i = 0; i < root->rowCount(); ++i)
    {
        PropagateSolidFlagRecursive(root->child(i));
    }
}

void SceneTree::PropagateSolidFlagRecursive(QStandardItem* root)
{
    DVASSERT(root != nullptr);
    QModelIndex rootIndex = root->index();
    DVASSERT(rootIndex.isValid());
    QModelIndex filteredIndex = filteringProxyModel->mapFromSource(rootIndex);
    if (isExpanded(filteredIndex))
    {
        treeModel->SetSolid(rootIndex, false);
        for (int i = 0; i < root->rowCount(); ++i)
        {
            PropagateSolidFlagRecursive(root->child(i));
        }
    }
    else
    {
        treeModel->SetSolid(rootIndex, true);
    }
}

void SceneTree::ExecuteModifyingCommand(Command2* command)
{
    SceneEditor2* sceneEditor = treeModel->GetScene();
    sceneEditor->Exec(command);
    sceneEditor->MarkAsChanged();
    treeModel->ResyncStructure(treeModel->invisibleRootItem(), sceneEditor);
}
