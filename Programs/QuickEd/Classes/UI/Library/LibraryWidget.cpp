#include "LibraryWidget.h"
#include "Modules/DocumentsModule/Document.h"
#include "LibraryModel.h"

#include "UI/UIControl.h"

LibraryWidget::LibraryWidget(QWidget* parent)
    : QDockWidget(parent)
    , libraryModel(new LibraryModel(this))
{
    setupUi(this);
    treeView->setModel(libraryModel);
}

void LibraryWidget::OnDocumentChanged(Document* document)
{
    if (document != nullptr)
    {
        libraryModel->SetPackageNode(document->GetPackage());
    }
    else
    {
        libraryModel->SetPackageNode(nullptr);
    }

    treeView->expandAll();
    treeView->collapse(libraryModel->GetDefaultControlsModelIndex());
}

void LibraryWidget::SetProjectLibraries(const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes, const DAVA::Vector<DAVA::FilePath>& libraryPackages)
{
    libraryModel->SetProjectLibraries(prototypes, libraryPackages);
}
