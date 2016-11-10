#include "CommandLine/SceneValidationTool.h"
#include "Qt/Scene/Validation/SceneValidation.h"
#include "Qt/Scene/Validation/ValidationOutput.h"
#include "Qt/Project/ConsoleProject.h"
#include "Qt/Project/ProjectManager.h"
#include "CommandLine/Private/OptionName.h"
#include "Utils/StringUtils.h"

namespace SceneValidationToolDetails
{
using namespace DAVA;

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

const DAVA::String SceneValidationTool::Key = "-scenevalidation";

SceneValidationTool::SceneValidationTool(const DAVA::Vector<DAVA::String>& commandLine)
    : CommandLineModule(commandLine, Key)
{
    options.AddOption(OptionName::Scene, DAVA::VariantType(DAVA::String("")), "Path to validated scene");
    options.AddOption(OptionName::SceneList, DAVA::VariantType(DAVA::String("")), "Path to file with the list of validated scenes");
    options.AddOption(OptionName::Validate, DAVA::VariantType(DAVA::String("all")), "Validation options: all, matrices, sameNames, collisionTypes, texturesRelevance, materialGroups", true);
}

void SceneValidationTool::EnableAllValidations()
{
    validateMatrices = true;
    validateSameNames = true;
    validateCollisionTypes = true;
    validateTexturesRelevance = true;
    validateMaterialGroups = true;
}

bool SceneValidationTool::PostInitInternal()
{
    scenePath = options.GetOption(OptionName::Scene).AsString();
    scenesListPath = options.GetOption(OptionName::SceneList).AsString();

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

    uint32 validationOptionsCount = options.GetOptionValuesCount(OptionName::Validate);
    if (validationOptionsCount == 0)
    {
        Logger::Error("Any validation option should be specified");
    }

    for (uint32 n = 0; n < validationOptionsCount; ++n)
    {
        DAVA::String option = options.GetOption(OptionName::Validate, n).AsString();
        if (option == "all")
        {
            EnableAllValidations();
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
            return false;
        }
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult SceneValidationTool::OnFrameInternal()
{
    using namespace DAVA;

    Vector<FilePath> scenePathes;
    if (!scenesListPath.IsEmpty())
    {
        scenePathes = SceneValidationToolDetails::ReadScenesListFile(scenesListPath);
    }
    else
    {
        scenePathes.push_back(scenePath);
    }

    new ConsoleProject;
    SCOPE_EXIT
    {
        ConsoleProject::Instance()->Release();
    };

    for (const FilePath& scenePath : scenePathes)
    {
        ConsoleProject::Instance()->OpenProject(ProjectManager::CreateProjectPathFromPath(scenePath));

        ScopedPtr<Scene> scene(new Scene);
        if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(scenePath))
        {
            Logger::Info("Validating scene '%s'", scenePath.GetAbsolutePathname().c_str());

            if (validateMatrices)
            {
                SceneValidation::LogValidationOutput output("Validating matrices");
                SceneValidation::ValidateMatrices(scene, &output);
            }

            if (validateSameNames)
            {
                SceneValidation::LogValidationOutput output("Validating same names");
                SceneValidation::ValidateSameNames(scene, &output);
            }

            if (validateCollisionTypes)
            {
                SceneValidation::LogValidationOutput output("Validating collision types");
                SceneValidation::ValidateCollisionProperties(scene, &output);
            }

            if (validateTexturesRelevance)
            {
                SceneValidation::LogValidationOutput output("Validating textures relevance");
                SceneValidation::ValidateTexturesRelevance(scene, &output);
            }

            if (validateMaterialGroups)
            {
                SceneValidation::LogValidationOutput output("Validating material groups");
                SceneValidation::ValidateMaterialsGroups(scene, &output);
            }
        }
        else
        {
            Logger::Error("Can't open scene '%s'", scenePath.GetAbsolutePathname().c_str());
        }
    }

    return eFrameResult::FINISHED;
}
