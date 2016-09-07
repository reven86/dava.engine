#pragma once

#include "Base/Introspection.h"
#include "Project/Project.h"
#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "AssetCache/AssetCacheClient.h"
#include "UI/mainwindow.h"
#include <QObject>

class QAction;
class Document;
class DocumentGroup;
class Project;
class PackageNode;
class SpritesPacker;

namespace DAVA
{
class AssetCacheClient;
class Engine;
class Window;
}

class EditorCore : public QObject, public DAVA::Singleton<EditorCore>, public DAVA::InspBase
{
    Q_OBJECT
public:
    explicit EditorCore(DAVA::Engine& engine);
    ~EditorCore();
    MainWindow* GetMainWindow() const;
    Project* GetProject() const;
    void Start();

    void OnWindowCreated(DAVA::Window& window);
    void OnLoopStarted();
    void OnLoopStopped();
    void OnGLWidgedInitialized(DAVA::Window& window);

private slots:
    bool CloseProject();
    void OnReloadSpritesStarted();
    void OnReloadSpritesFinished();

    void OnProjectPathChanged(const QString& path);

    void RecentMenu(QAction*);

    void UpdateLanguage();

    void OnRtlChanged(bool isRtl);
    void OnBiDiSupportChanged(bool support);
    void OnGlobalStyleClassesChanged(const QString& classesStr);

    void OnExit();
    void OnNewProject();
    void OnProjectOpenChanged(bool arg);

private:
    void OpenProject(const QString& path);

    bool IsUsingAssetCache() const;
    void SetUsingAssetCacheEnabled(bool enabled);

    void EnableCacheClient();
    void DisableCacheClient();

    std::unique_ptr<SpritesPacker> spritesPacker;
    std::unique_ptr<DAVA::AssetCacheClient> cacheClient;

    Project* project = nullptr;
    DocumentGroup* documentGroup = nullptr;
    std::unique_ptr<MainWindow> mainWindow;

    DAVA::AssetCacheClient::ConnectionParams connectionParams;
    bool assetCacheEnabled;

public:
    INTROSPECTION(EditorCore,
                  PROPERTY("isUsingAssetCache", "Asset cache/Use asset cache", IsUsingAssetCache, SetUsingAssetCacheEnabled, DAVA::I_PREFERENCE)
                  )
};

inline EditorFontSystem* GetEditorFontSystem()
{
    return EditorCore::Instance()->GetProject()->GetEditorFontSystem();
}
