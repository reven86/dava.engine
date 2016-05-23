#include "CommandLineApplication.h"
#include "Logger/Logger.h"

CommandLineApplication::CommandLineApplication(DAVA::String name)
    : appName(name)
    , helpOption("help")
{
}

void CommandLineApplication::SetParseErrorCode(int code)
{
    codeParseError = code;
}

void CommandLineApplication::SetOkCode(int code)
{
    codeOk = code;
}

void CommandLineApplication::AddTool(std::unique_ptr<CommandLineTool> tool)
{
    tools.emplace_back(std::move(tool));
}

int CommandLineApplication::Process(uint32 argc, char* argv[])
{
    if (helpOption.Parse(argc, argv) == true)
    {
        PrintUsage();
        return codeOk;
    }
    else
    {
        for (auto& tool : tools)
        {
            bool parsed = tool->ParseOptions(argc, argv);
            if (parsed)
            {
                return tool->Process();
            }
        }

        PrintUsage();
        return codeParseError;
    }
}

void CommandLineApplication::PrintUsage()
{
    std::stringstream ss;
    ss << "Usage: " << appName << " <command>" << std::endl;
    ss << " Commands: ";

    for (const auto& tool : tools)
    {
        ss << tool->GetToolKey() << ", ";
    }
    ss << helpOption.GetCommand() << std::endl
       << std::endl;

    for (const auto& tool : tools)
    {
        ss << tool->GetUsageString();
        ss << std::endl;
    }

    DAVA::Logger::Info("%s", ss.str().c_str());
}
