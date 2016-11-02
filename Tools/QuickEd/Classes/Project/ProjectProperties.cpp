#include "ProjectProperties.h"

#include "FileSystem/YamlNode.h"
#include "FileSystem/FilePath.h"
#include "Base/Result.h"
#include "FileSystem/FileSystem.h"

using namespace DAVA;

namespace ProjectPropertiesDetails
{
std::tuple<DAVA::ResultList, ProjectProperties> ParseLegacyProjectProperties(const DAVA::FilePath& projectFile, const YamlNode* root, int version)
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
    props.projectFile = projectFile;
    props.additionalResourceDirectory = String("./Data/");

    const YamlNode* fontNode = root->Get("font");
    // Get font node
    if (nullptr != fontNode)
    {
        // Get default font node
        const YamlNode* defaultFontPath = fontNode->Get("DefaultFontsPath");
        if (nullptr != defaultFontPath)
        {
            FilePath localizationFontsPath(defaultFontPath->AsString());
            if (FileSystem::Instance()->Exists(localizationFontsPath))
            {
                props.fontsConfigsDirectory = localizationFontsPath.GetDirectory().GetStringValue();
            }
        }
    }

    const YamlNode* localizationPathNode = root->Get("LocalizationPath");
    const YamlNode* localeNode = root->Get("Locale");
    if (localizationPathNode != nullptr && localeNode != nullptr)
    {
        FilePath localePath = localizationPathNode->AsString();
        props.textsDirectory = localePath.GetAbsolutePathname();
        props.defaultLanguage = localeNode->AsString();
    }

    const YamlNode* libraryNode = root->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            props.libraryPackages.push_back(libraryNode->Get(i)->AsString());
        }
    }

    return std::make_tuple(resultList, std::move(props));
}
}

ProjectProperties ProjectProperties::Default()
{
    ProjectProperties properties;

    properties.resourceDirectory = "./DataSource/";
    properties.intermediateResourceDirectory = "./Data/";
    properties.gfxDirectories.push_back(std::make_pair(String("./Gfx/"), Vector2(960, 640)));
    properties.uiDirectory = "./UI/";
    properties.fontsDirectory = "./Fonts/";
    properties.fontsConfigsDirectory = "./Fonts/Configs/";
    properties.textsDirectory = "./Strings/";
    properties.defaultLanguage = "en";

    return properties;
}

FilePath ProjectProperties::GetResourceDirectory() const
{
    return FilePath(projectFile.GetDirectory()) + resourceDirectory;
}

FilePath ProjectProperties::GetAdditionalResourceDirectory() const
{
    return FilePath(projectFile.GetDirectory()) + additionalResourceDirectory;
}

FilePath ProjectProperties::GetIntermediateResourceDirectory() const
{
    return FilePath(projectFile.GetDirectory()) + intermediateResourceDirectory;
}

FilePath ProjectProperties::GetUiDirectory(bool useAdditionalResDir /*= false*/) const
{
    return GetAbsolutePath(uiDirectory, useAdditionalResDir);
}

FilePath ProjectProperties::GetFontsDirectory(bool useAdditionalResDir /*= false*/) const
{
    return GetAbsolutePath(fontsDirectory, useAdditionalResDir);
}

FilePath ProjectProperties::GetFontsConfigsDirectory(bool useAdditionalResDir /*= false*/) const
{
    return GetAbsolutePath(fontsConfigsDirectory, useAdditionalResDir);
}

FilePath ProjectProperties::GetTextsDirectory(bool useAdditionalResDir /*= false*/) const
{
    return GetAbsolutePath(textsDirectory, useAdditionalResDir);
}

Vector<FilePath> ProjectProperties::GetGfxDirectories(bool useAdditionalResDir /*= false*/) const
{
    return Vector<FilePath>();
}

Vector<FilePath> ProjectProperties::GetLibraryPackages(bool useAdditionalResDir /*= false*/) const
{
    return Vector<FilePath>();
}

DAVA::FilePath ProjectProperties::GetAbsolutePath(const DAVA::String& relPath, bool useAdditionalResDir) const
{
    if (relPath.empty())
        return FilePath();

    const String directory = useAdditionalResDir ? resourceDirectory : additionalResourceDirectory;

    return FilePath(projectFile.GetDirectory()) + directory + relPath;
}

std::tuple<ResultList, ProjectProperties> ProjectProperties::Parse(const DAVA::FilePath& projectFile, const YamlNode* root)
{
    using namespace ProjectPropertiesDetails;

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
        return ParseLegacyProjectProperties(projectFile, root, version);
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
    props.projectFile = projectFile;

    const YamlNode* resourceDirNode = projectPropertiesNode->Get("ResourceDirectory");
    if (resourceDirNode != nullptr)
    {
        props.resourceDirectory = resourceDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.resourceDirectory.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* additionalResourceDirNode = projectPropertiesNode->Get("AdditionalResourceDirectory");
    if (additionalResourceDirNode != nullptr)
    {
        props.additionalResourceDirectory = additionalResourceDirNode->AsString();
    }

    const YamlNode* intermediateResourceDirNode = projectPropertiesNode->Get("IntermediateResourceDirectory");
    if (intermediateResourceDirNode != nullptr)
    {
        props.intermediateResourceDirectory = intermediateResourceDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.intermediateResourceDirectory.c_str());
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
            Vector2 resolution = gfxDirNode->Get("resolution")->AsVector2();
            props.gfxDirectories.push_back(std::make_pair(directory, resolution));
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s, with resolution %dx%d."
                                ,
                                props.gfxDirectories.front().first.c_str()
                                ,
                                props.gfxDirectories.front().second.dy
                                ,
                                props.gfxDirectories.front().second.dy);
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* uiDirNode = projectPropertiesNode->Get("UiDirectory");
    if (uiDirNode != nullptr)
    {
        props.uiDirectory = uiDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.uiDirectory.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* fontsDirNode = projectPropertiesNode->Get("FontsDirectory");
    if (fontsDirNode != nullptr)
    {
        props.fontsDirectory = fontsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.fontsDirectory.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* fontsConfigsDirNode = projectPropertiesNode->Get("FontsConfigsDirectory");
    if (fontsConfigsDirNode != nullptr)
    {
        props.fontsConfigsDirectory = fontsConfigsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.fontsConfigsDirectory.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* textsDirNode = projectPropertiesNode->Get("TextsDirectory");
    if (textsDirNode != nullptr)
    {
        props.textsDirectory = textsDirNode->AsString();
        const YamlNode* defaultLanguageNode = projectPropertiesNode->Get("DefaultLanguage");
        if (defaultLanguageNode != nullptr)
        {
            props.defaultLanguage = defaultLanguageNode->AsString();
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", props.textsDirectory.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* libraryNode = projectPropertiesNode->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            props.libraryPackages.push_back(libraryNode->Get(i)->AsString());
        }
    }

    return std::make_tuple(resultList, std::move(props));
}

RefPtr<YamlNode> ProjectProperties::Emit(const ProjectProperties& props)
{
    RefPtr<YamlNode> node(YamlNode::CreateMapNode(false));

    YamlNode* headerNode(YamlNode::CreateMapNode(false));
    headerNode->Add("Version", CURRENT_PROJECT_FILE_VERSION);
    node->Add("Header", headerNode);

    YamlNode* propertiesNode(YamlNode::CreateMapNode(false));
    YamlNode* resourceDirNode(YamlNode::CreateMapNode(false));
    propertiesNode->Add("ResourceDirectory", props.resourceDirectory);

    if (!props.additionalResourceDirectory.empty())
    {
        propertiesNode->Add("AdditionalResourceDirectory", props.additionalResourceDirectory);
    }

    propertiesNode->Add("IntermediateResourceDirectory", props.intermediateResourceDirectory);

    YamlNode* gfxDirsNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const auto& gfxDir : props.gfxDirectories)
    {
        YamlNode* gfxDirNode(YamlNode::CreateMapNode(false));
        gfxDirNode->Add("directory", gfxDir.first);
        const Vector2& resolution = gfxDir.second;
        gfxDirNode->Add("resolution", resolution);
    }
    propertiesNode->Add("GfxDirectories", gfxDirsNode);

    propertiesNode->Add("UiDirectory", props.uiDirectory);
    propertiesNode->Add("FontsDirectory", props.fontsDirectory);
    propertiesNode->Add("FontsConfigsDirectory", props.fontsConfigsDirectory);
    propertiesNode->Add("TextsDirectory", props.textsDirectory);
    propertiesNode->Add("DefaultLanguage", props.defaultLanguage);

    node->Add("ProjectProperties", propertiesNode);

    return node;
}
