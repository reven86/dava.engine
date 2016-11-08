#pragma once

#include "CommandLine/Private/CommandLineModule.h"

class VersionTool : public CommandLineModule
{
public:
    VersionTool(const DAVA::Vector<DAVA::String>& commandLine);

    static const DAVA::String Key;

protected:
    eFrameResult OnFrameInternal() override;
};
