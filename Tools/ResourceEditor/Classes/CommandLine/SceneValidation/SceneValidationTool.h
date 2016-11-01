#pragma once

#include "CommandLine/CommandLineTool.h"

class SceneValidationTool : public CommandLineTool
{
public:
    SceneValidationTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;
    DAVA::FilePath GetQualityConfigPath() const override;

    void SetValidationOptionsTo(bool newValue);
    bool AreValidationOptionsOff() const;

    FilePath scenePath;
    FilePath scenesListPath;

    bool validateMatrices = false;
    bool validateSameNames = false;
    bool validateCollisionTypes = false;
    bool validateTexturesRelevance = false;
    bool validateMaterialGroups = false;

    FilePath qualityConfigPath;
};
