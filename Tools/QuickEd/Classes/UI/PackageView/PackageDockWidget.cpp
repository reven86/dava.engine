//
//  PackageTreeWidget.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 10.9.14.
//
//

#include "PackageDockWidget.h"
#include "ui_PackageDockWidget.h"
#include "UIPackageModel.h"
#include "DAVAEngine.h"
#include "UI/PackageView/UIFilteredPackageModel.h"
#include "UI/PackageDocument.h"
#include "UI/PackageView/PackageModelCommands.h"
#include "UIControls/PackageHierarchy/PackageBaseNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"

#include "Project.h"

using namespace DAVA;

PackageDockWidget::PackageDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PackageDockWidget())
    , document(NULL)
{
    ui->setupUi(this);
    ui->treeView->header()->setResizeMode(QHeaderView::ResizeToContents);

    connect(ui->filterLine, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged(const QString &)));

    //setTitleBarWidget(ui->filterLine);
    //ui->filterLine->setEnabled(false);
    //ui->treeView->setEnabled(false);
    
    QAction *removeAction = new QAction("Remove", this);
    removeAction->setShortcut(QKeySequence(Qt::Key_Delete));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(OnRemove()));
    ui->treeView->addAction(removeAction);
}

PackageDockWidget::~PackageDockWidget()
{
    disconnect(ui->filterLine, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged(const QString &)));
    ui->treeView->setModel(NULL);
    delete ui;
    ui = NULL;
}

void PackageDockWidget::SetDocument(PackageDocument *newDocument)
{
    if (document)
    {
        disconnect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        ui->treeView->setModel(NULL);
        //ui->filterLine->setEnabled(false);
        //ui->treeView->setEnabled(false);
    }
    
    document = newDocument;
    
    if (document)
    {
        ui->treeView->setModel(document->GetTreeContext()->proxyModel);
        ui->treeView->expandToDepth(0);
        ui->treeView->setColumnWidth(0, ui->treeView->size().width());

        ui->filterLine->setText(document->GetTreeContext()->filterString);
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        //ui->filterLine->setEnabled(true);
        //ui->treeView->setEnabled(true);
    }
}

void PackageDockWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    QList<ControlNode*> selectedRootControl;
    QList<ControlNode*> deselectedRootControl;
    
    QList<ControlNode*> selectedControl;
    QList<ControlNode*> deselectedControl;
    
    QItemSelection selected = document->GetTreeContext()->proxyModel->mapSelectionToSource(proxySelected);
    QItemSelection deselected = document->GetTreeContext()->proxyModel->mapSelectionToSource(proxyDeselected);
    
    QModelIndexList selectedIndexList = selected.indexes();
    if (!selectedIndexList.empty())
    {
        foreach(QModelIndex index, selectedIndexList)
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->GetControl())
            {
                selectedControl.push_back(static_cast<ControlNode*>(node));
                
                while (node->GetParent() && node->GetParent()->GetControl())
                    node = node->GetParent();
                
                if (selectedRootControl.indexOf(static_cast<ControlNode*>(node)) < 0)
                    selectedRootControl.push_back(static_cast<ControlNode*>(node));
            }
        }
    }
    
    QModelIndexList deselectedIndexList = deselected.indexes();
    if (!selectedIndexList.empty())
    {
        foreach(QModelIndex index, deselectedIndexList)
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->GetControl())
            {
                deselectedControl.push_back(static_cast<ControlNode*>(node));
                
                while(node->GetParent() && node->GetParent()->GetControl())
                    node = node->GetParent();
                
                if (deselectedRootControl.indexOf(static_cast<ControlNode*>(node)) < 0)
                    deselectedRootControl.push_back(static_cast<ControlNode*>(node));
            }
        }
    }

    emit SelectionRootControlChanged(selectedRootControl, deselectedRootControl);
    emit SelectionControlChanged(selectedControl, deselectedControl);
}

void PackageDockWidget::OnRemove()
{
    QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();
    if (!list.empty())
    {
        QModelIndex &index = list.first();
        QModelIndex srcIndex = document->GetTreeContext()->proxyModel->mapToSource(index);
        ControlNode *sourceNode = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(srcIndex.internalPointer()));
        UIPackageModel *model = document->GetTreeContext()->model;
        if (sourceNode && (sourceNode->GetCreationType() == ControlNode::CREATED_FROM_CLASS || sourceNode->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE))
        {
            RemoveControlNodeCommand *cmd = new RemoveControlNodeCommand(model, srcIndex.row(), srcIndex.parent());
            document->UndoStack()->push(cmd);
        }
    }
}

void PackageDockWidget::filterTextChanged(const QString &filterText)
{
    if (document)
    {
        document->GetTreeContext()->proxyModel->setFilterFixedString(filterText);
        ui->treeView->expandAll();
    }
}
