#pragma once

#include "CommandLine/CommandLineModule.h"
#include "Reflection/ReflectionRegistrator.h"

class SceneSaverTool : public CommandLineModule
{
public:
    SceneSaverTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    enum eAction : DAVA::int32
    {
        ACTION_NONE = 0,

        ACTION_SAVE,
        ACTION_RESAVE_SCENE,
        ACTION_RESAVE_YAML,
    };
    eAction commandAction = ACTION_NONE;
    DAVA::String filename;

    DAVA::FilePath inFolder;
    DAVA::FilePath outFolder;

    bool copyConverted = false;

    DAVA_VIRTUAL_REFLECTION(SceneSaverTool, CommandLineModule)
    {
        DAVA::ReflectionRegistrator<SceneSaverTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};
