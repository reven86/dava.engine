#include "LibraryWidget.h"
#include "Document/Document.h"
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
        libraryModel->SetPackageNode(document->GetProject(), document->GetPackage());
    }
    else
    {
        libraryModel->SetPackageNode(nullptr, nullptr);
    }

    treeView->expandAll();
    treeView->collapse(libraryModel->GetDefaultControlsModelIndex());
}

void LibraryWidget::SetLibraryPackages(const DAVA::Vector<DAVA::FilePath>& libraryPackages)
{
    libraryModel->SetLibraryPackages(libraryPackages);
}
