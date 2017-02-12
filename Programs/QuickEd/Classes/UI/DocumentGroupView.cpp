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
    connect(this, &MainWindow::DocumentGroupView::DocumentChanged, mainWindow->packageWidget, &PackageWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::DocumentChanged, mainWindow->libraryWidget, &LibraryWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::DocumentChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDocumentChanged);
    connect(this, &MainWindow::DocumentGroupView::DocumentChanged, mainWindow->styleSheetInspectorWidget, &StyleSheetInspectorWidget::OnDocumentChanged);

    connect(mainWindow->fileSystemDockWidget, &FileSystemDockWidget::OpenPackageFile, this, &MainWindow::DocumentGroupView::OpenPackageFile);
}

void MainWindow::DocumentGroupView::SetDocumentActionsEnabled(bool enabled)
{
    mainWindow->packageWidget->setEnabled(enabled);
    mainWindow->propertiesWidget->setEnabled(enabled);
    mainWindow->libraryWidget->setEnabled(enabled);
    mainWindow->styleSheetInspectorWidget->setEnabled(enabled);
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
        mainWindow->libraryWidget->SetProjectLibraries(project->GetPrototypes(), libraryPackages);
    }
    else
    {
        mainWindow->libraryWidget->SetProjectLibraries(DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>(), DAVA::Vector<DAVA::FilePath>());
    }

    mainWindow->propertiesWidget->SetProject(project);
}

void MainWindow::DocumentGroupView::SetDocument(Document* document)
{
    SetDocumentActionsEnabled(document != nullptr);
    emit DocumentChanged(document);
}
