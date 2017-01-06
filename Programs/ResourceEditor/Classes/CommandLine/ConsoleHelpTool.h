#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"
#include "Reflection/ReflectionRegistrator.h"

class ConsoleHelpTool : public REConsoleModuleCommon
{
public:
    ConsoleHelpTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    eFrameResult OnFrameInternal() override;
    void ShowHelpInternal() override;

    DAVA_VIRTUAL_REFLECTION(ConsoleHelpTool, REConsoleModuleCommon)
    {
        DAVA::ReflectionRegistrator<ConsoleHelpTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};
