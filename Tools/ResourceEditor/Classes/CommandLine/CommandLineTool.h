#ifndef __COMMAND_LINE_TOOL_H__
#define __COMMAND_LINE_TOOL_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#include "CommandLine/ProgramOptions.h"

class CommandLineTool
{
public:
    CommandLineTool(const DAVA::String& toolName);
    virtual ~CommandLineTool() = default;

    DAVA::String GetToolKey() const;

    bool ParseCommandLine(int argc, char* argv[]);
    void PrintUsage() const;
    void Process();

protected:
    virtual void ConvertOptionsToParamsInternal() = 0;
    virtual bool InitializeInternal() = 0;
    virtual void ProcessInternal() = 0;

    virtual DAVA::FilePath GetQualityConfigPath() const;
    DAVA::FilePath CreateQualityConfigPath(const DAVA::FilePath& path) const;

private:
    void PrepareEnvironment() const;
    void PrepareQualitySystem() const;
    bool Initialize();

protected:
    DAVA::ProgramOptions options;
};

#endif // __COMMAND_LINE_TOOL_H__
