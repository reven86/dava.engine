#pragma once

#include "Base/BaseTypes.h"
#include "CommandLine/CommandLineModule.h"
#include "Utils/Dump/SceneDumper.h"
#include "Reflection/ReflectionRegistrator.h"

class DumpTool : public CommandLineModule
{
public:
    DumpTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,
        ACTION_DUMP_LINKS
    };
    eAction commandAction = ACTION_NONE;
    DAVA::String filename;
    DAVA::FilePath inFolder;
    DAVA::FilePath outFile;

    DAVA::Vector<DAVA::eGPUFamily> compressedGPUs;
    SceneDumper::eMode mode = SceneDumper::eMode::REQUIRED;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DumpTool, CommandLineModule)
    {
        DAVA::ReflectionRegistrator<DumpTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};
