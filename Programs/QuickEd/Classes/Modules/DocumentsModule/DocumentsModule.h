#pragma once

#include "Application/QEGlobal.h"
#include <TArc/Core/ControllerModule.h>
#include <TArc/Utils/QtConnections.h>

class PreviewWidget;
class EditorSystemsManager;
struct DocumentData;

class DocumentsModule : public DAVA::TArc::ControllerModule
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
    void OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne) override;

private:
    void InitEditorSystems();
    void InitMainWindow();
    void InitCentralWidget();

    void CreateActions();
    void RegisterOperations();

    void OpenDocument(const QString& path);
    std::unique_ptr<DocumentData> CreateDocument(const QString& path);

    void TryCloseActiveDocument();
    void CloseAllDocuments();
    void CloseDocuments(const QEGlobal::IDList& ids);

    void ReloadCurrentDocument();
    void ReloadDocument(const DAVA::TArc::DataContext::ContextID& contextID);
    void ReloadDocuments(const QEGlobal::IDList& ids);

    bool HasUnsavedDocuments() const;
    void SaveDocument(const DAVA::TArc::DataContext::ContextID& contextID);
    void SaveAllDocuments();
    void SaveCurrentDocument();

    void OnActiveTabChanged(const DAVA::Any& contextID);

    PreviewWidget* centralWidget = nullptr;
    std::unique_ptr<EditorSystemsManager> systemsManager;
    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DocumentsModule, DAVA::TArc::ControllerModule)
    {
        DAVA::ReflectionRegistrator<DocumentsModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
