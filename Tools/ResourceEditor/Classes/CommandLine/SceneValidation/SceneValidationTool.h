#pragma once

#include "CommandLine/Private/CommandLineModule.h"

class SceneValidationTool : public CommandLineModule
{
public:
    SceneValidationTool(const DAVA::Vector<DAVA::String>& commandLine);

    static const DAVA::String Key;

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;

    void SetAllValidationOptionsTo(bool newValue);

    DAVA::FilePath scenePath;
    DAVA::FilePath scenesListPath;

    bool validateMatrices = false;
    bool validateSameNames = false;
    bool validateCollisionTypes = false;
    bool validateTexturesRelevance = false;
    bool validateMaterialGroups = false;

    DAVA::FilePath qualityConfigPath;
};
