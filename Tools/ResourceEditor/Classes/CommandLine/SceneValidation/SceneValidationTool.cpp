#include "CommandLine/SceneValidation/SceneValidationTool.h"
#include "Qt/Scene/Validation/SceneValidation.h"
#include "CommandLine/OptionName.h"

using namespace DAVA;

namespace SceneValidationToolDetails
{
Vector<FilePath> ReadScenesListFile(const FilePath& listFilePath)
{
    Vector<FilePath> scenes;
    ScopedPtr<File> listFile(File::Create(listFilePath, File::OPEN | File::READ));
    if (listFile)
    {
        while (!listFile->IsEof())
        {
            String str = StringUtils::Trim(listFile->ReadLine());
            if (!str.empty())
            {
                scenes.push_back(str);
            }
        }
    }
    else
    {
        Logger::Error("Can't open scenes listfile %s", listFilePath.GetAbsolutePathname().c_str());
    }

    return scenes;
}
}

SceneValidationTool::SceneValidationTool()
    : CommandLineTool("scenevalidation")
{
    options.AddOption(OptionName::Scene, VariantType(String("")), "Path to validated scene");
    options.AddOption(OptionName::SceneList, VariantType(String("")), "Path to file with the list of validated scenes");
    options.AddOption(OptionName::Validate, VariantType(String("all")), "Validation options: all, matrices, sameNames, collisionTypes, texturesRelevance, materialGroups", true);
}

void SceneValidationTool::SetValidationOptionsTo(bool newValue)
{
    validateMatrices = newValue;
    validateSameNames = newValue;
    validateCollisionTypes = newValue;
    validateTexturesRelevance = newValue;
    validateMaterialGroups = newValue;
}

bool SceneValidationTool::AreValidationOptionsOff() const
{
    return !validateMatrices
    && !validateSameNames
    && !validateCollisionTypes
    && !validateTexturesRelevance
    && !validateMaterialGroups;
}

void SceneValidationTool::ConvertOptionsToParamsInternal()
{
    scenePath = options.GetOption(OptionName::Scene).AsString();
    scenesListPath = options.GetOption(OptionName::SceneList).AsString();

    uint32 validationOptionsCount = options.GetOptionValuesCount(OptionName::Validate);
    for (uint32 n = 0; n < validationOptionsCount; ++n)
    {
        String option = options.GetOption(OptionName::Validate, n).AsString();
        if (option == "all")
        {
            SetValidationOptionsTo(true);
            break;
        }
        else if (option == "matrices")
        {
            validateMatrices = true;
        }
        else if (option == "sameNames")
        {
            validateSameNames = true;
        }
        else if (option == "collisionTypes")
        {
            validateCollisionTypes = true;
        }
        else if (option == "texturesRelevance")
        {
            validateTexturesRelevance = true;
        }
        else if (option == "materialGroups")
        {
            validateMaterialGroups = true;
        }
        else
        {
            Logger::Error("Undefined validation option: '%s'", option.c_str());
            SetValidationOptionsTo(false);
            break;
        }
    }

    if (validationOptionsCount == 0)
    {
        Logger::Error("Any validation option should be specified");
    }
}

bool SceneValidationTool::InitializeInternal()
{
    if (scenePath.IsEmpty() && scenesListPath.IsEmpty())
    {
        Logger::Error("scene or scenesList param should be specified");
        return false;
    }

    if (!scenePath.IsEmpty() && !scenesListPath.IsEmpty())
    {
        Logger::Error("Both scene and scenesList params should not be specified");
        return false;
    }

    if (AreValidationOptionsOff())
    {
        return false;
    }

    return true;
}

void SceneValidationTool::ProcessInternal()
{
    Vector<FilePath> scenePathes;
    if (!scenesListPath.IsEmpty())
    {
        scenePathes = SceneValidationToolDetails::ReadScenesListFile(scenesListPath);
    }
    else
    {
        scenePathes.push_back(scenePath);
    }

    for (const FilePath& scenePath : scenePathes)
    {
        ScopedPtr<SceneEditor2> scene(new SceneEditor2());
        if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(scenePath))
        {
            Logger::Info("Validating scene '%s'", scenePath.GetAbsolutePathname().c_str());

            if (validateMatrices)
                SceneValidation::ValidateMatrices(scene);
            if (validateSameNames)
                SceneValidation::ValidateSameNames(scene);
            if (validateCollisionTypes)
                SceneValidation::ValidateCollisionProperties(scene);
            if (validateTexturesRelevance)
                SceneValidation::ValidateTexturesRelevance(scene);
            if (validateMaterialGroups)
            {
                new ProjectManager();
                ProjectManager::Instance()->OpenProject(ProjectManager::CreateProjectPathFromPath(scenePath));

                SceneValidation::ValidateMaterialsGroups(scene);

                ProjectManager::Instance()->CloseProject();
                ProjectManager::Instance()->Release();
            }
        }
        else
        {
            Logger::Error("Can't open scene '%s'", scenePath.GetAbsolutePathname().c_str());
        }
    }
}

DAVA::FilePath SceneValidationTool::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(scenePath);
    }

    return qualityConfigPath;
}
