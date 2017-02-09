#include "Modules/ProjectModule/ProjectData.h"

#include <Base/Result.h>
#include <Engine/Engine.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/YamlNode.h>
#include <Utils/Utils.h>
#include <Utils/StringFormat.h>
#include <FileSystem/YamlParser.h>

#include <QObject>
#include <QFileInfo>

DAVA_VIRTUAL_REFLECTION_IMPL(ProjectData)
{
    DAVA::ReflectionRegistrator<ProjectData>::Begin()
    .Field(projectPathPropertyName, &ProjectData::GetProjectFile, nullptr)
    .End();
}

using namespace DAVA;

const char* ProjectData::projectPathPropertyName = "ProjectPath";

DAVA::ResultList ProjectData::ParseLegacyProperties(const DAVA::FilePath& projectFile, const YamlNode* root, int version)
{
    ResultList resultList;

    DVASSERT(version == ProjectData::CURRENT_PROJECT_FILE_VERSION - 1, "Supported only one recent previous version of project");
    if (version != ProjectData::CURRENT_PROJECT_FILE_VERSION - 1)
    {
        String message = Format("Supported only project files with versions %d and %d.", ProjectData::CURRENT_PROJECT_FILE_VERSION, ProjectData::CURRENT_PROJECT_FILE_VERSION - 1);
        resultList.AddResult(Result::RESULT_ERROR, message);
        return resultList;
    }

    additionalResourceDirectory.relative = String("./Data/");

    if (root == nullptr) // for support old project
    {
        SetDefaultLanguage("");
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
                fontsConfigsDirectory.relative = fontsConfigsPath;
            }
        }

        const YamlNode* localizationPathNode = root->Get("LocalizationPath");
        const YamlNode* localeNode = root->Get("Locale");
        if (localizationPathNode != nullptr && localeNode != nullptr)
        {
            String localePath = FilePath(localizationPathNode->AsString()).GetRelativePathname("~res:/");
            textsDirectory.relative = localePath;
            defaultLanguage = localeNode->AsString();
        }

        const YamlNode* libraryNode = root->Get("Library");
        if (libraryNode != nullptr)
        {
            for (uint32 i = 0; i < libraryNode->GetCount(); i++)
            {
                String packagePath = FilePath(libraryNode->Get(i)->AsString()).GetRelativePathname("~res:/");
                libraryPackages.push_back({ "", packagePath });
            }
        }
    }

    SetProjectFile(projectFile);

    return resultList;
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

const DAVA::String& ProjectData::GetDefaultLanguage() const
{
    return defaultLanguage;
}

void ProjectData::SetDefaultLanguage(const DAVA::String& lang)
{
    defaultLanguage = lang;
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

DAVA::ResultList ProjectData::Parse(const DAVA::FilePath& projectFile, const YamlNode* root)
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

        return resultList;
    }

    const YamlNode* resourceDirNode = ProjectDataNode->Get("ResourceDirectory");
    if (resourceDirNode != nullptr)
    {
        resourceDirectory.relative = resourceDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directory not set. Used default directory: %s.", resourceDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* additionalResourceDirNode = ProjectDataNode->Get("AdditionalResourceDirectory");
    if (additionalResourceDirNode != nullptr)
    {
        additionalResourceDirectory.relative = additionalResourceDirNode->AsString();
    }

    const YamlNode* convertedResourceDirNode = ProjectDataNode->Get("ConvertedResourceDirectory");
    if (convertedResourceDirNode != nullptr)
    {
        convertedResourceDirectory.relative = convertedResourceDirNode->AsString();
    }
    else
    {
        String message = Format("Directory for converted sources not set. Used default directory: %s.", convertedResourceDirectory.relative.c_str());
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
            gfxDirectories.push_back({ ResDir{ FilePath(), directory }, resolution });
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s, with resolution %dx%d.",
                                gfxDirectories.front().directory.relative.c_str(),
                                gfxDirectories.front().resolution.dx,
                                gfxDirectories.front().resolution.dy);
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* uiDirNode = ProjectDataNode->Get("UiDirectory");
    if (uiDirNode != nullptr)
    {
        uiDirectory.relative = uiDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", uiDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* fontsDirNode = ProjectDataNode->Get("FontsDirectory");
    if (fontsDirNode != nullptr)
    {
        fontsDirectory.relative = fontsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", fontsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* fontsConfigsDirNode = ProjectDataNode->Get("FontsConfigsDirectory");
    if (fontsConfigsDirNode != nullptr)
    {
        fontsConfigsDirectory.relative = fontsConfigsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", fontsConfigsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* textsDirNode = ProjectDataNode->Get("TextsDirectory");
    if (textsDirNode != nullptr)
    {
        textsDirectory.relative = textsDirNode->AsString();
        const YamlNode* defaultLanguageNode = ProjectDataNode->Get("DefaultLanguage");
        if (defaultLanguageNode != nullptr)
        {
            defaultLanguage = defaultLanguageNode->AsString();
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", textsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }

    const YamlNode* libraryNode = ProjectDataNode->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            libraryPackages.push_back(ResDir{ FilePath(), libraryNode->Get(i)->AsString() });
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
            prototypes[packagePath] = packagePrototypes;
        }
    }

    SetProjectFile(projectFile);

    return resultList;
}

DAVA::ResultList ProjectData::LoadProject(const QString& path)
{
    using namespace DAVA;
    ResultList resultList;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        QString message = QObject::tr("%1 does not exist.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return resultList;
    }

    if (!fileInfo.isFile())
    {
        QString message = QObject::tr("%1 is not a file.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return resultList;
    }

    RefPtr<YamlParser> parser(YamlParser::Create(path.toStdString()));
    if (parser.Get() == nullptr)
    {
        QString message = QObject::tr("Can not parse project file %1.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());

        return resultList;
    }
    return Parse(path.toStdString(), parser->GetRootNode());
}

RefPtr<YamlNode> ProjectData::SerializeToYamlNode() const
{
    RefPtr<YamlNode> node(YamlNode::CreateMapNode(false));

    YamlNode* headerNode(YamlNode::CreateMapNode(false));
    headerNode->Add("version", CURRENT_PROJECT_FILE_VERSION);
    node->Add("Header", headerNode);

    YamlNode* propertiesNode(YamlNode::CreateMapNode(false));
    propertiesNode->Add("ResourceDirectory", resourceDirectory.relative);

    if (!additionalResourceDirectory.relative.empty())
    {
        propertiesNode->Add("AdditionalResourceDirectory", additionalResourceDirectory.relative);
    }

    propertiesNode->Add("IntermediateResourceDirectory", convertedResourceDirectory.relative);

    propertiesNode->Add("UiDirectory", uiDirectory.relative);
    propertiesNode->Add("FontsDirectory", fontsDirectory.relative);
    propertiesNode->Add("FontsConfigsDirectory", fontsConfigsDirectory.relative);
    propertiesNode->Add("TextsDirectory", textsDirectory.relative);
    propertiesNode->Add("DefaultLanguage", defaultLanguage);

    YamlNode* gfxDirsNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const auto& gfxDir : gfxDirectories)
    {
        YamlNode* gfxDirNode(YamlNode::CreateMapNode(false));
        gfxDirNode->Add("directory", gfxDir.directory.relative);
        Vector2 resolution((float32)gfxDir.resolution.dx, (float32)gfxDir.resolution.dy);
        gfxDirNode->Add("resolution", resolution);
        gfxDirsNode->Add(gfxDirNode);
    }
    propertiesNode->Add("GfxDirectories", gfxDirsNode);

    YamlNode* librarysNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const auto& resDir : libraryPackages)
    {
        librarysNode->Add(resDir.relative);
    }
    propertiesNode->Add("Library", librarysNode);

    node->Add("ProjectProperties", propertiesNode);

    return node;
}

ProjectData::ProjectData()
{
    resourceDirectory.relative = "./DataSource/";
    convertedResourceDirectory.relative = "./Data/";
    gfxDirectories.push_back({ ResDir{ FilePath(), String("./Gfx/") }, Size2i(960, 640) });
    uiDirectory.relative = "./UI/";
    fontsDirectory.relative = "./Fonts/";
    fontsConfigsDirectory.relative = "./Fonts/Configs/";
    textsDirectory.relative = "./Strings/";
    defaultLanguage = "en";
}

ProjectData::~ProjectData()
{
    FilePath::RemoveResourcesFolder(resourceDirectory.absolute);
    FilePath::RemoveResourcesFolder(additionalResourceDirectory.absolute);
    FilePath::RemoveResourcesFolder(convertedResourceDirectory.absolute);
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

bool ProjectData::ResDir::operator==(const ProjectData::ResDir& other) const
{
    if (this == &other)
    {
        return true;
    }
    return other.absolute == absolute &&
    other.relative == relative;
}
