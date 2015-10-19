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

#include "core_generic_plugin_manager/config_plugin_loader.hpp"
#include "core_generic_plugin_manager/generic_plugin_manager.hpp"
#include "core_generic_plugin/interfaces/i_command_line_parser.hpp"
#include "core_generic_plugin/interfaces/i_application.hpp"
#include "core_generic_plugin/interfaces/i_plugin_context_manager.hpp"
#include "core_dependency_system/i_interface.hpp"

#include <QFileInfo>
#include <QString>
#include <QDir>

class CommandLineParser
: public Implements<ICommandLineParser>
{
public:
    CommandLineParser(int argc_, char** argv_)
        : m_argc(argc_)
        , m_argv(argv_)
    {
        ResolvePaths();
    }

    int argc() const override
    {
        return m_argc;
    }

    char** argv() const override
    {
        return m_argv;
    }

    const char* pluginConfigPath() const override
    {
        return configPath.c_str();
    }

    const wchar_t* pluginConfigPathW() const override
    {
        return configWPath.c_str();
    }

    std::wstring const& GetPluginsBasePath() const
    {
        return pluginsBasePath;
    }

private:
    void ResolvePaths()
    {
        QFileInfo appFileInfo(m_argv[0]);

#ifdef _WIN32
        QString pluginsBasePath_ = appFileInfo.absolutePath() + "/";
        QString pluginsPathDesc_ = pluginsBasePath_ + "plugins/plugins.txt";
#elif __APPLE__
        QString pluginsBasePath_ = appFileInfo.absolutePath() + "/../PlugIns/";
        QString pluginsPathDesc_ = pluginsBasePath_ + "plugins/plugins.txt";
#endif

        pluginsBasePath = pluginsBasePath_.toStdWString();
        configPath = pluginsPathDesc_.toStdString();
        configWPath = pluginsPathDesc_.toStdWString();
    }

private:
    int m_argc;
    char** m_argv;
    std::string configPath;
    std::wstring configWPath;
    std::wstring pluginsBasePath;
};

int main(int argc, char** argv)
{
    CommandLineParser cmdParser(argc, argv);

    int result = 1;
    std::vector<std::wstring> plugins;
    if (!ConfigPluginLoader::getPlugins(plugins, cmdParser.pluginConfigPathW()))
    {
        return result;
    }

    {
        std::transform(plugins.begin(), plugins.end(), plugins.begin(), [&cmdParser](std::wstring const& pluginPath) {
            return cmdParser.GetPluginsBasePath() + pluginPath;
        });
        GenericPluginManager pluginManager(false);
        IComponentContext* globalContext = pluginManager.getContextManager().getGlobalContext();
        globalContext->registerInterface<ICommandLineParser>(&cmdParser, false);

        pluginManager.loadPlugins(plugins);

        IApplication* application = globalContext->queryInterface<IApplication>();
        if (application != NULL)
        {
            result = application->startApplication();
        }
    }

    return result;
}