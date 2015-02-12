#include "LibraryWidget.h"
#include "ui_LibraryWidget.h"

#include "UI/Document.h"
#include "UI/LibraryContext.h"

LibraryWidget::LibraryWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LibraryWidget())
    , document(NULL)
{
    ui->setupUi(this);
}

LibraryWidget::~LibraryWidget()
{
    delete ui;
}

void LibraryWidget::SetDocument(Document *newDocument)
{
    if (document)
    {
//        disconnect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        ui->treeView->setModel(NULL);
    }
    
    document = newDocument;
    
    if (document)
    {
        ui->treeView->setModel(document->GetLibraryContext()->GetModel());
        ui->treeView->expandToDepth(0);
        ui->treeView->setColumnWidth(0, ui->treeView->size().width());
        
//        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
    }

}
