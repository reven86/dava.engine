#include "LibraryWidget.h"
#include "ui_LibraryWidget.h"

LibraryWidget::LibraryWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LibraryWidget())
{
    ui->setupUi(this);
//TODO:
    //check that we need to do this after model changed
    //!!ui->treeView->expandToDepth(0);
    //!!ui->treeView->setColumnWidth(0, ui->treeView->size().width());
}

LibraryWidget::~LibraryWidget()
{
    delete ui;
}

void LibraryWidget::OnModelChanged(QAbstractItemModel* model)
{
    ui->treeView->setModel(model);
}