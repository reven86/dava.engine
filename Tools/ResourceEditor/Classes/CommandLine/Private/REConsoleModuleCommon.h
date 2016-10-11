#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#include "CommandLine/ProgramOptions.h"

#include "TArcCore/ConsoleModule.h"

class REConsoleModuleCommon : public DAVA::TArc::ConsoleModule
{
public:
    REConsoleModuleCommon(const DAVA::Vector<DAVA::String>& commandLine, const DAVA::String& moduleName);

protected:
    virtual bool PostInitInternal();
    virtual void ShowHelpInternal();

    virtual eFrameResult OnFrameInternal();
    virtual void BeforeDestroyedInternal();

    DAVA::ProgramOptions options;

private:
    void PostInit() override;
    eFrameResult OnFrame() override;
    void BeforeDestroyed() override;

    bool isInitialized = false;
    DAVA::Vector<DAVA::String> commandLine;
};
