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
#include "QtTools/FrameworkBinding/FrameworkLoop.h"
#include "SceneViewer.h"
#include "Library.h"
#include "SceneTree.h"
#include "PropertyPanel.h"
#include "SceneSignals.h"

#include "core_dependency_system/di_ref.hpp"
#include "core_generic_plugin/generic_plugin.hpp"
#include "core_command_system/i_command_manager.hpp"
#include "core_reflection/i_definition_manager.hpp"
#include "core_qt_common/shared_controls.hpp"

#include "core_ui_framework/i_window.hpp"
#include "core_ui_framework/i_view.hpp"
#include "core_ui_framework/i_ui_framework.hpp"
#include "core_ui_framework/i_ui_application.hpp"

#include "ReflectionBridge.h"

#include "FileSystem/Logger.h"

#include <memory>

class DAVAPlugin : public PluginMain
{
public:
    explicit DAVAPlugin(IComponentContext& context)
    {
    }

    bool PostLoad(IComponentContext & context) override
    {
        return true;
    }

    void Initialise(IComponentContext & context) override
    {
        IUIFramework * uiFramework = context.queryInterface<IUIFramework>();
        IUIApplication * uiApplication = context.queryInterface<IUIApplication>();

        if (uiFramework == nullptr)
        {
            DAVA::Logger::Error("Can't query IUIFramework interface");
            return;
        }

        if (uiApplication == nullptr)
        {
            DAVA::Logger::Error("Can't query IUIApplication interface");
            return;
        }

        IMetaTypeManager * metaTypeMng = context.queryInterface<IMetaTypeManager>();
        if (metaTypeMng == nullptr)
        {
            DAVA::Logger::Error("Can't query IMetaTypeManager interface");
            return;
        }
        Variant::setMetaTypeManager(metaTypeMng);

        IDefinitionManager* definitionMng = context.queryInterface<IDefinitionManager>();
        if (definitionMng == nullptr)
        {
            DAVA::Logger::Error("Can't query IDefinitionManager interface");
            return;
        }

        uiFramework->loadActionData(":/default/actions.xml", IUIFramework::ResourceType::File);
        mainWindow = uiFramework->createWindow(":/default/MainWindow.ui", IUIFramework::ResourceType::File);

        DAVA::Core::Run(0, nullptr);

        new DAVA::QtLayer();
        new DavaLoop();
        new FrameworkLoop();

        propertyPanel.reset(new PropertyPanel());
        propertyPanel->Initialize(*uiFramework, *uiApplication);

        sceneTree.reset(new SceneTree());
        sceneTree->Initialize(*uiFramework, *uiApplication);

        sceneWidget.reset(new SceneViewer());

        library.reset(new Library());
        library->Initialize(*uiFramework, *uiApplication);

        DVVERIFY(QObject::connect(library.get(), &Library::OpenScene, sceneWidget.get(), &SceneViewer::OnOpenScene));
        DVVERIFY(QObject::connect(sceneWidget.get(), &SceneViewer::SceneLoaded, sceneTree.get(), &SceneTree::SetScene));
        DVVERIFY(QObject::connect(SceneSignals::Instance(), &SceneSignals::SelectionChanged,
                                  sceneTree.get(), &SceneTree::OnSceneSelectionChanged));

        sceneTree->SelectedEntityChanged.Connect(sceneWidget.get(), &SceneViewer::SetSelection);
        sceneTree->SelectedEntityChanged.Connect([this](DAVA::Entity* entity)
                                                 {
            propertyPanel->SetObject(entity);
                                                 });

        uiApplication->addWindow(*mainWindow);
        uiApplication->addView(library->GetView());
        uiApplication->addView(sceneWidget->GetView());

        ICommandManager* commandMng = context.queryInterface<ICommandManager>();
        if (commandMng == nullptr)
        {
            DAVA::Logger::Error("Can't query ICommandManager interface");
            return;
        }

        undoAction = uiFramework->createAction("Undo", std::bind(&ICommandManager::undo, commandMng),
                                               std::bind(&ICommandManager::canUndo, commandMng));
        redoAction = uiFramework->createAction("Redo", std::bind(&ICommandManager::redo, commandMng),
                                               std::bind(&ICommandManager::canRedo, commandMng));
        beginBatch = uiFramework->createAction("BeginBatch", std::bind(&ICommandManager::beginBatchCommand, commandMng));
        endBatch = uiFramework->createAction("EndBatch", std::bind(&ICommandManager::endBatchCommand, commandMng));

        uiApplication->addAction(*undoAction);
        uiApplication->addAction(*redoAction);
        uiApplication->addAction(*beginBatch);
        uiApplication->addAction(*endBatch);

        mainWindow->show();

        DavaLoop::Instance()->StartLoop(FrameworkLoop::Instance());
    }

    bool Finalise(IComponentContext & context) override
    {
        propertyPanel->Finalize();
        sceneTree->Finilize();
        sceneWidget->Finalise();
        library->Finilize();

        FrameworkLoop::Instance()->Release();
        DAVA::QtLayer::Instance()->Release();
        DavaLoop::Instance()->Release();

        propertyPanel.reset();
        sceneTree.reset();
        sceneWidget.reset();
        library.reset();
        mainWindow.reset();

        return true;
    }

    void Unload(IComponentContext & context) override
    {
    }

private:
    std::unique_ptr<IWindow> mainWindow;
    std::unique_ptr<Library> library;
    std::unique_ptr<PropertyPanel> propertyPanel;
    std::unique_ptr<SceneViewer> sceneWidget;
    std::unique_ptr<SceneTree> sceneTree;

    std::unique_ptr<IAction> undoAction;
    std::unique_ptr<IAction> redoAction;
    std::unique_ptr<IAction> beginBatch;
    std::unique_ptr<IAction> endBatch;
};

PLG_CALLBACK_FUNC(DAVAPlugin)