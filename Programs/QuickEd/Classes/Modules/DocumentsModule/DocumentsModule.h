#pragma once

#include "Application/QEGlobal.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "Utils/PackageListenerProxy.h"

#include <TArc/Core/ControllerModule.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/QtConnections.h>

#include <QtTools/Utils/QtDelayedExecutor.h>

class FindInDocumentController;
class PreviewWidget;
class EditorSystemsManager;
class PackageNode;
class ControlNode;

class DocumentsModule : public DAVA::TArc::ControllerModule, PackageListener
{
public:
    DocumentsModule();
    ~DocumentsModule() override;

protected:
    void OnRenderSystemInitialized(DAVA::Window* window) override;
    bool CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key, DAVA::String& requestWindowText) override;
    void SaveOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::TArc::WindowKey& key) override;

    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;

    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

private:
    void InitEditorSystems();
    void InitCentralWidget();
    void InitWatcher();

    void CreateDocumentsActions();
    void RegisterOperations();

    //Edit
    void CreateUndoRedoActions();
    void OnUndo();
    void OnRedo();

    //View
    void CreateViewActions();

    DAVA::TArc::DataContext::ContextID OpenDocument(const QString& path);
    DAVA::RefPtr<PackageNode> CreatePackage(const QString& path);

    void CloseActiveDocument();
    void CloseDocument(DAVA::uint64 id);
    void CloseAllDocuments();
    void DeleteAllDocuments();
    void CloseDocuments(const DAVA::Set<DAVA::TArc::DataContext::ContextID>& ids);

    void ReloadCurrentDocument();
    void ReloadDocument(const DAVA::TArc::DataContext::ContextID& contextID);
    void ReloadDocuments(const DAVA::Set<DAVA::TArc::DataContext::ContextID>& ids);

    bool HasUnsavedDocuments() const;
    bool SaveDocument(const DAVA::TArc::DataContext::ContextID& contextID);
    void SaveAllDocuments();
    void SaveCurrentDocument();

    void SelectControl(const QString& documentPath, const QString& controlPath);

    //previewWidget helper functions
    void ChangeControlText(ControlNode* node);

    //documents watcher
    void OnFileChanged(const QString& path);
    void OnApplicationStateChanged(Qt::ApplicationState state);

    void ApplyFileChanges();
    DAVA::TArc::DataContext::ContextID GetContextByPath(const QString& path) const;

    void OnDragStateChanged(EditorSystemsManager::eDragState dragState, EditorSystemsManager::eDragState previousState);
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;

    PreviewWidget* previewWidget = nullptr;
    std::unique_ptr<EditorSystemsManager> systemsManager;
    DAVA::TArc::QtConnections connections;

    friend class FindInDocumentController;
    std::unique_ptr<FindInDocumentController> findInDocumentController;

    QtDelayedExecutor delayedExecutor;

    PackageListenerProxy packageListenerProxy;

    DAVA_VIRTUAL_REFLECTION(DocumentsModule, DAVA::TArc::ControllerModule);
};
