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

#include "core_generic_plugin/generic_plugin.hpp"

#include "core_ui_framework/i_window.hpp"
#include "core_ui_framework/i_view.hpp"
#include "core_ui_framework/i_ui_framework.hpp"
#include "core_ui_framework/i_ui_application.hpp"

#include "FileSystem/Logger.h"

#include <memory>

class DAVAPlugin : public PluginMain
{
public:
    explicit DAVAPlugin(IComponentContext & context) {}

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

        uiFramework->loadActionData(":/default/actions.xml", IUIFramework::ResourceType::File);
        mainWindow = uiFramework->createWindow(":/default/MainWindow.ui", IUIFramework::ResourceType::File);

        DAVA::Core::Run(0, nullptr);

        new DAVA::QtLayer();
        new DavaLoop();
        new FrameworkLoop();

        sceneTree.reset(new SceneTree());
        sceneTree->Initialize(*uiFramework, *uiApplication);

        sceneWidget.reset(new SceneViewer());
        
        library.reset(new Library());
        library->Initialize(*uiFramework, *uiApplication);

        DVVERIFY(QObject::connect(library.get(), &Library::OpenScene, sceneWidget.get(), &SceneViewer::OnOpenScene));
        DVVERIFY(QObject::connect(sceneWidget.get(), &SceneViewer::SceneLoaded, sceneTree.get(), &SceneTree::SetScene));

        uiApplication->addWindow(*mainWindow);
        uiApplication->addView(library->GetView());
        uiApplication->addView(sceneWidget->GetView());
        
        mainWindow->show();

        DavaLoop::Instance()->StartLoop(FrameworkLoop::Instance());
    }

    bool Finalise(IComponentContext & context) override
    {
        sceneTree->Finilize();
        sceneWidget->Finalise();
        library->Finilize();

        FrameworkLoop::Instance()->Release();
        DAVA::QtLayer::Instance()->Release();
        DavaLoop::Instance()->Release();

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
    std::unique_ptr<SceneViewer> sceneWidget;
    std::unique_ptr<SceneTree> sceneTree;
};

PLG_CALLBACK_FUNC(DAVAPlugin)