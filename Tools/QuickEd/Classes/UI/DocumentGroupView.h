#pragma once
#include "UI/mainwindow.h"

class Document;
class LibraryWidget;
class PropertiesWidget;

class MainWindow::DocumentGroupView : public QObject
{
    Q_OBJECT
public:
    DocumentGroupView(MainWindow* aMainWindow);

    void SetDocumentActionsEnabled(bool enabled);

    LibraryWidget* GetLibraryWidget();
    PropertiesWidget* GetPropertiesWidget();

    QTabBar* GetTabBar();
    QAction* GetActionRedo();
    QAction* GetActionUndo();

    QAction* GetActionSaveDocument();
    QAction* GetActionSaveAllDocuments();
    QAction* GetActionCloseDocument();
    QAction* GetActionReloadDocument();

    MainWindow* mainWindow = nullptr;

signals:
    void OpenPackageFile(const QString& path);
    void OnDocumentChanged(Document* document);
};