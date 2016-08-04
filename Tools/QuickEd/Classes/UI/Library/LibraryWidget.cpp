#include "LibraryWidget.h"
#include "Document.h"
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
    libraryModel->SetPackageNode(nullptr != document ? document->GetPackage() : nullptr);
    treeView->expandAll();
    treeView->collapse(libraryModel->GetDefaultControlsModelIndex());
}

void LibraryWidget::SetLibraryPackages(const DAVA::Vector<DAVA::FilePath>& libraryPackages)
{
    libraryModel->SetLibraryPackages(libraryPackages);
}
