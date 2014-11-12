#include "LibraryDockWidget.h"
#include "ui_LibraryDockWidget.h"

#include "UI/PackageDocument.h"

LibraryDockWidget::LibraryDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LibraryDockWidget())
    , document(NULL)
{
    ui->setupUi(this);
}

LibraryDockWidget::~LibraryDockWidget()
{
    delete ui;
}

void LibraryDockWidget::SetDocument(PackageDocument *newDocument)
{
    if (document)
    {
//        disconnect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        ui->treeView->setModel(NULL);
    }
    
    document = newDocument;
    
    if (document)
    {
        ui->treeView->setModel(document->GetLibraryContext()->model);
        ui->treeView->expandToDepth(0);
        ui->treeView->setColumnWidth(0, ui->treeView->size().width());
        
//        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
    }

}
