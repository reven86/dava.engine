#pragma once

#include "CommandLine/CommandLineModule.h"
#include "Reflection/ReflectionRegistrator.h"

class ConsoleHelpTool : public CommandLineModule
{
public:
    ConsoleHelpTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    eFrameResult OnFrameInternal() override;
    void ShowHelpInternal() override;

    DAVA_VIRTUAL_REFLECTION_INPLACE(ConsoleHelpTool, CommandLineModule)
    {
        DAVA::ReflectionRegistrator<ConsoleHelpTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};
