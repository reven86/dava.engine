#include "REApplication.h"

#include "CommandLine/CommandLineManager.h"
#include "Logger/Logger.h"

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    CommandLineManager cmdLineMng(cmdline);
    if (cmdLineMng.IsEnabled())
    {
        return REApplication::Run(cmdLineMng);
    }
    else if (cmdline.size() == 1
#if defined(__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
             || (cmdline.size() == 3 && cmdline[1] == "-NSDocumentRevisionsDebugMode" && cmdline[2] == "YES")
#endif //#if defined (__DAVAENGINE_DEBUG__) && defined(__DAVAENGINE_MACOS__)
             )
    {
        return REApplication::Run();
    }
    else
    {
        DAVA::Logger::Error("Wrong command line. Exit on start.");
        return 1; //wrong commandLine
    }
}
