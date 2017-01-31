#pragma once

#include "Classes/CommandLine/CommandLineModule.h"
#include "Classes/Qt/Scene/Validation/ValidationProgress.h"

#include <Reflection/ReflectionRegistrator.h>

class ProjectManagerData;
class SceneValidationTool : public CommandLineModule
{
public:
    SceneValidationTool(const DAVA::Vector<DAVA::String>& commandLine);

    static const DAVA::String Key;

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;

    ProjectManagerData* GetProjectManagerData();

    void UpdateResult(DAVA::Result);
    void EnableAllValidations();

    DAVA::FilePath scenePath;
    DAVA::FilePath scenesListPath;

    bool validateMatrices = false;
    bool validateSameNames = false;
    bool validateCollisionTypes = false;
    bool validateTexturesRelevance = false;
    bool validateMaterialGroups = false;

    DAVA::FilePath qualityConfigPath;

    DAVA_VIRTUAL_REFLECTION(SceneValidationTool, CommandLineModule)
    {
        DAVA::ReflectionRegistrator<SceneValidationTool>::Begin()
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};
