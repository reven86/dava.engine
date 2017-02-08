#pragma once
#include "UI/mainwindow.h"

class Project;
class Document;
class LibraryWidget;
class PropertiesWidget;
class MainWindow;
class QTabBar;
class QAction;

class MainWindow::DocumentGroupView : public QObject
{
    Q_OBJECT
public:
    DocumentGroupView(MainWindow* mainWindow);

    void SetDocumentActionsEnabled(bool enabled);

    void SetProject(Project* project);

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