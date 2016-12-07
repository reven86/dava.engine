#include "FindWidget.h"
#include "Document/Document.h"

#include "UI/UIControl.h"

FindWidget::FindWidget(QWidget* parent)
    : QDockWidget(parent)
{
    ui.setupUi(this);
    //    treeView->setModel(libraryModel);
}

void FindWidget::OnDocumentChanged(Document* document)
{
    if (document != nullptr)
    {
    }
    else
    {
    }

    ui.treeView->expandAll();
}
