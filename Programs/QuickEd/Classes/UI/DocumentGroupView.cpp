#include "DocumentGroupView.h"

#include "Modules/LegacySupportModule/Private/Document.h"
#include "Modules/LegacySupportModule/Private/Project.h"
#include "UI/Library/LibraryWidget.h"
#include "UI/Package/PackageWidget.h"
#include "UI/Preview/PreviewWidget.h"
#include "UI/Properties/PropertiesWidget.h"
#include "UI/StyleSheetInspector/StyleSheetInspectorWidget.h"

#include "ui_mainwindow.h"

MainWindow::DocumentGroupView::DocumentGroupView(MainWindow* mainWindow_)
    : QObject(mainWindow_)
    , mainWindow(mainWindow_)
{
    SetDocumentActionsEnabled(false);
    connect(this, &MainWindow::DocumentGroupView::DocumentChanged, mainWindow->ui->packageWidget, &PackageWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::DocumentChanged, mainWindow->ui->libraryWidget, &LibraryWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::DocumentChanged, mainWindow->ui->propertiesWidget, &PropertiesWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::DocumentChanged, mainWindow->ui->styleSheetInspectorWidget, &StyleSheetInspectorWidget::OnDocumentChanged);

    connect(mainWindow->ui->fileSystemDockWidget, &FileSystemDockWidget::OpenPackageFile, this, &MainWindow::DocumentGroupView::OpenPackageFile);
}

void MainWindow::DocumentGroupView::SetDocumentActionsEnabled(bool enabled)
{
    mainWindow->ui->packageWidget->setEnabled(enabled);
    mainWindow->ui->propertiesWidget->setEnabled(enabled);
    mainWindow->ui->libraryWidget->setEnabled(enabled);
    mainWindow->ui->styleSheetInspectorWidget->setEnabled(enabled);
}

void MainWindow::DocumentGroupView::SetProject(Project* project)
{
    if (project)
    {
        DAVA::Vector<DAVA::FilePath> libraryPackages;
        for (const auto& resDir : project->GetLibraryPackages())
        {
            libraryPackages.push_back(resDir.absolute);
        }
        mainWindow->ui->libraryWidget->SetProjectLibraries(project->GetPrototypes(), libraryPackages);
    }
    else
    {
        mainWindow->ui->libraryWidget->SetProjectLibraries(DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>(), DAVA::Vector<DAVA::FilePath>());
    }

    mainWindow->ui->propertiesWidget->SetProject(project);
}

void MainWindow::DocumentGroupView::SetDocument(Document* document)
{
    SetDocumentActionsEnabled(document != nullptr);
    emit DocumentChanged(document);
}
