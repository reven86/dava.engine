#ifndef QUICKED_EDITOR_CORE_H
#define QUICKED_EDITOR_CORE_H


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
}

class EditorCore : public QObject, public DAVA::Singleton<EditorCore>, public DAVA::InspBase
{
    Q_OBJECT
public:
    explicit EditorCore(QObject* parent = nullptr);
    ~EditorCore();
    MainWindow* GetMainWindow() const;
    Project* GetProject() const;
    void Start();

private slots:

    void OnReloadSpritesStarted();
    void OnReloadSpritesFinished();

    void OnProjectPathChanged(const QString& path);
    void OnGLWidgedInitialized();

    void RecentMenu(QAction*);

    void UpdateLanguage();

    void OnRtlChanged(bool isRtl);
    void OnBiDiSupportChanged(bool support);
    void OnGlobalStyleClassesChanged(const QString& classesStr);

    bool CloseProject();
    void OnExit();
    void OnNewProject();

private:
    void OpenProject(const QString& path);

    bool IsUsingAssetCache() const;
    bool eventFilter(QObject* object, QEvent* event) override;
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
    REGISTER_PREFERENCES(EditorCore)

public:
    INTROSPECTION(EditorCore,
                  PROPERTY("isUsingAssetCache", "Asset cache/Use asset cache", IsUsingAssetCache, SetUsingAssetCacheEnabled, DAVA::I_PREFERENCE)
                  )
};

inline EditorFontSystem* GetEditorFontSystem()
{
    return EditorCore::Instance()->GetProject()->GetEditorFontSystem();
}

#endif // QUICKED_EDITOR_CORE_H
