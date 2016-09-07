#include "Core/Core.h"
#include "Particles/ParticleEmitter.h"
#include "FileSystem/FileSystem.h"

#include "EditorCore.h"

#include "TextureCompression/PVRConverter.h"
#include "QtTools/Utils/MessageHandler.h"
#include "QtTools/Utils/AssertGuard.h"
#include "QtTools/Utils/Themes/Themes.h"

#include <QtGlobal>
#include <QApplication>

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
    DAVA::Logger::Instance()->SetLogFilename("QuickEd.txt");
    DAVA::ParticleEmitter::FORCE_DEEP_CLONE = true;
    const char* settingsPath = "QuickEdSettings.archive";
    DAVA::FilePath localPrefrencesPath(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);
    int retCode = 0;
    {
        qInstallMessageHandler(DAVAMessageHandler);
        ToolsAssetGuard::Instance()->Init();
        QApplication a(argc, argv);
        qApp->setOrganizationName("DAVA");
        qApp->setApplicationName("QuickEd");

        const char* settingsPath = "QuickEdSettings.archive";
        DAVA::FilePath localPrefrencesPath(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory() + settingsPath);
        PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

        Q_INIT_RESOURCE(QtToolsResources);
        InitPVRTexTool();

        Themes::InitFromQApplication();

        EditorCore editorCore;
        editorCore.Start();

        retCode = a.exec();
    }
    return retCode;
}
