#include <QApplication>
#include "UI/mainwindow.h"

#include "EditorCore.h"

#include "Platform/Qt5/QtLayer.h"
#include "TextureCompression/PVRConverter.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "QtTools/Utils/MessageHandler.h"
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

    int returnCode = 0;
    {
        qInstallMessageHandler(DAVAMessageHandler);

        QApplication a(argc, argv);
        a.setOrganizationName("DAVA");
        a.setApplicationName("QuickEd");

        Themes::InitFromQApplication();
        Q_INIT_RESOURCE(QtToolsResources);

        QObject::connect(&a, &QApplication::applicationStateChanged, [&qtLayer](Qt::ApplicationState state) {
            state == Qt::ApplicationActive ? qtLayer.OnResume() : qtLayer.OnSuspend();
        });
        InitPVRTexTool();
        {
            // Editor Settings might be used by any singleton below during initialization, so
            // initialize it before any other one.
            EditorSettings editorSettings;

            EditorCore editorCore;

            editorCore.Start();
            returnCode = a.exec();
        }
    }
    return returnCode;
}
