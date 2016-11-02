#pragma once
#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"

namespace DAVA
{
class FilePath;
class YamlNode;
class ResultList;
}

class ProjectProperties
{
public:
    struct ResDir
    {
        DAVA::FilePath absolute;
        DAVA::String relative;
    };

    struct GfxDir
    {
        ResDir directory;
        DAVA::Size2i resolution;
    };

    static const DAVA::int32 CURRENT_PROJECT_FILE_VERSION = 1;

    static ProjectProperties Default();

    static std::tuple<DAVA::ResultList, ProjectProperties> Parse(const DAVA::FilePath& projectFile, const DAVA::YamlNode* node);
    static DAVA::RefPtr<DAVA::YamlNode> Emit(const ProjectProperties& properties);

    static const DAVA::String& GetProjectFileName();
    static const DAVA::String& GetFontsConfigFileName();

    const DAVA::FilePath& GetProjectFile() const;
    void SetProjectFile(const DAVA::FilePath& newProjectFile);
    const DAVA::FilePath& GetProjectDirectory() const;

    const ResDir& GetResourceDirectory() const;
    const ResDir& GetAdditionalResourceDirectory() const;
    const ResDir& GetIntermediateResourceDirectory() const;

    const ResDir& GetUiDirectory() const;
    const ResDir& GetFontsDirectory() const;
    const ResDir& GetFontsConfigsDirectory() const;
    const ResDir& GetTextsDirectory() const;
    const DAVA::Vector<GfxDir>& GetGfxDirectories() const;
    const DAVA::Vector<ResDir>& GetLibraryPackages() const;

    const DAVA::String& GetDefaultLanguage() const
    {
        return defaultLanguage;
    }

private:
    static std::tuple<DAVA::ResultList, ProjectProperties> ParseLegacyProperties(const DAVA::FilePath& projectFile, const DAVA::YamlNode* root, int version);
    void RefreshAbsolutePaths();
    DAVA::FilePath MakeAbsolutePath(const DAVA::String& relPath) const;

    DAVA::FilePath projectFile;
    DAVA::FilePath projectDirectory;

    DAVA::String defaultLanguage;

    ResDir resourceDirectory;
    ResDir additionalResourceDirectory;
    ResDir intermediateResourceDirectory;

    ResDir uiDirectory;
    ResDir fontsDirectory;
    ResDir fontsConfigsDirectory;
    ResDir textsDirectory;
    DAVA::Vector<GfxDir> gfxDirectories;
    DAVA::Vector<ResDir> libraryPackages;
};