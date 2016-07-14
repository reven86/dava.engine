#include "Core/Core.h"
#include "Particles/ParticleEmitter.h"
#include "FileSystem/FileSystem.h"

#include <QApplication>

#include "EditorCore.h"
#include "QEApplication.h"

#include "Platform/Qt5/QtLayer.h"
#include "TextureCompression/PVRConverter.h"
#include "QtTools/Utils/MessageHandler.h"
#include "QtTools/Utils/AssertGuard.h"

#include <QtGlobal>

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
    {
        qInstallMessageHandler(DAVAMessageHandler);
        ToolsAssetGuard::Instance()->Init();

        QEApplication a(argc, argv);
        a.LoadPlugins();

        Q_INIT_RESOURCE(QtToolsResources);

        InitPVRTexTool();
        a.Run();
    }
    return 0;
}
