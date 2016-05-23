#pragma once

class QString;
class QStringList;

class ResourcesManageHelper
{
public:
    // Helper to work with resources
    static QString GetFontRelativePath(const QString& resourceFileName, bool graphicsFont = false);
    static QStringList GetFontsList();
    // Get path to documentation folder
    static QString GetDocumentationPath();

    // Resource directory
    static QString GetResourceRootDirectory();
    // Get project title string
    static QString GetProjectTitle();
    // Get project title string
    static QString GetProjectTitle(const QString& projectFilePath);
    // Get the folder wehere project is located
    static QString GetProjectPath();
    static void SetProjectPath(const QString& path);
    // Get folder with resource data for specified project path
    static QString GetDataPath(const QString& projectPath);
    // Project file path for specified project path
    static QString GetProjectFilePath(const QString& projectPath);
    // Initialize project internal resources
    // Do not use this function elsewhere than during application init!!!
    static void InitInternalResources();

private:
    static QString projectTitle;
    static QString projectPath;
};
