#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"
#include "Reflection/ReflectionRegistrator.h"

class VersionTool : public REConsoleModuleCommon
{
public:
    VersionTool(const DAVA::Vector<DAVA::String>& commandLine);

protected:
    eFrameResult OnFrameInternal() override;

private:
    DAVA_VIRTUAL_REFLECTION(VersionTool, REConsoleModuleCommon)
    {
        DAVA::ReflectionRegistrator<VersionTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};
