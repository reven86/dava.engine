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

#include "NGTApplication.h"
#include "NgtTools/Common/GlobalContext.h"

#include "Debug/DVAssert.h"

#include "core_generic_plugin/interfaces/i_plugin_context_manager.hpp"
#include "core_generic_plugin/interfaces/i_application.hpp"
#include "core_generic_plugin/generic_plugin.hpp"
#include "core_variant/variant.hpp"
#include "core_ui_framework/i_ui_application.hpp"
#include "core_qt_common/i_qt_framework.hpp"

#include "core_qt_common/qt_window.hpp"

#include <QMainWindow>
#include <QFileInfo>

/// Hack to avoid linker errors
/// This function must be implememted if you want link with core_generic_plugin
/// In this case we need to link with core_qt_common that require linkage with core_generic_plugin
PluginMain* createPlugin(IComponentContext& contextManager)
{
    return nullptr;
}

namespace NGTLayer
{
BaseApplication::BaseApplication(int argc, char** argv)
    : pluginManager(false)
    , commandLineParser(argc, argv)
{
}

BaseApplication::~BaseApplication()
{
    OnPreUnloadPlugins();
    NGTLayer::SetGlobalContext(nullptr);
}

void BaseApplication::LoadPlugins()
{
    DAVA::Vector<DAVA::WideString> pluginList;
    GetPluginsForLoad(pluginList);

    DAVA::WideString plugindFolder = GetPluginsFolder();

    std::transform(pluginList.begin(), pluginList.end(), pluginList.begin(), [&plugindFolder](std::wstring const& pluginPath)
                   {
                       return plugindFolder + pluginPath;
                   });

    pluginManager.getContextManager().getGlobalContext()->registerInterface<ICommandLineParser>(&commandLineParser, false /* transferOwnership*/);
    pluginManager.loadPlugins(pluginList);
    NGTLayer::SetGlobalContext(pluginManager.getContextManager().getGlobalContext());
    Variant::setMetaTypeManager(NGTLayer::queryInterface<IMetaTypeManager>());

    OnPostLoadPugins();
}

IComponentContext& BaseApplication::GetComponentContext()
{
    IComponentContext* context = pluginManager.getContextManager().getGlobalContext();
    DVASSERT(context != nullptr);
    return *context;
}

int BaseApplication::StartApplication(QMainWindow* appMainWindow)
{
    IQtFramework* framework = pluginManager.queryInterface<IQtFramework>();
    DVASSERT(framework != nullptr);

    std::unique_ptr<QtWindow> window(new QtWindow(*framework, std::unique_ptr<QMainWindow>(appMainWindow)));

    IUIApplication* app = pluginManager.queryInterface<IUIApplication>();
    DVASSERT(app != nullptr);
    window->show();
    app->addWindow(*window);
    int result = app->startApplication();
    app->removeWindow(*window);
    window->releaseWindow();

    return result;
}

DAVA::WideString BaseApplication::GetPluginsFolder() const
{
    QFileInfo appFileInfo(commandLineParser.argv()[0]);

#if defined(Q_OS_WIN)
    QString pluginsBasePath_ = appFileInfo.absolutePath() + "/plugins/";
#elif defined(Q_OS_OSX)
    QString pluginsBasePath_ = appFileInfo.absolutePath() + "/../PlugIns/plugins/";
#endif

    return pluginsBasePath_.toStdWString();
}
} // namespace NGTLayer