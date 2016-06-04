#include "NGTApplication.h"
#include "NgtTools/Common/GlobalContext.h"

#include "Debug/DVAssert.h"

#include <core_generic_plugin/interfaces/i_plugin_context_manager.hpp>
#include <core_generic_plugin/interfaces/i_application.hpp>
#include <core_generic_plugin/generic_plugin.hpp>
#include <core_variant/variant.hpp>
#include <core_ui_framework/i_ui_application.hpp>
#include <core_qt_common/i_qt_framework.hpp>

#include <core_qt_common/qt_window.hpp>

#include <QMainWindow>
#include <QFileInfo>

/// Hack to avoid linker errors
/// This function must be implememted if you want link with core_generic_plugin
/// In this case we need to link with core_qt_common that require linkage with core_generic_plugin
namespace wgt
{
PluginMain* createPlugin(IComponentContext& contextManager)
{
    return nullptr;
}
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

    pluginManager.getContextManager().getGlobalContext()->registerInterface<wgt::ICommandLineParser>(&commandLineParser, false /* transferOwnership*/);
    pluginManager.loadPlugins(pluginList);
    NGTLayer::SetGlobalContext(pluginManager.getContextManager().getGlobalContext());
    wgt::Variant::setMetaTypeManager(NGTLayer::queryInterface<wgt::IMetaTypeManager>());

    OnPostLoadPugins();
}

wgt::IComponentContext& BaseApplication::GetComponentContext()
{
    wgt::IComponentContext* context = pluginManager.getContextManager().getGlobalContext();
    DVASSERT(context != nullptr);
    return *context;
}

int BaseApplication::StartApplication(QMainWindow* appMainWindow)
{
    wgt::IQtFramework* framework = pluginManager.queryInterface<wgt::IQtFramework>();
    DVASSERT(framework != nullptr);

    std::unique_ptr<wgt::QtWindow> window(new wgt::QtWindow(*framework, std::unique_ptr<QMainWindow>(appMainWindow)));

    wgt::IUIApplication* app = pluginManager.queryInterface<wgt::IUIApplication>();
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