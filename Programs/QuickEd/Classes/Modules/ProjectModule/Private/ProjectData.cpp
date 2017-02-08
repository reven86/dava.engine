#include "Modules/ProjectModule/ProjectData.h"

#include <Base/Result.h>
#include <Engine/Engine.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/YamlNode.h>
#include <Utils/Utils.h>
#include <Utils/StringFormat.h>

using namespace DAVA;

const char* ProjectData::projectPathPropertyName = "ProjectPath";
const char* ProjectData::uiDirectoryPropertyName = "UI directory";

std::tuple<std::unique_ptr<ProjectData>, ResultList> ProjectData::ParseLegacyProperties(const DAVA::FilePath& projectFile, const YamlNode* root, int version)
{
    ResultList resultList;

    DVASSERT(version == ProjectData::CURRENT_PROJECT_FILE_VERSION - 1, "Supported only ");
    if (version != ProjectData::CURRENT_PROJECT_FILE_VERSION - 1)
    {
        String message = Format("Supported only project files with versions %d and %d.", ProjectData::CURRENT_PROJECT_FILE_VERSION, ProjectData::CURRENT_PROJECT_FILE_VERSION - 1);
        resultList.AddResult(Result::RESULT_ERROR, message);
        return std::make_tuple(std::unique_ptr<ProjectData>(), resultList);
    }

    std::unique_ptr<ProjectData> data = ProjectData::Default();
    data->additionalResourceDirectory.relative = String("./Data/");

    if (root == nullptr) // for support old project
    {
        data->SetDefaultLanguage("");
    }
    else
    {
        const YamlNode* fontNode = root->Get("font");
        // Get font node
        if (nullptr != fontNode)
        {
            // Get default font node
            const YamlNode* defaultFontPath = fontNode->Get("DefaultFontsPath");
            if (nullptr != defaultFontPath)
            {
                String fontsConfigsPath = FilePath(defaultFontPath->AsString()).GetDirectory().GetRelativePathname("~res:/");
                data->fontsConfigsDirectory.relative = fontsConfigsPath;
            }
        }

        const YamlNode* localizationPathNode = root->Get("LocalizationPath");
        const YamlNode* localeNode = root->Get("Locale");
        if (localizationPathNode != nullptr && localeNode != nullptr)
        {
            String localePath = FilePath(localizationPathNode->AsString()).GetRelativePathname("~res:/");
            data->textsDirectory.relative = localePath;
            data->defaultLanguage = localeNode->AsString();
        }

        const YamlNode* libraryNode = root->Get("Library");
        if (libraryNode != nullptr)
        {
            for (uint32 i = 0; i < libraryNode->GetCount(); i++)
            {
                String packagePath = FilePath(libraryNode->Get(i)->AsString()).GetRelativePathname("~res:/");
                data->libraryPackages.push_back({ "", packagePath });
            }
        }
    }

    data->SetProjectFile(projectFile);

    return std::make_tuple(std::move(data), resultList);
}

void ProjectData::RefreshAbsolutePaths()
{
    DVASSERT(!projectFile.IsEmpty());
    projectDirectory = projectFile.GetDirectory();
    resourceDirectory.absolute = projectDirectory + resourceDirectory.relative;
    if (additionalResourceDirectory.relative.empty())
    {
        additionalResourceDirectory.absolute = FilePath();
    }
    else
    {
        additionalResourceDirectory.absolute = projectDirectory + additionalResourceDirectory.relative;
    }

    convertedResourceDirectory.absolute = projectDirectory + convertedResourceDirectory.relative;

    uiDirectory.absolute = MakeAbsolutePath(uiDirectory.relative);
    fontsDirectory.absolute = MakeAbsolutePath(fontsDirectory.relative);
    fontsConfigsDirectory.absolute = MakeAbsolutePath(fontsConfigsDirectory.relative);
    textsDirectory.absolute = MakeAbsolutePath(textsDirectory.relative);

    for (auto& gfxDir : gfxDirectories)
    {
        gfxDir.directory.absolute = MakeAbsolutePath(gfxDir.directory.relative);
    }

    for (auto& resDir : libraryPackages)
    {
        resDir.absolute = MakeAbsolutePath(resDir.relative);
    }
}

std::unique_ptr<ProjectData> ProjectData::Default()
{
    std::unique_ptr<ProjectData> data(new ProjectData());

    data->resourceDirectory.relative = "./DataSource/";
    data->convertedResourceDirectory.relative = "./Data/";
    data->gfxDirectories.push_back({ ResDir{ FilePath(), String("./Gfx/") }, Size2i(960, 640) });
    data->uiDirectory.relative = "./UI/";
    data->fontsDirectory.relative = "./Fonts/";
    data->fontsConfigsDirectory.relative = "./Fonts/Configs/";
    data->textsDirectory.relative = "./Strings/";
    data->defaultLanguage = "en";

    return data;
}

const ProjectData::ResDir& ProjectData::GetResourceDirectory() const
{
    return resourceDirectory;
}

const ProjectData::ResDir& ProjectData::GetAdditionalResourceDirectory() const
{
    return additionalResourceDirectory;
}

const ProjectData::ResDir& ProjectData::GetConvertedResourceDirectory() const
{
    return convertedResourceDirectory;
}

const ProjectData::ResDir& ProjectData::GetUiDirectory() const
{
    return uiDirectory;
}

const ProjectData::ResDir& ProjectData::GetFontsDirectory() const
{
    return fontsDirectory;
}

const ProjectData::ResDir& ProjectData::GetFontsConfigsDirectory() const
{
    return fontsConfigsDirectory;
}

const ProjectData::ResDir& ProjectData::GetTextsDirectory() const
{
    return textsDirectory;
}

const Vector<ProjectData::GfxDir>& ProjectData::GetGfxDirectories() const
{
    return gfxDirectories;
}

const Vector<ProjectData::ResDir>& ProjectData::GetLibraryPackages() const
{
    return libraryPackages;
}

const Map<String, DAVA::Set<FastName>>& ProjectData::GetPrototypes() const
{
    return prototypes;
}

DAVA::FilePath ProjectData::MakeAbsolutePath(const DAVA::String& relPath) const
{
    if (relPath.empty())
        return FilePath();

    const DAVA::EngineContext* engineContext = GetEngineContext();
    DAVA::FileSystem* fileSystem = engineContext->fileSystem;

    FilePath pathInResDir = resourceDirectory.absolute + relPath;
    if (fileSystem->Exists(pathInResDir))
    {
        return pathInResDir;
    }

    FilePath pathInAddResDir = additionalResourceDirectory.absolute + relPath;
    if (fileSystem->Exists(pathInAddResDir))
    {
        return pathInAddResDir;
    }

    return FilePath();
}

std::tuple<std::unique_ptr<ProjectData>, ResultList> ProjectData::Parse(const DAVA::FilePath& projectFile, const YamlNode* root)
{
    int32 version = 0;
    if (root != nullptr)
    {
        const YamlNode* headerNode = root->Get("Header");
        if (headerNode != nullptr)
        {
            const YamlNode* versionNode = headerNode->Get("version");
            if (versionNode != nullptr && versionNode->AsInt32())
            {
                version = versionNode->AsInt32();
            }
        }
    }

    if (version != CURRENT_PROJECT_FILE_VERSION)
    {
        return ParseLegacyProperties(projectFile, root, version);
    }

    ResultList resultList;

    const YamlNode* ProjectDataNode = root->Get("ProjectProperties");
    if (ProjectDataNode == nullptr)
    {
        String message = Format("Wrong project properties in file %s.", projectFile.GetAbsolutePathname().c_str());
        resultList.AddResult(Result::RESULT_ERROR, message);

        return std::make_tuple(std::unique_ptr<ProjectData>(), resultList);
    }

    std::unique_ptr<ProjectData> data = Default();

    const YamlNode* resourceDirNode = ProjectDataNode->Get("ResourceDirectory");
    if (resourceDirNode != nullptr)
    {
        data->resourceDirectory.relative = resourceDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directory not set. Used default directory: %s.", data->resourceDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* additionalResourceDirNode = ProjectDataNode->Get("AdditionalResourceDirectory");
    if (additionalResourceDirNode != nullptr)
    {
        data->additionalResourceDirectory.relative = additionalResourceDirNode->AsString();
    }

    const YamlNode* convertedResourceDirNode = ProjectDataNode->Get("ConvertedResourceDirectory");
    if (convertedResourceDirNode != nullptr)
    {
        data->convertedResourceDirectory.relative = convertedResourceDirNode->AsString();
    }
    else
    {
        String message = Format("Directory for converted sources not set. Used default directory: %s.", data->convertedResourceDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* gfxDirsNode = ProjectDataNode->Get("GfxDirectories");
    if (gfxDirsNode != nullptr)
    {
        for (uint32 index = 0; index < gfxDirsNode->GetCount(); ++index)
        {
            const YamlNode* gfxDirNode = gfxDirsNode->Get(index);
            DVASSERT(gfxDirNode);
            String directory = gfxDirNode->Get("directory")->AsString();
            Vector2 res = gfxDirNode->Get("resolution")->AsVector2();
            Size2i resolution((int32)res.dx, (int32)res.dy);
            data->gfxDirectories.push_back({ ResDir{ FilePath(), directory }, resolution });
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s, with resolution %dx%d.",
                                data->gfxDirectories.front().directory.relative.c_str(),
                                data->gfxDirectories.front().resolution.dx,
                                data->gfxDirectories.front().resolution.dy);
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* uiDirNode = ProjectDataNode->Get("UiDirectory");
    if (uiDirNode != nullptr)
    {
        data->uiDirectory.relative = uiDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", data->uiDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* fontsDirNode = ProjectDataNode->Get("FontsDirectory");
    if (fontsDirNode != nullptr)
    {
        data->fontsDirectory.relative = fontsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", data->fontsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* fontsConfigsDirNode = ProjectDataNode->Get("FontsConfigsDirectory");
    if (fontsConfigsDirNode != nullptr)
    {
        data->fontsConfigsDirectory.relative = fontsConfigsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", data->fontsConfigsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* textsDirNode = ProjectDataNode->Get("TextsDirectory");
    if (textsDirNode != nullptr)
    {
        data->textsDirectory.relative = textsDirNode->AsString();
        const YamlNode* defaultLanguageNode = ProjectDataNode->Get("DefaultLanguage");
        if (defaultLanguageNode != nullptr)
        {
            data->defaultLanguage = defaultLanguageNode->AsString();
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", data->textsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* libraryNode = ProjectDataNode->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            data->libraryPackages.push_back(ResDir{ FilePath(), libraryNode->Get(i)->AsString() });
        }
    }

    const YamlNode* prototypesNode = ProjectDataNode->Get("Prototypes");
    if (prototypesNode != nullptr)
    {
        for (uint32 i = 0; i < prototypesNode->GetCount(); i++)
        {
            Set<FastName> packagePrototypes;
            const YamlNode* packNode = prototypesNode->Get(i);
            const YamlNode* packagePrototypesNode = packNode->Get("prototypes");

            for (uint32 j = 0; j < packagePrototypesNode->GetCount(); j++)
            {
                packagePrototypes.insert(packagePrototypesNode->Get(j)->AsFastName());
            }

            const String& packagePath = packNode->Get("file")->AsString();
            data->prototypes[packagePath] = packagePrototypes;
        }
    }

    data->SetProjectFile(projectFile);

    return std::make_tuple(std::move(data), resultList);
}

RefPtr<YamlNode> ProjectData::SerializeToYamlNode(const ProjectData* data)
{
    RefPtr<YamlNode> node(YamlNode::CreateMapNode(false));

    YamlNode* headerNode(YamlNode::CreateMapNode(false));
    headerNode->Add("version", CURRENT_PROJECT_FILE_VERSION);
    node->Add("Header", headerNode);

    YamlNode* propertiesNode(YamlNode::CreateMapNode(false));
    propertiesNode->Add("ResourceDirectory", data->resourceDirectory.relative);

    if (!data->additionalResourceDirectory.relative.empty())
    {
        propertiesNode->Add("AdditionalResourceDirectory", data->additionalResourceDirectory.relative);
    }

    propertiesNode->Add("IntermediateResourceDirectory", data->convertedResourceDirectory.relative);

    propertiesNode->Add("UiDirectory", data->uiDirectory.relative);
    propertiesNode->Add("FontsDirectory", data->fontsDirectory.relative);
    propertiesNode->Add("FontsConfigsDirectory", data->fontsConfigsDirectory.relative);
    propertiesNode->Add("TextsDirectory", data->textsDirectory.relative);
    propertiesNode->Add("DefaultLanguage", data->defaultLanguage);

    YamlNode* gfxDirsNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const auto& gfxDir : data->gfxDirectories)
    {
        YamlNode* gfxDirNode(YamlNode::CreateMapNode(false));
        gfxDirNode->Add("directory", gfxDir.directory.relative);
        Vector2 resolution((float32)gfxDir.resolution.dx, (float32)gfxDir.resolution.dy);
        gfxDirNode->Add("resolution", resolution);
        gfxDirsNode->Add(gfxDirNode);
    }
    propertiesNode->Add("GfxDirectories", gfxDirsNode);

    YamlNode* librarysNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const auto& resDir : data->libraryPackages)
    {
        librarysNode->Add(resDir.relative);
    }
    propertiesNode->Add("Library", librarysNode);

    node->Add("ProjectData", propertiesNode);

    return node;
}

const DAVA::String& ProjectData::GetProjectFileName()
{
    static const String projectFile("ui.quicked");
    return projectFile;
}

const DAVA::String& ProjectData::GetFontsConfigFileName()
{
    static const String configFile("fonts.yaml");
    return configFile;
}

const DAVA::FilePath& ProjectData::GetProjectFile() const
{
    return projectFile;
}

void ProjectData::SetProjectFile(const DAVA::FilePath& newProjectFile)
{
    projectFile = newProjectFile;
    RefreshAbsolutePaths();
}

const DAVA::FilePath& ProjectData::GetProjectDirectory() const
{
    return projectDirectory;
}
