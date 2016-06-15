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
    treeView->expandToDepth(0);
}

void LibraryWidget::OnDocumentChanged(Document* document)
{
    libraryModel->SetPackageNode(nullptr != document ? document->GetPackage() : nullptr);
    treeView->expandToDepth(0);
}
