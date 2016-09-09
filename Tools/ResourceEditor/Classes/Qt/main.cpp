#include "Classes/Qt/Application/REConsoleApplication.h"
#include "Classes/Qt/Application/REGuiApplication.h"

#include "CommandLine/CommandLineManager.h"
#include "Logger/Logger.h"

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    CommandLineManager cmdLineMng(cmdline);
    if (cmdLineMng.IsEnabled())
    {
        REConsoleApplication app(cmdLineMng);
        app.beforeTerminate.Connect([&cmdLineMng]()
                                    {
                                        cmdLineMng.Cleanup();
                                    });
        return app.Run();
    }
    else if (cmdline.size() == 1
#if defined(__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
             || (cmdline.size() == 3 && cmdline[1] == "-NSDocumentRevisionsDebugMode" && cmdline[2] == "YES")
#endif //#if defined (__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
             )
    {
        REGuiApplication app;
        app.beforeTerminate.Connect([&cmdLineMng]()
                                    {
                                        cmdLineMng.Cleanup();
                                    });
        return app.Run();
    }
    else
    {
        DAVA::Logger::Error("Wrong command line. Exit on start.");
        return 1; //wrong commandLine
    }
}
