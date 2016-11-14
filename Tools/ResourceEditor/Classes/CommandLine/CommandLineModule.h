#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#include "CommandLine/ProgramOptions.h"

#include "TArcCore/ConsoleModule.h"

class CommandLineModule : public DAVA::TArc::ConsoleModule
{
    friend class CommandLineModuleTestExecute;

public:
    CommandLineModule(const DAVA::Vector<DAVA::String>& commandLine, const DAVA::String& moduleName);

protected:
    void PostInit() override;
    eFrameResult OnFrame() override;
    void BeforeDestroyed() override;

    virtual bool PostInitInternal();
    virtual void ShowHelpInternal();

    virtual eFrameResult OnFrameInternal();
    virtual void BeforeDestroyedInternal();

    DAVA::Vector<DAVA::String> commandLine;
    DAVA::ProgramOptions options;

    bool isInitialized = false;
};
