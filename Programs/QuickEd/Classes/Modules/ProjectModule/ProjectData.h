#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
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

    static const DAVA::int32 CURRENT_PROJECT_FILE_VERSION = 1;

    static const DAVA::String& GetProjectFileName();
    static const DAVA::String& GetFontsConfigFileName();

    DAVA::ResultList LoadProject(const QString& path);
    DAVA::RefPtr<DAVA::YamlNode> SerializeToYamlNode() const;

    const DAVA::FilePath& GetProjectFile() const;
    void SetProjectFile(const DAVA::FilePath& newProjectFile);
    const DAVA::FilePath& GetProjectDirectory() const;

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

    void SetDefaultLanguage(const DAVA::String& lang);

    static const char* projectPathPropertyName;
    static const char* uiDirectoryPropertyName;

private:
    DAVA::ResultList ParseLegacyProperties(const DAVA::FilePath& projectFile, const DAVA::YamlNode* root, int version);
    DAVA::ResultList Parse(const DAVA::FilePath& projectFile, const DAVA::YamlNode* node);

    void RefreshAbsolutePaths();
    DAVA::FilePath MakeAbsolutePath(const DAVA::String& relPath) const;

    DAVA::FilePath projectFile;
    DAVA::FilePath projectDirectory;

    DAVA::String defaultLanguage;

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

namespace DAVA
{
template <>
struct AnyCompare<ProjectData::ResDir>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        const ProjectData::ResDir& s1 = v1.Get<ProjectData::ResDir>();
        const ProjectData::ResDir& s2 = v2.Get<ProjectData::ResDir>();
        return s1 == s2;
    }
};
}
