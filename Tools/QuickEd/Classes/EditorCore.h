#ifndef QUICKED_EDITOR_CORE_H
#define QUICKED_EDITOR_CORE_H

#include <QObject>
#include "UI/mainwindow.h"
#include "Project/Project.h"
#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

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

class EditorCore final : public QObject, public DAVA::Singleton<EditorCore>
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
    void ApplyFileChanges();
    Document* GetDocument(const QString& path) const;
    void OpenProject(const QString& path);

    int CreateDocument(int index, const DAVA::RefPtr<PackageNode>& package);

    std::unique_ptr<SpritesPacker> spritesPacker;
    std::unique_ptr<DAVA::AssetCacheClient> cacheClient;

    Project* project = nullptr;
    DocumentGroup* documentGroup = nullptr;
    std::unique_ptr<MainWindow> mainWindow;
};

inline EditorFontSystem* GetEditorFontSystem()
{
    return EditorCore::Instance()->GetProject()->GetEditorFontSystem();
}

#endif // QUICKED_EDITOR_CORE_H
