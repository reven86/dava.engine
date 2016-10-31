#include "DocumentGroupView.h"
#include "ui_mainwindow.h"
#include "UI/Preview/PreviewWidget.h"
#include "UI/Package/PackageWidget.h"
#include "UI/Library/LibraryWidget.h"
#include "UI/Properties/PropertiesWidget.h"
#include "Document/Document.h"

MainWindow::DocumentGroupView::DocumentGroupView(MainWindow* aMainWindow)
    : QObject(aMainWindow)
    , mainWindow(aMainWindow)
{
    SetDocumentActionsEnabled(false);
    connect(this, &MainWindow::DocumentGroupView::OnDocumentChanged, mainWindow->ui->previewWidget, &PreviewWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::OnDocumentChanged, mainWindow->ui->packageWidget, &PackageWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::OnDocumentChanged, mainWindow->ui->libraryWidget, &LibraryWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::OnDocumentChanged, mainWindow->ui->propertiesWidget, &PropertiesWidget::OnDocumentChanged);

    connect(mainWindow->ui->fileSystemDockWidget, &FileSystemDockWidget::OpenPackageFile, this, &MainWindow::DocumentGroupView::OpenPackageFile);
    connect(mainWindow->ui->previewWidget, &PreviewWidget::OpenPackageFile, this, &MainWindow::DocumentGroupView::OpenPackageFile);
}

QTabBar* MainWindow::DocumentGroupView::GetTabBar()
{
    return mainWindow->ui->tabBar;
}

QAction* MainWindow::DocumentGroupView::GetActionRedo()
{
    return mainWindow->ui->actionRedo;
}

QAction* MainWindow::DocumentGroupView::GetActionUndo()
{
    return mainWindow->ui->actionUndo;
}

QAction* MainWindow::DocumentGroupView::GetActionSaveDocument()
{
    return mainWindow->ui->actionSaveDocument;
}

QAction* MainWindow::DocumentGroupView::GetActionSaveAllDocuments()
{
    return mainWindow->ui->actionSaveAllDocuments;
}

QAction* MainWindow::DocumentGroupView::GetActionCloseDocument()
{
    return mainWindow->ui->actionCloseDocument;
}

QAction* MainWindow::DocumentGroupView::GetActionReloadDocument()
{
    return mainWindow->ui->actionReloadDocument;
}

void MainWindow::DocumentGroupView::SetDocumentActionsEnabled(bool enabled)
{
    mainWindow->ui->packageWidget->setEnabled(enabled);
    mainWindow->ui->propertiesWidget->setEnabled(enabled);
    mainWindow->ui->libraryWidget->setEnabled(enabled);

    mainWindow->ui->actionSaveDocument->setEnabled(enabled);
    mainWindow->ui->actionSaveAllDocuments->setEnabled(enabled);
    mainWindow->ui->actionReloadDocument->setEnabled(enabled);
    mainWindow->ui->actionCloseDocument->setEnabled(enabled);

    mainWindow->ui->actionRedo->setEnabled(enabled);
    mainWindow->ui->actionUndo->setEnabled(enabled);
}

LibraryWidget* MainWindow::DocumentGroupView::GetLibraryWidget()
{
    return mainWindow->ui->libraryWidget;
}

PropertiesWidget* MainWindow::DocumentGroupView::GetPropertiesWidget()
{
    return mainWindow->ui->propertiesWidget;
}
