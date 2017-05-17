#pragma once

#include "Classes/CommandLine/CommandLineModule.h"

class DuplicateObjectTool : public CommandLineModule
{
public:
    DuplicateObjectTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    DAVA::FilePath filePath;
    DAVA::FilePath outDirPath;
    DAVA::uint32 count = 1;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DuplicateObjectTool, CommandLineModule)
    {
        DAVA::ReflectionRegistrator<DuplicateObjectTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};

class RandomPlaceHingedEquip : public CommandLineModule
{
public:
    RandomPlaceHingedEquip(const DAVA::Vector<DAVA::String>& commandLine);

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    DAVA::Vector<DAVA::FilePath> scenesPath;
    DAVA::FilePath projectRootFolder;
    DAVA::FilePath hindedEquipLibrary;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(RandomPlaceHingedEquip, CommandLineModule)
    {
        DAVA::ReflectionRegistrator<RandomPlaceHingedEquip>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};