#include "Core/Core.h"
#include "Particles/ParticleEmitter.h"
#include "FileSystem/FileSystem.h"

#include <QApplication>
#include "UI/mainwindow.h"

#include "EditorCore.h"

#include "Platform/Qt5/QtLayer.h"
#include "TextureCompression/PVRConverter.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "QtTools/Utils/MessageHandler.h"
#include "QtTools/Utils/AssertGuard.h"
#include "NgtTools/Application/NGTApplication.h"

#include <QtGlobal>

class QEApplication : public NGTLayer::BaseApplication
{
public:
    QEApplication(int argc, char** argv)
        : BaseApplication(argc, argv)
    {
    }

    int Run()
    {
        editorCore.reset(new EditorCore());
        editorCore->Start();
        return StartApplication(editorCore->GetMainWindow());
    }

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override
    {
        names.push_back(L"plg_reflection");
        names.push_back(L"plg_variant");
        names.push_back(L"plg_command_system");
        names.push_back(L"plg_serialization");
        names.push_back(L"plg_file_system");
        names.push_back(L"plg_editor_interaction");
        names.push_back(L"plg_qt_app");
        names.push_back(L"plg_qt_common");
    }

    void OnPostLoadPlugins() override
    {
        qApp->setOrganizationName("DAVA");
        qApp->setApplicationName("QuickEd");

        BaseApplication::OnPostLoadPlugins();
    }

    bool OnRequestCloseApp() override
    {
        return editorCore->CloseProject();
    }

    void ConfigureLineCommand(NGTLayer::NGTCmdLineParser& lineParser) override
    {
        lineParser.addParam("preferenceFolder", DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory().GetAbsolutePathname() + "QuickEd/");
    }

private:
    std::unique_ptr<EditorCore> editorCore;
};

void InitPVRTexTool()
{
#if defined(__DAVAENGINE_MACOS__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif
    DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);
}

int main(int argc, char* argv[])
{
    DAVA::QtLayer qtLayer;
    DAVA::Core::Run(argc, argv);
    DAVA::Logger::Instance()->SetLogFilename("QuickEd.txt");
    DAVA::ParticleEmitter::FORCE_DEEP_CLONE = true;
    const char* settingsPath = "QuickEdSettings.archive";
    DAVA::FilePath localPrefrencesPath(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);
    int returnCode = 0;
    {
        qInstallMessageHandler(DAVAMessageHandler);
        ToolsAssetGuard::Instance()->Init();

        QEApplication a(argc, argv);
        a.LoadPlugins();

        Themes::InitFromQApplication();
        Q_INIT_RESOURCE(QtToolsResources);

        InitPVRTexTool();
        returnCode = a.Run();
    }
    return returnCode;
}
