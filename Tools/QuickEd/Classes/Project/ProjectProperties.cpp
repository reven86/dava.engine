#include "ProjectProperties.h"

#include "FileSystem/YamlNode.h"
#include "FileSystem/FilePath.h"
#include "Base/Result.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"

using namespace DAVA;

std::tuple<DAVA::ResultList, ProjectProperties> ProjectProperties::ParseLegacyProperties(const DAVA::FilePath& projectFile, const YamlNode* root, int version)
{
    ResultList resultList;

    DVASSERT_MSG(version == ProjectProperties::CURRENT_PROJECT_FILE_VERSION - 1, "Supported only ");
    if (version != ProjectProperties::CURRENT_PROJECT_FILE_VERSION - 1)
    {
        String message = Format("Supported only project files with versions %d and %d.", ProjectProperties::CURRENT_PROJECT_FILE_VERSION, ProjectProperties::CURRENT_PROJECT_FILE_VERSION - 1);
        resultList.AddResult(Result::RESULT_ERROR, message);
        return std::make_tuple(resultList, ProjectProperties());
    }

    ProjectProperties props = ProjectProperties::Default();
    props.additionalResourceDirectory.relative = String("./Data/");

    const YamlNode* fontNode = root->Get("font");
    // Get font node
    if (nullptr != fontNode)
    {
        // Get default font node
        const YamlNode* defaultFontPath = fontNode->Get("DefaultFontsPath");
        if (nullptr != defaultFontPath)
        {
            String fontsConfigsPath = "./" + FilePath(defaultFontPath->AsString()).GetDirectory().GetRelativePathname("~res:/");
            props.fontsConfigsDirectory.relative = fontsConfigsPath;
        }
    }

    const YamlNode* localizationPathNode = root->Get("LocalizationPath");
    const YamlNode* localeNode = root->Get("Locale");
    if (localizationPathNode != nullptr && localeNode != nullptr)
    {
        String localePath = "./" + FilePath(localizationPathNode->AsString()).GetRelativePathname("~res:/");
        props.textsDirectory.relative = localePath;
        props.defaultLanguage = localeNode->AsString();
    }

    const YamlNode* libraryNode = root->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            String packagePath = "./" + FilePath(libraryNode->Get(i)->AsString()).GetRelativePathname("~res:/");
            props.libraryPackages.push_back({ "", packagePath });
        }
    }

    props.SetProjectFile(projectFile);

    return std::make_tuple(resultList, std::move(props));
}

void ProjectProperties::RefreshAbsolutePaths()
{
    DVASSERT(!projectFile.IsEmpty());
    projectDirectory = projectFile.GetDirectory();
    resourceDirectory.absolute = projectDirectory + resourceDirectory.relative;
    additionalResourceDirectory.absolute = projectDirectory + additionalResourceDirectory.relative;
    intermediateResourceDirectory.absolute = projectDirectory + intermediateResourceDirectory.relative;

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

ProjectProperties ProjectProperties::Default()
{
    ProjectProperties properties;

    properties.resourceDirectory.relative = "./DataSource/";
    properties.intermediateResourceDirectory.relative = "./Data/";
    properties.gfxDirectories.push_back({ ResDir{ FilePath(), String("./Gfx/") }, Size2i(960, 640) });
    properties.uiDirectory.relative = "./UI/";
    properties.fontsDirectory.relative = "./Fonts/";
    properties.fontsConfigsDirectory.relative = "./Fonts/Configs/";
    properties.textsDirectory.relative = "./Strings/";
    properties.defaultLanguage = "en";

    return properties;
}

const ProjectProperties::ResDir& ProjectProperties::GetResourceDirectory() const
{
    return resourceDirectory;
}

const ProjectProperties::ResDir& ProjectProperties::GetAdditionalResourceDirectory() const
{
    return additionalResourceDirectory;
}

const ProjectProperties::ResDir& ProjectProperties::GetIntermediateResourceDirectory() const
{
    return intermediateResourceDirectory;
}

const ProjectProperties::ResDir& ProjectProperties::GetUiDirectory() const
{
    return uiDirectory;
}

const ProjectProperties::ResDir& ProjectProperties::GetFontsDirectory() const
{
    return fontsDirectory;
}

const ProjectProperties::ResDir& ProjectProperties::GetFontsConfigsDirectory() const
{
    return fontsConfigsDirectory;
}

const ProjectProperties::ResDir& ProjectProperties::GetTextsDirectory() const
{
    return textsDirectory;
}

const Vector<ProjectProperties::GfxDir>& ProjectProperties::GetGfxDirectories() const
{
    return gfxDirectories;
}

const Vector<ProjectProperties::ResDir>& ProjectProperties::GetLibraryPackages() const
{
    return libraryPackages;
}

DAVA::FilePath ProjectProperties::MakeAbsolutePath(const DAVA::String& relPath) const
{
    if (relPath.empty())
        return FilePath();

    FilePath pathInResDir = resourceDirectory.absolute + relPath;
    if (FileSystem::Instance()->Exists(pathInResDir))
    {
        return pathInResDir;
    }

    FilePath pathInAddResDir = additionalResourceDirectory.absolute + relPath;
    if (FileSystem::Instance()->Exists(pathInAddResDir))
    {
        return pathInAddResDir;
    }

    return FilePath();
}

std::tuple<ResultList, ProjectProperties> ProjectProperties::Parse(const DAVA::FilePath& projectFile, const YamlNode* root)
{
    const YamlNode* headerNode = root->Get("Header");
    int32 version = 0;
    if (headerNode != nullptr)
    {
        const YamlNode* versionNode = headerNode->Get("version");
        if (versionNode != nullptr && versionNode->AsInt32())
        {
            version = versionNode->AsInt32();
        }
    }

    if (version != CURRENT_PROJECT_FILE_VERSION)
    {
        return ParseLegacyProperties(projectFile, root, version);
    }

    ResultList resultList;

    const YamlNode* projectPropertiesNode = root->Get("ProjectProperties");
    if (projectPropertiesNode == nullptr)
    {
        String message = Format("Wrong project properties in file %s.", projectFile.GetAbsolutePathname().c_str());
        resultList.AddResult(Result::RESULT_ERROR, message);

        return std::make_tuple(resultList, ProjectProperties());
    }

    ProjectProperties props = Default();

    const YamlNode* resourceDirNode = projectPropertiesNode->Get("ResourceDirectory");
    if (resourceDirNode != nullptr)
    {
        props.resourceDirectory.relative = resourceDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.resourceDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* additionalResourceDirNode = projectPropertiesNode->Get("AdditionalResourceDirectory");
    if (additionalResourceDirNode != nullptr)
    {
        props.additionalResourceDirectory.relative = additionalResourceDirNode->AsString();
    }

    const YamlNode* intermediateResourceDirNode = projectPropertiesNode->Get("IntermediateResourceDirectory");
    if (intermediateResourceDirNode != nullptr)
    {
        props.intermediateResourceDirectory.relative = intermediateResourceDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.intermediateResourceDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* gfxDirsNode = projectPropertiesNode->Get("GfxDirectories");
    if (gfxDirsNode != nullptr)
    {
        for (uint32 index = 0; index < gfxDirsNode->GetCount(); ++index)
        {
            const YamlNode* gfxDirNode = gfxDirsNode->Get(index);
            DVASSERT(gfxDirNode);
            String directory = gfxDirNode->Get("directory")->AsString();
            Vector2 res = gfxDirNode->Get("resolution")->AsVector2();
            Size2i resolution((int32)res.dx, (int32)res.dy);
            props.gfxDirectories.push_back({ ResDir{ FilePath(), directory }, resolution });
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s, with resolution %dx%d."
                                ,
                                props.gfxDirectories.front().directory.relative.c_str()
                                ,
                                props.gfxDirectories.front().resolution.dx
                                ,
                                props.gfxDirectories.front().resolution.dy);
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* uiDirNode = projectPropertiesNode->Get("UiDirectory");
    if (uiDirNode != nullptr)
    {
        props.uiDirectory.relative = uiDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.uiDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* fontsDirNode = projectPropertiesNode->Get("FontsDirectory");
    if (fontsDirNode != nullptr)
    {
        props.fontsDirectory.relative = fontsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.fontsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* fontsConfigsDirNode = projectPropertiesNode->Get("FontsConfigsDirectory");
    if (fontsConfigsDirNode != nullptr)
    {
        props.fontsConfigsDirectory.relative = fontsConfigsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.fontsConfigsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* textsDirNode = projectPropertiesNode->Get("TextsDirectory");
    if (textsDirNode != nullptr)
    {
        props.textsDirectory.relative = textsDirNode->AsString();
        const YamlNode* defaultLanguageNode = projectPropertiesNode->Get("DefaultLanguage");
        if (defaultLanguageNode != nullptr)
        {
            props.defaultLanguage = defaultLanguageNode->AsString();
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.textsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* libraryNode = projectPropertiesNode->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            props.libraryPackages.push_back(ResDir{ FilePath(), libraryNode->Get(i)->AsString() });
        }
    }

    props.SetProjectFile(projectFile);

    return std::make_tuple(resultList, std::move(props));
}

RefPtr<YamlNode> ProjectProperties::Emit(const ProjectProperties& props)
{
    RefPtr<YamlNode> node(YamlNode::CreateMapNode(false));

    YamlNode* headerNode(YamlNode::CreateMapNode(false));
    headerNode->Add("version", CURRENT_PROJECT_FILE_VERSION);
    node->Add("Header", headerNode);

    YamlNode* propertiesNode(YamlNode::CreateMapNode(false));
    YamlNode* resourceDirNode(YamlNode::CreateMapNode(false));
    propertiesNode->Add("ResourceDirectory", props.resourceDirectory.relative);

    if (!props.additionalResourceDirectory.relative.empty())
    {
        propertiesNode->Add("AdditionalResourceDirectory", props.additionalResourceDirectory.relative);
    }

    propertiesNode->Add("IntermediateResourceDirectory", props.intermediateResourceDirectory.relative);

    propertiesNode->Add("UiDirectory", props.uiDirectory.relative);
    propertiesNode->Add("FontsDirectory", props.fontsDirectory.relative);
    propertiesNode->Add("FontsConfigsDirectory", props.fontsConfigsDirectory.relative);
    propertiesNode->Add("TextsDirectory", props.textsDirectory.relative);
    propertiesNode->Add("DefaultLanguage", props.defaultLanguage);

    YamlNode* gfxDirsNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const auto& gfxDir : props.gfxDirectories)
    {
        YamlNode* gfxDirNode(YamlNode::CreateMapNode(false));
        gfxDirNode->Add("directory", gfxDir.directory.relative);
        Vector2 resolution((float32)gfxDir.resolution.dx, (float32)gfxDir.resolution.dy);
        gfxDirNode->Add("resolution", resolution);
        gfxDirsNode->Add(gfxDirNode);
    }
    propertiesNode->Add("GfxDirectories", gfxDirsNode);

    YamlNode* librarysNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const auto& resDir : props.libraryPackages)
    {
        librarysNode->Add(resDir.relative);
    }
    propertiesNode->Add("Library", librarysNode);

    node->Add("ProjectProperties", propertiesNode);

    return node;
}

const DAVA::String& ProjectProperties::GetProjectFileName()
{
    static const String projectFile("ui.quicked");
    return projectFile;
}

const DAVA::String& ProjectProperties::GetFontsConfigFileName()
{
    static const String configFile("fonts.yaml");
    return configFile;
}

const DAVA::FilePath& ProjectProperties::GetProjectFile() const
{
    return projectFile;
}

void ProjectProperties::SetProjectFile(const DAVA::FilePath& newProjectFile)
{
    projectFile = newProjectFile;
    RefreshAbsolutePaths();
}

const DAVA::FilePath& ProjectProperties::GetProjectDirectory() const
{
    return projectDirectory;
}
