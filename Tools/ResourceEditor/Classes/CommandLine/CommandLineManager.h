#pragma once

#include "Base/BaseTypes.h"
#include "CommandLine/ProgramOptions.h"
#include "CommandLine/CommandLineTool.h"

class CommandLineManager
{
public:
    CommandLineManager(const DAVA::Vector<DAVA::String>& cmdLine);
    ~CommandLineManager();

    bool IsEnabled() const
    {
        return isConsoleModeEnabled;
    };
    void Process();

private:
    void CreateTools();

    void ParseCommandLine(const DAVA::Vector<DAVA::String>& cmdLine);

    void PrintUsage();

    using CommandLineToolPtr = std::unique_ptr<CommandLineTool>;
    DAVA::List<CommandLineToolPtr> commandLineTools;

    CommandLineTool* activeTool = nullptr;
    bool isConsoleModeEnabled = false;
    bool helpRequested = false;

    DAVA::ProgramOptions helpOption;
};
