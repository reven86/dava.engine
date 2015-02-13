#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Document.h"
#include "UI/PropertiesContext.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"
#include "Model/PackageHierarchy/ControlNode.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PropertiesWidget())
    , context(NULL)
{
    ui->setupUi(this);
}

PropertiesWidget::~PropertiesWidget()
{
    delete ui;
    ui = NULL;
}

void PropertiesWidget::SetContext(PropertiesContext *newContext)
{
    if (context)
    {
        disconnect(context->GetDocument(), SIGNAL(controlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), this, SLOT(OnControlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
        ui->treeView->setModel(NULL);
        //ui->filterLine->setEnabled(false);
        //ui->treeView->setEnabled(false);
    }

    context = newContext;

    if (context)
    {
        //ui->treeView->setModel(document->GetTreeContext()->proxyModel);
        //ui->filterLine->setText(document->GetTreeContext()->filterString);
        connect(context->GetDocument(), SIGNAL(controlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)), this, SLOT(OnControlsSelectionChanged(const QList<ControlNode*> &, const QList<ControlNode*> &)));
        //ui->filterLine->setEnabled(true);
        //ui->treeView->setEnabled(true);
    }
}

void PropertiesWidget::OnControlsSelectionChanged(const QList<ControlNode*> &activatedControls, const QList<ControlNode*> &deactivatedControls)
{
    if (!activatedControls.empty())
        SetControl(activatedControls.front());
    else
        SetControl(NULL);
}

void PropertiesWidget::SetControl(ControlNode *controlNode)
{
    if (controlNode)
    {
        QStyledItemDelegate *itemDelegate = new PropertiesTreeItemDelegate(this);
        PropertiesModel *model = new PropertiesModel(controlNode, context, this);
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
