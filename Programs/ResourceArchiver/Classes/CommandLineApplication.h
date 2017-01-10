#pragma once

#include "Base/BaseTypes.h"
#include "CommandLine/ProgramOptions.h"
#include "CommandLineTool.h"

// the idea is to place CommandLineApplication and CommandLineTool into Sources/Tools in near future,
// combine them with CommandToolManager, CommandLineTool of ResourceEditor/Classes/CommandLine/
// and use that classes througout all our command line tools
class CommandLineApplication
{
public:
    CommandLineApplication(DAVA::String appName);
    void SetParseErrorCode(int errorCode);
    void SetOkCode(int errorCode);
    void AddTool(std::unique_ptr<CommandLineTool> tool); // todo : probably use multiple inheritance instead of AddTool()

    int Process(const DAVA::Vector<DAVA::String>& cmdline);
    int Process(DAVA::uint32 argc, char* argv[]);

private:
    void PrintUsage();

    const DAVA::String appName;
    int codeParseError = -1;
    int codeOk = 0;
    DAVA::Vector<std::unique_ptr<CommandLineTool>> tools;
    DAVA::ProgramOptions helpOption;
};
