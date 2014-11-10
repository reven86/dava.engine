//
//  PropertiesTreeWidget.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 12.9.14.
//
//

#include "PropertiesDockWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>

#include "ui_PropertiesDockWidget.h"
#include "PropertiesTreeModel.h"
#include "UI/PackageDocument.h"
#include "UI/PropertiesView/PropertiesTreeItemDelegate.h"
#include "PropertiesViewContext.h"
#include "UIControls/PackageHierarchy/ControlNode.h"

using namespace DAVA;

PropertiesDockWidget::PropertiesDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PropertiesDockWidget())
    , context(NULL)
{
    ui->setupUi(this);
}

PropertiesDockWidget::~PropertiesDockWidget()
{
    delete ui;
    ui = NULL;
}

void PropertiesDockWidget::SetContext(PropertiesViewContext *newContext)
{
    if (context)
    {
        disconnect(context->Document(), SIGNAL(controlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), this, SLOT(OnControlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
        ui->treeView->setModel(NULL);
        //ui->filterLine->setEnabled(false);
        //ui->treeView->setEnabled(false);
    }

    context = newContext;

    if (context)
    {
        //ui->treeView->setModel(document->GetTreeContext()->proxyModel);
        //ui->filterLine->setText(document->GetTreeContext()->filterString);
        connect(context->Document(), SIGNAL(controlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), this, SLOT(OnControlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
        //ui->filterLine->setEnabled(true);
        //ui->treeView->setEnabled(true);
    }
}

void PropertiesDockWidget::OnControlsSelectionChanged(const QList<ControlNode*> &activatedControls, const QList<ControlNode*> &deactivatedControls)
{
    if (!activatedControls.empty())
        SetControl(activatedControls.front());
    else
        SetControl(NULL);
}

void PropertiesDockWidget::SetControl(ControlNode *controlNode)
{
    if (controlNode)
    {
        QStyledItemDelegate *itemDelegate = new PropertiesTreeItemDelegate(this);
        PropertiesTreeModel *model = new PropertiesTreeModel(controlNode->GetPropertiesRoot(), context, this);
//        QItemEditorFactory *itemEditorFactory = new QItemEditorFactory();
//        itemEditorFactory->registerEditor( QVariant::Vector2D, new QStandardItemEditorCreator<Vector2Edit>());
//        itemDelegate->setItemEditorFactory(itemEditorFactory);

        //QItemEditorFactory::setDefaultFactory(itemEditorFactory);
        ui->treeView->setItemDelegate(itemDelegate);
        ui->treeView->setModel(model);
        ui->treeView->expandToDepth(0);
        ui->treeView->resizeColumnToContents(0);
    }
    else
    {
        ui->treeView->setModel(NULL);
    }
}
