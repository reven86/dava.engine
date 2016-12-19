#pragma once

#include "Base/Introspection.h"
#include "AssetCache/AssetCacheClient.h"

#include <QObject>

class QAction;
class QDir;
class Document;
class DocumentGroup;
class Project;
class PackageNode;
class SpritesPacker;
class MainWindow;

namespace DAVA
{
class AssetCacheClient;
class Engine;
struct Result;
class ResultList;
}

class EditorCore : public QObject, public DAVA::InspBase
{
    Q_OBJECT
public:
    EditorCore();

    ~EditorCore();

    void OnRenderingInitialized();

    MainWindow* GetMainWindow() const;
    bool CanCloseProjectSilently() const;
    QStringList GetUnsavedDocumentsNames() const;
    void SaveAllDocuments();
    bool CloseProject(bool force);

signals:
    void AssetCacheChanged(DAVA::AssetCacheClient* assetCacheClient);
    bool TryCloseDocuments();
    void ProjectChanged(Project* project);

private slots:
    void OnNewProject();
    void OnOpenProject();
    void OnCloseProject();
    void OnShowHelp();

private:
    void OpenProject(const QString& path);
    void SetProject(Project* project);

    bool IsUsingAssetCache() const;
    void SetUsingAssetCacheEnabled(bool enabled);

    void EnableCacheClient();
    void DisableCacheClient();

    const QStringList& GetRecentProjects() const;
    QString GetLastProject() const;
    void AddRecentProject(const QString& projectPath);

    DAVA::String GetRecentProjectsAsString() const;
    void SetRecentProjectsFromString(const DAVA::String& str);

    QString GenerateEditorTitle() const;

    void UnpackHelp();

    std::unique_ptr<DAVA::AssetCacheClient> cacheClient;

    std::unique_ptr<Project> project;

    //TArc is owner of mainWindow
    MainWindow* mainWindow = nullptr;

    DAVA::AssetCacheClient::ConnectionParams connectionParams;
    bool assetCacheEnabled;

    QStringList recentProjects;

    DAVA::uint32 projectsHistorySize; //maximum size of projects history

public:
    INTROSPECTION(EditorCore,
                  PROPERTY("isUsingAssetCache", "Asset cache/Use asset cache", IsUsingAssetCache, SetUsingAssetCacheEnabled, DAVA::I_PREFERENCE)
                  PROPERTY("projectsHistory", "ProjectInternal/ProjectsHistory", GetRecentProjectsAsString, SetRecentProjectsFromString, DAVA::I_SAVE | DAVA::I_PREFERENCE)
                  MEMBER(projectsHistorySize, "Project/projects history size", DAVA::I_SAVE | DAVA::I_PREFERENCE)
                  )
};
