#pragma once

#include "CommandLine/CommandLineModule.h"
#include "Reflection/ReflectionRegistrator.h"

class VersionTool : public CommandLineModule
{
public:
    VersionTool(const DAVA::Vector<DAVA::String>& commandLine);

protected:
    eFrameResult OnFrameInternal() override;

private:
    DAVA_VIRTUAL_REFLECTION_INPLACE(VersionTool, CommandLineModule)
    {
        DAVA::ReflectionRegistrator<VersionTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};
