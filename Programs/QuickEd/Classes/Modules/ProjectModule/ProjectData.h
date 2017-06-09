#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <Base/Result.h>
#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <Math/Math2D.h>
#include <FileSystem/FilePath.h>

#include <QString>

#include <tuple>

namespace DAVA
{
class YamlNode;
class ResultList;
}

class ProjectData : public DAVA::TArc::DataNode
{
public:
    struct ResDir
    {
        DAVA::FilePath absolute;
        DAVA::String relative;
        bool operator==(const ResDir& other) const;
    };

    struct GfxDir
    {
        ResDir directory;
        DAVA::Size2i resolution;
    };

    ProjectData();
    ~ProjectData() override;

    static const DAVA::int32 CURRENT_PROJECT_FILE_VERSION = 1;

    static const DAVA::String& GetProjectFileName();
    static const DAVA::String& GetFontsConfigFileName();
    static DAVA::Result CreateNewProjectInfrastructure(const QString& projectPath);

    const DAVA::FilePath& GetProjectFile() const;
    const DAVA::FilePath& GetProjectDirectory() const;

    const ResDir& GetPluginsDirectory() const;
    const ResDir& GetResourceDirectory() const;
    const ResDir& GetAdditionalResourceDirectory() const;
    const ResDir& GetConvertedResourceDirectory() const;

    const ResDir& GetUiDirectory() const;
    const ResDir& GetFontsDirectory() const;
    const ResDir& GetFontsConfigsDirectory() const;
    const ResDir& GetTextsDirectory() const;
    const DAVA::Vector<GfxDir>& GetGfxDirectories() const;
    const DAVA::Vector<ResDir>& GetLibraryPackages() const;
    const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& GetPrototypes() const;

    const DAVA::String& GetDefaultLanguage() const;

    bool Save() const;

    static DAVA::FastName projectPathPropertyName;

private:
    friend class ProjectModule;

    DAVA::ResultList ParseLegacyProperties(const DAVA::FilePath& projectFile, const DAVA::YamlNode* root, int version);
    DAVA::ResultList Parse(const DAVA::FilePath& projectFile, const DAVA::YamlNode* node);

    void RefreshAbsolutePaths();
    DAVA::FilePath MakeAbsolutePath(const DAVA::String& relPath) const;

    DAVA::ResultList LoadProject(const QString& path);
    DAVA::RefPtr<DAVA::YamlNode> SerializeToYamlNode() const;

    void SetProjectFile(const DAVA::FilePath& newProjectFile);
    void SetDefaultLanguage(const DAVA::String& lang);

    DAVA::FilePath projectFile;
    DAVA::FilePath projectDirectory;

    DAVA::String defaultLanguage;

    ResDir pluginsDirectory;
    ResDir resourceDirectory;
    ResDir additionalResourceDirectory;
    ResDir convertedResourceDirectory;

    ResDir uiDirectory;
    ResDir fontsDirectory;
    ResDir fontsConfigsDirectory;
    ResDir textsDirectory;
    DAVA::Vector<GfxDir> gfxDirectories;
    DAVA::Vector<ResDir> libraryPackages;
    DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>> prototypes;

    DAVA_VIRTUAL_REFLECTION(ProjectData, DAVA::TArc::DataNode);
};
