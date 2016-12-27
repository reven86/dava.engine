#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#include "CommandLine/ProgramOptions.h"

#include "TArc/Core/ConsoleModule.h"

class REConsoleModuleCommon : public DAVA::TArc::ConsoleModule
{
    friend class REConsoleModuleTestUtils;

public:
    REConsoleModuleCommon(const DAVA::Vector<DAVA::String>& commandLine, const DAVA::String& moduleName);

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
