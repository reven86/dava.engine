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
#include <QSignalBlocker>

#include "Qt/Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Main/Guards.h"
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

#include "QtTools/Updaters/LazyUpdater.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

namespace SceneTreeDetails
{
QString GetParticlesConfigPath()
{
    return ProjectManager::Instance()->GetParticlesConfigPath().GetAbsolutePathname().c_str();
}

void SaveEmitter(SceneEditor2* scene, DAVA::ParticleEffectComponent* component, DAVA::ParticleEmitter* emitter,
                 bool askFileName, const QString& defaultName, const DAVA::Function<Command2::Pointer(const FilePath&)>& commandCreator)
{
    askFileName |= emitter->configPath.IsEmpty();

    FilePath yamlPath = emitter->configPath;
    if (askFileName)
    {
        FilePath defaultPath = SettingsManager::GetValue(Settings::Internal_ParticleLastEmitterDir).AsFilePath();
        QString particlesPath = defaultPath.IsEmpty() ? ProjectManager::Instance()->GetParticlesConfigPath().GetAbsolutePathname().c_str() : defaultPath.GetAbsolutePathname().c_str();

        FileSystem::Instance()->CreateDirectory(FilePath(particlesPath.toStdString()), true); //to ensure that folder is created

        QString emitterPathname = particlesPath + defaultName;
        QString title = QStringLiteral("Save Particle Emitter ") + QString(emitter->name.c_str());
        QString filePath = FileDialog::getSaveFileName(nullptr, title, emitterPathname, QStringLiteral("YAML File (*.yaml)"));

        if (filePath.isEmpty())
        {
            return;
        }

        yamlPath = FilePath(filePath.toStdString());

        SettingsManager::SetValue(Settings::Internal_ParticleLastEmitterDir, VariantType(yamlPath.GetDirectory()));
    }

    scene->Exec(commandCreator(yamlPath));
    if (askFileName)
    {
        scene->SetChanged(true);
    }
}
}

class SceneTree::BaseContextMenu : public QObject
{
public:
    BaseContextMenu(SceneTree* treeWidget_)
        : treeWidget(treeWidget_)
    {
    }

    void Show(const QPoint& pos)
    {
        if (!IsValidForShow())
            return;

        QMenu menu;
        FillActions(menu);
        menu.exec(pos);
    }

    bool IsStructureChanged() const
    {
        return isStructureChanged;
    }

protected:
    virtual bool IsValidForShow() const
    {
        return GetScene() != nullptr;
    }

    virtual void FillActions(QMenu& menu) = 0;

    template <typename Func>
    void Connect(QAction* action, const typename QtPrivate::FunctionPointer<Func>::Object* receiver, Func slot)
    {
        DVVERIFY(QObject::connect(action, &QAction::triggered, receiver, slot));
    }

    template <typename Func>
    void Connect(QAction* action, Func slot)
    {
        DVVERIFY(QObject::connect(action, &QAction::triggered, slot));
    }

    void MarkStructureChanged()
    {
        GetScene()->SetChanged(true);
        isStructureChanged = true;
    }

    SceneEditor2* GetScene() const
    {
        return GetSceneModel()->GetScene();
    }

    SceneTreeModel* GetSceneModel() const
    {
        return treeWidget->treeModel;
    }

    SceneTreeFilteringModel* GetFilteredModel() const
    {
        return treeWidget->filteringProxyModel;
    }

    void ForEachSelectedByType(SceneTreeItem::eItemType type, const DAVA::Function<void(SceneTreeItem*)>& callback)
    {
        foreach (QModelIndex index, treeWidget->selectionModel()->selectedRows())
        {
            QModelIndex srcIndex = treeWidget->filteringProxyModel->mapToSource(index);
            SceneTreeItem* item = treeWidget->treeModel->GetItem(srcIndex);
            DVASSERT(item != nullptr);

            if (static_cast<SceneTreeItem::eItemType>(item->ItemType()) == type)
            {
                callback(item);
            }
        }
    }

    bool IsMultiselect() const
    {
        return treeWidget->selectionModel()->selectedRows().size() > 1;
    }

    QWidget* GetParentWidget()
    {
        return treeWidget;
    }

private:
    SceneTree* treeWidget;
    bool isStructureChanged = false;
};

class SceneTree::EntityContextMenu : public SceneTree::BaseContextMenu
{
    using TBase = BaseContextMenu;

public:
    EntityContextMenu(SceneTreeItemEntity* item, SceneTree* treeWidget)
        : TBase(treeWidget)
        , entityItem(item)
    {
    }

protected:
    bool IsValidForShow() const override
    {
        return TBase::IsValidForShow() && entityItem != nullptr;
    }

    void FillActions(QMenu& menu) override
    {
        SceneEditor2* scene = GetScene();
        size_t selectionSize = scene->selectionSystem->GetSelectionCount();

        Entity* entity = entityItem->entity;
        Camera* camera = GetCamera(entityItem->entity);

        if (GetSceneModel()->GetCustomFlags(entityItem->index()) & SceneTreeModel::CF_Disabled)
        {
            if (selectionSize == 1 && camera != nullptr)
            {
                FillCameraActions(menu);
                menu.addSeparator();
            }

            if ((camera != scene->GetCurrentCamera()) && (entity->GetNotRemovable() == false))
            {
                Connect(menu.addAction(SharedIcon(":/QtIcons/remove.png"), QStringLiteral("Remove entity")), [scene] { RemoveSelection(scene); });
            }
        }
        else
        {
            Connect(menu.addAction(SharedIcon(":/QtIcons/zoom.png"), QStringLiteral("Look at")), [scene] { LookAtSelection(scene); });
            if (camera != nullptr)
            {
                FillCameraActions(menu);
            }

            menu.addSeparator();
            if (entity->GetLocked() == false && (camera != scene->GetCurrentCamera()) && (entity->GetNotRemovable() == false))
            {
                Connect(menu.addAction(SharedIcon(":/QtIcons/remove.png"), QStringLiteral("Remove entity")), [scene] { RemoveSelection(scene); });
            }

            menu.addSeparator();
            QAction* lockAction = menu.addAction(SharedIcon(":/QtIcons/lock_add.png"), QStringLiteral("Lock"));
            QAction* unlockAction = menu.addAction(SharedIcon(":/QtIcons/lock_delete.png"), QStringLiteral("Unlock"));
            Connect(lockAction, [scene] { LockTransform(scene); });
            Connect(unlockAction, [scene] { UnlockTransform(scene); });
            if (entity->GetLocked())
            {
                lockAction->setDisabled(true);
            }
            else
            {
                unlockAction->setDisabled(true);
            }

            menu.addSeparator();
            Connect(menu.addAction(SharedIcon(":/QtIcons/save_as.png"), QStringLiteral("Save Entity As...")), this, &EntityContextMenu::SaveEntityAs);

            DAVA::KeyedArchive* customProp = GetCustomPropertiesArchieve(entity);
            if (nullptr != customProp)
            {
                DAVA::FilePath ownerRef = customProp->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
                if (!ownerRef.IsEmpty())
                {
                    if (selectionSize == 1)
                    {
                        Connect(menu.addAction(QStringLiteral("Edit Model")), this, &EntityContextMenu::EditModel);
                    }

                    Connect(menu.addAction(QStringLiteral("Reload Model...")), this, &EntityContextMenu::ReloadModel);
                }
            }

            Connect(menu.addAction(QStringLiteral("Reload Model As...")), this, &EntityContextMenu::ReloadModelAs);

            // particle effect
            DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(entity);
            if (nullptr != effect)
            {
                menu.addSeparator();
                QMenu* particleEffectMenu = menu.addMenu("Particle Effect");

                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/emitter_particle.png"), QStringLiteral("Add Emitter")), this, &EntityContextMenu::AddEmitter);
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/savescene.png"), QStringLiteral("Save Effect Emitters")), this, &EntityContextMenu::SaveEffectEmitters);
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/savescene.png"), QStringLiteral("Save Effect Emitters As...")), this, &EntityContextMenu::SaveEffectEmittersAs);
                particleEffectMenu->addSeparator();
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/play.png"), QStringLiteral("Start")), this, &EntityContextMenu::StartEffect);
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/stop.png"), QStringLiteral("Stop")), this, &EntityContextMenu::StopEffect);
                Connect(particleEffectMenu->addAction(SharedIcon(":/QtIcons/restart.png"), QStringLiteral("Restart")), this, &EntityContextMenu::RestartEffect);
            }

            if (selectionSize == 1)
            {
                menu.addSeparator();
                Connect(menu.addAction(SharedIcon(":/QtIconsTextureDialog/filter.png"), QStringLiteral("Set name as filter")), this, &EntityContextMenu::SetEntityNameAsFilter);
            }
        }
    }

private:
    void FillCameraActions(QMenu& menu)
    {
        Connect(menu.addAction(SharedIcon(":/QtIcons/eye.png"), QStringLiteral("Look from")), this, &EntityContextMenu::SetCurrentCamera);
        Connect(menu.addAction(SharedIcon(":/QtIcons/camera.png"), QStringLiteral("Set custom draw camera")), this, &EntityContextMenu::SetCustomDrawCamera);
    }

    void SaveEntityAs()
    {
        SceneEditor2* scene = GetScene();
        const EntityGroup& selection = scene->selectionSystem->GetSelection();
        if (!selection.IsEmpty())
        {
            DAVA::FilePath scenePath = scene->GetScenePath().GetDirectory();
            if (!FileSystem::Instance()->Exists(scenePath) || !scene->IsLoaded())
            {
                scenePath = ProjectManager::Instance()->GetDataSourcePath();
            }

            QString baseDir(scenePath.GetDirectory().GetAbsolutePathname().c_str());
            QString filePath = FileDialog::getSaveFileName(nullptr, QStringLiteral("Save scene file"), baseDir, QStringLiteral("DAVA SceneV2 (*.sc2)"));
            if (!filePath.isEmpty())
            {
                scene->Exec(Command2::Create<SaveEntityAsAction>(&selection, filePath.toStdString()));
            }
        }
    }

    void EditModel()
    {
        SceneSelectionSystem* ss = GetScene()->selectionSystem;
        for (const auto& item : ss->GetSelection().GetContent())
        {
            DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(item.first);
            if (archive)
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

    void ReloadModel()
    {
        SceneEditor2* sceneEditor = GetScene();
        QDialog dlg(GetParentWidget());

        QVBoxLayout* dlgLayout = new QVBoxLayout();
        dlgLayout->setMargin(10);

        dlg.setWindowTitle("Reload Model options");
        dlg.setLayout(dlgLayout);

        QCheckBox* lightmapsChBox = new QCheckBox(QStringLiteral("Leave lightmap settings"), &dlg);
        dlgLayout->addWidget(lightmapsChBox);
        lightmapsChBox->setCheckState(Qt::Checked);

        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
        dlgLayout->addWidget(buttons);

        QObject::connect(buttons, SIGNAL(accepted()), &dlg, SLOT(accept()));
        QObject::connect(buttons, SIGNAL(rejected()), &dlg, SLOT(reject()));

        if (QDialog::Accepted == dlg.exec())
        {
            const EntityGroup& selection = sceneEditor->selectionSystem->GetSelection();
            String wrongPathes;
            for (const auto& item : selection.GetContent())
            {
                DAVA::Entity* entity = item.first;
                DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(entity);
                if (archive)
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
            EntityGroup newSelection = sceneEditor->structureSystem->ReloadEntities(selection, lightmapsChBox->isChecked());
            sceneEditor->selectionSystem->SetSelection(newSelection);
        }
    }

    void ReloadModelAs()
    {
        SceneEditor2* sceneEditor = GetScene();
        DAVA::Entity* entity = sceneEditor->selectionSystem->GetFirstSelectionEntity();
        DAVA::KeyedArchive* archive = GetCustomPropertiesArchieve(entity);
        if (archive != nullptr)
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

            QString filePath = FileDialog::getOpenFileName(GetParentWidget(), QStringLiteral("Open scene file"), ownerPath.c_str(), QStringLiteral("DAVA SceneV2 (*.sc2)"));
            if (!filePath.isEmpty())
            {
                EntityGroup newSelection = sceneEditor->structureSystem->ReloadEntitiesAs(sceneEditor->selectionSystem->GetSelection(), filePath.toStdString());
                sceneEditor->selectionSystem->SetSelection(newSelection);
            }
        }
    }

    void AddEmitter()
    {
        Entity* entity = entityItem->entity;
        DVASSERT(DAVA::GetEffectComponent(entity) != nullptr);

        GetScene()->Exec(Command2::Create<CommandAddParticleEmitter>(entity));
        MarkStructureChanged();
    }

    void SaveEffectEmitters()
    {
        PerformSaveEffectEmitters(false);
    }

    void SaveEffectEmittersAs()
    {
        PerformSaveEffectEmitters(true);
    }

    void PerformSaveEffectEmitters(bool forceAskFileName)
    {
        SceneEditor2* sceneEditor = GetScene();
        ParticleEffectComponent* effect = DAVA::GetEffectComponent(entityItem->entity);
        DVASSERT(effect != nullptr);

        QString effectName(entityItem->entity->GetName().c_str());
        for (int32 i = 0, sz = effect->GetEmittersCount(); i != sz; ++i)
        {
            ParticleEmitter* emitter = effect->GetEmitter(i);
            QString defName = effectName + "_" + QString::number(i + 1) + "_" + QString(emitter->name.c_str()) + ".yaml";
            SceneTreeDetails::SaveEmitter(sceneEditor, effect, emitter, forceAskFileName, defName, [&](const DAVA::FilePath& path)
                                          {
                                              return Command2::Create<CommandSaveParticleEmitterToYaml>(effect, emitter, path);
                                          });
        }
    }

    void StartEffect()
    {
        SceneEditor2* sceneEditor = GetScene();
        SceneSelectionSystem* ss = sceneEditor->selectionSystem;
        for (const auto& item : ss->GetSelection().GetContent())
        {
            DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(item.first);
            if (nullptr != effect)
            {
                sceneEditor->Exec(Command2::Create<CommandStartStopParticleEffect>(item.first, true));
            }
        }
    }

    void StopEffect()
    {
        SceneEditor2* sceneEditor = GetScene();
        SceneSelectionSystem* ss = sceneEditor->selectionSystem;
        for (const auto& item : ss->GetSelection().GetContent())
        {
            DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(item.first);
            if (nullptr != effect)
            {
                sceneEditor->Exec(Command2::Create<CommandStartStopParticleEffect>(item.first, false));
            }
        }
    }

    void RestartEffect()
    {
        SceneEditor2* sceneEditor = GetScene();
        SceneSelectionSystem* ss = sceneEditor->selectionSystem;
        for (const auto& item : ss->GetSelection().GetContent())
        {
            DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(item.first);
            if (nullptr != effect)
            {
                sceneEditor->Exec(Command2::Create<CommandRestartParticleEffect>(item.first));
            }
        }
    }

    void SetEntityNameAsFilter()
    {
        const EntityGroup& selection = GetScene()->selectionSystem->GetSelection();
        DVASSERT(selection.Size() == 1);
        QtMainWindow::Instance()->GetUI()->sceneTreeFilterEdit->setText(selection.GetFirstEntity()->GetName().c_str());
    }

    void SetCurrentCamera()
    {
        DAVA::Camera* camera = GetCamera(entityItem->entity);
        DVASSERT(camera != nullptr);
        GetScene()->SetCurrentCamera(camera);
    }

    void SetCustomDrawCamera()
    {
        DAVA::Camera* camera = GetCamera(entityItem->entity);
        DVASSERT(camera != nullptr);
        GetScene()->SetCustomDrawCamera(camera);
    }

private:
    SceneTreeItemEntity* entityItem;
};

class SceneTree::ParticleLayerContextMenu : public BaseContextMenu
{
    using TBase = BaseContextMenu;

public:
    ParticleLayerContextMenu(SceneTreeItemParticleLayer* layerItem_, SceneTree* treeWidget)
        : TBase(treeWidget)
        , layerItem(layerItem_)
    {
    }

protected:
    bool IsValidForShow() const override
    {
        return TBase::IsValidForShow() && layerItem != nullptr;
    }

    void FillActions(QMenu& menu) override
    {
        Connect(menu.addAction(SharedIcon(":/QtIcons/clone.png"), QStringLiteral("Clone Layer")), this, &ParticleLayerContextMenu::CloneLayer);
        QString removeLayerText = IsMultiselect() == false ? QStringLiteral("Remove Layer") : QStringLiteral("Remove Layers");
        Connect(menu.addAction(SharedIcon(":/QtIcons/remove_layer.png"), removeLayerText), this, &ParticleLayerContextMenu::RemoveLayer);
        menu.addSeparator();
        Connect(menu.addAction(SharedIcon(":/QtIcons/force.png"), QStringLiteral("Add Force")), this, &ParticleLayerContextMenu::AddForce);
    }

private:
    void CloneLayer()
    {
        GetScene()->Exec(Command2::Create<CommandCloneParticleEmitterLayer>(layerItem->emitter, layerItem->layer));
        MarkStructureChanged();
    }

    void RemoveLayer()
    {
        SceneEditor2* sceneEditor = GetScene();
        bool hasSelectedLayers = false;
        ForEachSelectedByType(SceneTreeItem::EIT_Layer, [&sceneEditor, &hasSelectedLayers](SceneTreeItem* item)
                              {
                                  hasSelectedLayers = true;
                                  SceneTreeItemParticleLayer* layerItem = static_cast<SceneTreeItemParticleLayer*>(item);
                                  sceneEditor->Exec(Command2::Create<CommandRemoveParticleEmitterLayer>(layerItem->emitter, layerItem->layer));
                              });

        DVASSERT(hasSelectedLayers == true);
        MarkStructureChanged();
    }

    void AddForce()
    {
        GetScene()->Exec(Command2::Create<CommandAddParticleEmitterForce>(layerItem->layer));
        MarkStructureChanged();
    }

private:
    SceneTreeItemParticleLayer* layerItem;
};

class SceneTree::ParticleForceContextMenu : public SceneTree::BaseContextMenu
{
    using TBase = BaseContextMenu;

public:
    ParticleForceContextMenu(SceneTree* treeWidget)
        : TBase(treeWidget)
    {
    }

protected:
    void FillActions(QMenu& menu) override
    {
        QString removeForce = IsMultiselect() == false ? QStringLiteral("Remove Forces") : QStringLiteral("Remove Force");
        Connect(menu.addAction(SharedIcon(":/QtIcons/remove_force.png"), removeForce), this, &ParticleForceContextMenu::RemoveForce);
    }

private:
    void RemoveForce()
    {
        SceneEditor2* sceneEditor = GetScene();
        bool hasSelectedForces = false;
        ForEachSelectedByType(SceneTreeItem::EIT_Force, [&sceneEditor, &hasSelectedForces](SceneTreeItem* item)
                              {
                                  hasSelectedForces = true;
                                  SceneTreeItemParticleForce* forceItem = static_cast<SceneTreeItemParticleForce*>(item);
                                  sceneEditor->Exec(Command2::Create<CommandRemoveParticleEmitterForce>(forceItem->layer, forceItem->force));
                              });

        MarkStructureChanged();
    }
};

class SceneTree::ParticleEmitterContextMenu : public SceneTree::BaseContextMenu
{
    using TBase = BaseContextMenu;

public:
    ParticleEmitterContextMenu(SceneTreeItemParticleEmitter* emitter, SceneTree* treeWidget)
        : TBase(treeWidget)
        , emitterItem(emitter)
    {
    }

protected:
    bool IsValidForShow() const override
    {
        return TBase::IsValidForShow() && emitterItem != nullptr;
    }

    virtual bool IsRemovable() const
    {
        return true;
    }

    void FillActions(QMenu& menu) override
    {
        if (IsRemovable())
        {
            QString removeEmitterText = IsMultiselect() == false ? QStringLiteral("Remove emitter") : QStringLiteral("Remove emitters");
            Connect(menu.addAction(SharedIcon(":/QtIcons/remove.png"), removeEmitterText), this, &ParticleEmitterContextMenu::RemoveEmitter);
            menu.addSeparator();
        }
        Connect(menu.addAction(SharedIcon(":/QtIcons/layer_particle.png"), QStringLiteral("Add Layer")), this, &ParticleEmitterContextMenu::AddLayer);
        menu.addSeparator();
        Connect(menu.addAction(SharedIcon(":/QtIcons/openscene.png"), QStringLiteral("Load Emitter from Yaml")), this, &ParticleEmitterContextMenu::LoadEmitterFromYaml);
        Connect(menu.addAction(SharedIcon(":/QtIcons/savescene.png"), QStringLiteral("Save Emitter to Yaml")), this, &ParticleEmitterContextMenu::SaveEmitterToYaml);
        Connect(menu.addAction(SharedIcon(":/QtIcons/save_as.png"), QStringLiteral("Save Emitter to Yaml As...")), this, &ParticleEmitterContextMenu::SaveEmitterToYamlAs);
    }

    void RemoveEmitter()
    {
        SceneEditor2* sceneEditor = GetScene();
        bool hasSelectedForces = false;
        ForEachSelectedByType(SceneTreeItem::EIT_Emitter, [&sceneEditor, &hasSelectedForces](SceneTreeItem* item)
                              {
                                  hasSelectedForces = true;
                                  SceneTreeItemParticleEmitter* emitterItem = static_cast<SceneTreeItemParticleEmitter*>(item);
                                  sceneEditor->Exec(Command2::Create<CommandRemoveParticleEmitter>(emitterItem->effect, emitterItem->emitter));
                              });

        MarkStructureChanged();
    }

    void AddLayer()
    {
        GetScene()->Exec(Command2::Create<CommandAddParticleEmitterLayer>(emitterItem->emitter));
        MarkStructureChanged();
    }

    void LoadEmitterFromYaml()
    {
        QString filePath = FileDialog::getOpenFileName(nullptr, QStringLiteral("Open Particle Emitter Yaml file"),
                                                       SceneTreeDetails::GetParticlesConfigPath(), QStringLiteral("YAML File (*.yaml)"));
        if (filePath.isEmpty())
        {
            return;
        }

        GetScene()->Exec(CreateLoadCommand(filePath.toStdString()));
        MarkStructureChanged();
    }

    void SaveEmitterToYaml()
    {
        SaveEmitter(false);
    }

    void SaveEmitterToYamlAs()
    {
        SaveEmitter(true);
    }

    void SaveEmitter(bool forceAskFileName)
    {
        SceneTreeDetails::SaveEmitter(GetScene(), emitterItem->effect, emitterItem->emitter,
                                      forceAskFileName, QString(),
                                      DAVA::MakeFunction(this, &ParticleEmitterContextMenu::CreateSaveCommand));
    }

    virtual Command2::Pointer CreateLoadCommand(const DAVA::String& path)
    {
        return Command2::Create<CommandLoadParticleEmitterFromYaml>(emitterItem->effect, emitterItem->emitter, path);
    }

    virtual Command2::Pointer CreateSaveCommand(const DAVA::FilePath& path)
    {
        return Command2::Create<CommandSaveParticleEmitterToYaml>(emitterItem->effect, emitterItem->emitter, path);
    }

private:
    SceneTreeItemParticleEmitter* emitterItem;
};

class SceneTree::ParticleInnerEmitterContextMenu : public SceneTree::ParticleEmitterContextMenu
{
    using TBase = ParticleEmitterContextMenu;

public:
    ParticleInnerEmitterContextMenu(SceneTreeItemParticleInnerEmitter* item, SceneTree* treeWidget)
        : TBase(item, treeWidget)
        , emitterItem(item)
    {
    }

protected:
    bool IsValidForShow() const override
    {
        return TBase::IsValidForShow() && emitterItem != nullptr;
    }

    bool IsRemovable() const override
    {
        return false;
    }

    Command2::Pointer CreateLoadCommand(const DAVA::String& path) override
    {
        emitterItem->parent->innerEmitterPath = path;
        return Command2::Create<CommandLoadInnerParticleEmitterFromYaml>(emitterItem->emitter, path);
    }

    Command2::Pointer CreateSaveCommand(const DAVA::FilePath& path) override
    {
        emitterItem->parent->innerEmitterPath = path;
        return Command2::Create<CommandSaveInnerParticleEmitterToYaml>(emitterItem->emitter, path);
    }

private:
    SceneTreeItemParticleInnerEmitter* emitterItem;
};

SceneTree::SceneTree(QWidget* parent /*= 0*/)
    : QTreeView(parent)
    , isInSync(false)
{
    Function<void()> fn(this, &SceneTree::UpdateTree);
    treeUpdater = new LazyUpdater(fn, this);

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
        treeModel->SetScene(nullptr);
    }
}

void SceneTree::SceneSelectionChanged(SceneEditor2* scene, const EntityGroup* selected, const EntityGroup* deselected)
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
        const QSignalBlocker guard(selectionModel());
        treeModel->ResyncStructure(treeModel->invisibleRootItem(), treeModel->GetScene());
        treeModel->ReloadFilter();
        filteringProxyModel->invalidate();

        SyncSelectionToTree();
        EmitParticleSignals(QItemSelection());

        if (treeModel->IsFilterSet())
        {
            ExpandFilteredItems();
        }
    }
}

void SceneTree::CommandExecuted(SceneEditor2* scene, const Command2* command, bool redo)
{
    static const Vector<int32> idsForUpdate =
    { {
    CMDID_COMPONENT_ADD,
    CMDID_COMPONENT_REMOVE,
    CMDID_INSP_MEMBER_MODIFY,
    CMDID_INSP_DYNAMIC_MODIFY,
    CMDID_ENTITY_LOCK,
    CMDID_PARTICLE_EMITTER_ADD,
    CMDID_PARTICLE_EMITTER_MOVE,
    CMDID_PARTICLE_EMITTER_REMOVE,
    CMDID_PARTICLE_LAYER_REMOVE,
    CMDID_PARTICLE_LAYER_MOVE,
    CMDID_PARTICLE_FORCE_REMOVE,
    CMDID_PARTICLE_FORCE_MOVE,
    CMDID_META_OBJ_MODIFY,
    CMDID_PARTICLE_EMITTER_LAYER_ADD,
    CMDID_PARTICLE_EMITTER_LAYER_REMOVE,
    CMDID_PARTICLE_EMITTER_LAYER_CLONE,
    CMDID_PARTICLE_EMITTER_FORCE_ADD,
    CMDID_PARTICLE_EMITTER_FORCE_REMOVE
    } };

    if (command->MatchCommandIDs(idsForUpdate))
    {
        treeUpdater->Update();
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
    SceneEditor2* sceneEditor = treeModel->GetScene();
    if (nullptr != sceneEditor)
    {
        // TODO:
        // ...
    }
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
        const QSignalBlocker guard(this);
        item->setCheckState(sceneTreeItemChecked ? Qt::Unchecked : Qt::Checked);
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
    LookAtSelection(treeModel->GetScene());
}

void SceneTree::ShowContextMenu(const QPoint& pos)
{
    QModelIndex index = filteringProxyModel->mapToSource(indexAt(pos));
    if (!index.isValid())
    {
        return;
    }

    SceneTreeItem* item = treeModel->GetItem(index);
    DVASSERT(item != nullptr);

    QPoint globalPos = mapToGlobal(pos);
    bool updateStructure = false;

    auto showMenuFn = [&](BaseContextMenu&& menu)
    {
        menu.Show(globalPos);
        if (menu.IsStructureChanged())
        {
            treeModel->ResyncStructure(treeModel->invisibleRootItem(), treeModel->GetScene());
        }
    };

    switch (item->ItemType())
    {
    case SceneTreeItem::EIT_Entity:
        showMenuFn(EntityContextMenu(static_cast<SceneTreeItemEntity*>(item), this));
        break;
    case SceneTreeItem::EIT_Emitter:
        showMenuFn(ParticleEmitterContextMenu(static_cast<SceneTreeItemParticleEmitter*>(item), this));
        break;
    case SceneTreeItem::EIT_Layer:
        showMenuFn(ParticleLayerContextMenu(static_cast<SceneTreeItemParticleLayer*>(item), this));
        break;
    case SceneTreeItem::EIT_Force:
        showMenuFn(ParticleForceContextMenu(this));
        break;
    case SceneTreeItem::EIT_InnerEmitter:
        showMenuFn(ParticleInnerEmitterContextMenu(static_cast<SceneTreeItemParticleInnerEmitter*>(item), this));
        break;
    default:
        break;
    };
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

void SceneTree::CollapseAll()
{
    QTreeView::collapseAll();
    bool needSync = false;
    {
        Guard::ScopedBoolGuard guard(isInSync, true);

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
        Guard::ScopedBoolGuard guard(isInSync, true);

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

    Guard::ScopedBoolGuard guard(isInSync, true);

    using TSelectionMap = DAVA::Map<QModelIndex, DAVA::Vector<QModelIndex>>;
    TSelectionMap toSelect;

    QModelIndex lastValidIndex;
    const auto& selection = curScene->selectionSystem->GetSelection();
    for (const auto& item : selection.GetContent())
    {
        QModelIndex sIndex = filteringProxyModel->mapFromSource(treeModel->GetIndex(item.first));
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
        Guard::ScopedBoolGuard guard(isInSync, true);

        SceneEditor2* curScene = treeModel->GetScene();
        if (nullptr != curScene)
        {
            // select items in scene
            EntityGroup group;

            QModelIndexList indexList = selectionModel()->selection().indexes();
            for (int i = 0; i < indexList.size(); ++i)
            {
                DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(treeModel->GetItem(filteringProxyModel->mapToSource(indexList[i])));
                if (entity != nullptr) // it could be emitter, etc
                {
                    group.Add(entity, curScene->selectionSystem->GetUntransformedBoundingBox(entity));
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
            if (nullptr != item)
            {
                switch (item->ItemType())
                {
                case SceneTreeItem::EIT_Entity:
                {
                    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(item);
                    if (nullptr != DAVA::GetEffectComponent(entity))
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
                    if (nullptr != itemLayer->emitter && nullptr != itemLayer->layer)
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
                    if (nullptr != layer)
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
        curScene->particlesSystem->SetEmitterSelected(nullptr, nullptr);
    if (!isParticleElements)
    {
        SceneSignals::Instance()->EmitEmitterSelected(nullptr, nullptr, nullptr);
    }
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
