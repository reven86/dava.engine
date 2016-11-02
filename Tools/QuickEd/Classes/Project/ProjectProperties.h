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
    static const DAVA::int32 CURRENT_PROJECT_FILE_VERSION = 1;

    static ProjectProperties Default();

    static std::tuple<DAVA::ResultList, ProjectProperties> Parse(const DAVA::FilePath& projectFile, const DAVA::YamlNode* node);
    static DAVA::RefPtr<DAVA::YamlNode> Emit(const ProjectProperties& properties);

    DAVA::FilePath GetResourceDirectory() const;
    DAVA::FilePath GetAdditionalResourceDirectory() const;
    DAVA::FilePath GetIntermediateResourceDirectory() const;

    DAVA::FilePath GetUiDirectory(bool useAdditionalResDir = false) const;
    DAVA::FilePath GetFontsDirectory(bool useAdditionalResDir = false) const;
    DAVA::FilePath GetFontsConfigsDirectory(bool useAdditionalResDir = false) const;
    DAVA::FilePath GetTextsDirectory(bool useAdditionalResDir = false) const;
    DAVA::Vector<DAVA::FilePath> GetGfxDirectories(bool useAdditionalResDir = false) const;
    DAVA::Vector<DAVA::FilePath> GetLibraryPackages(bool useAdditionalResDir = false) const;

    DAVA::FilePath projectFile;

    DAVA::String resourceDirectory;
    DAVA::String additionalResourceDirectory;
    DAVA::String intermediateResourceDirectory;

    DAVA::String uiDirectory;
    DAVA::String fontsDirectory;
    DAVA::String fontsConfigsDirectory;
    DAVA::String textsDirectory;

    DAVA::String defaultLanguage;

    DAVA::Vector<std::pair<DAVA::String, DAVA::Vector2>> gfxDirectories;
    DAVA::Vector<DAVA::String> libraryPackages;

private:
    DAVA::FilePath GetAbsolutePath(const DAVA::String& relPath, bool useAdditionalResDir) const;
};