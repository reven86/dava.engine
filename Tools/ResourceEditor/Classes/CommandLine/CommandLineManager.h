#ifndef __COMMAND_LINE_MANAGER_H__
#define __COMMAND_LINE_MANAGER_H__

#include "Base/BaseTypes.h"
#include "CommandLine/ProgramOptions.h"
#include "CommandLine/CommandLineTool.h"

class CommandLineManager
{
public:
    CommandLineManager(int argc, char* argv[]);
    ~CommandLineManager();

    bool IsEnabled()
    {
        return isConsoleModeEnabled;
    };
    void Process();

private:
    void CreateTools();

    void ParseCommandLine(int argc, char* argv[]);

    void PrintUsage();

    using CommandLineToolPtr = std::unique_ptr<CommandLineTool>;
    DAVA::List<CommandLineToolPtr> commandLineTools;

    CommandLineTool* activeTool = nullptr;
    bool isConsoleModeEnabled = false;
    bool helpRequested = false;

    DAVA::ProgramOptions helpOption;
};

#endif // __COMMAND_LINE_MANAGER_H__
