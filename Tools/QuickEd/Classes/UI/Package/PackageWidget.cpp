
#include <QClipboard>
#include <QFileDialog>

#include "ui_PackageWidget.h"
#include "PackageModel.h"

#include "UI/QtModelPackageCommandExecutor.h"

#include "UI/Package/FilteredPackageModel.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageRef.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/EditorUIPackageBuilder.h"

#include "Project.h"
#include "Utils/QtDavaConvertion.h"

#include "UI/WidgetContext.h"

using namespace DAVA;

PackageWidget::PackageWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PackageWidget())
    , widgetContext(nullptr)
    , proxyModel(nullptr)
{
    ui->setupUi(this);
    ui->treeView->header()->setSectionResizeMode/*setResizeMode*/(QHeaderView::ResizeToContents);

    connect(ui->filterLine, &QLineEdit::textChanged, this, &PackageWidget::filterTextChanged);

    importPackageAction = new QAction(tr("Import package"), this);

    cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence(QKeySequence::Cut));
    cutAction->setShortcutContext(Qt::WidgetShortcut);
    connect(cutAction, &QAction::triggered, this, &PackageWidget::OnCut);

    copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence(QKeySequence::Copy));
    copyAction->setShortcutContext(Qt::WidgetShortcut);
    connect(copyAction, &QAction::triggered, this, &PackageWidget::OnCopy);

    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence(QKeySequence::Paste));
    pasteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(pasteAction, &QAction::triggered, this, &PackageWidget::OnPaste);

    delAction = new QAction(tr("Delete"), this);
    delAction->setShortcut(QKeySequence(QKeySequence::Delete));
    delAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delAction, &QAction::triggered, this, &PackageWidget::OnDelete);

    ui->treeView->addAction(importPackageAction);
    ui->treeView->addAction(copyAction);
    ui->treeView->addAction(pasteAction);
    ui->treeView->addAction(cutAction);
    ui->treeView->addAction(delAction);
}

PackageWidget::~PackageWidget()
{
    delete ui;
} 

void PackageWidget::OnContextChanged(WidgetContext *context)
{
    ui->treeView->setUpdatesEnabled(false);

    if (nullptr != widgetContext)
    {
        SaveExpanded();
        SaveSelection();
        SaveFilterString();
    }
    widgetContext = context;
    if (nullptr == widgetContext)
    {
        OnAllControlsDeselectedInEditor();
    }
    UpdateModel();
    UpdateExpanded();
    UpdateSelection();
    UpdateFilterString();
    ui->treeView->setColumnWidth(0, ui->treeView->size().width());
    ui->treeView->setUpdatesEnabled(true);
}

void PackageWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "packageModel")
    {
        UpdateModel();
    }
    if (role == "selectedNode")
    {
        OnControlSelectedInEditor(widgetContext->GetData("selectedNode").value<ControlNode*>());
    }
    if (role == "controlDeselected")
    {
        OnAllControlsDeselectedInEditor();
    }
}

void PackageWidget::UpdateModel()
{
    FilteredPackageModel *oldModel = proxyModel.data();
    if (nullptr != widgetContext)
    {
        QAbstractItemModel *model = widgetContext->GetData("packageModel").value<QAbstractItemModel*>();
        DVASSERT(model);
        proxyModel = new FilteredPackageModel(this);
        proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxyModel->setSourceModel(model);
        ui->treeView->setModel(proxyModel.data());
        ui->treeView->expandToDepth(0);
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
    }
    else
    {
        proxyModel = nullptr;
        ui->treeView->setModel(proxyModel);
    }
    delete oldModel;
}

void PackageWidget::UpdateSelection()
{
    if (nullptr == widgetContext)
    {
        //nothing to do here
    }
    else
    {
        QItemSelection selection = widgetContext->GetData("selection").value<QItemSelection>();
        ui->treeView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    }
}

void PackageWidget::UpdateExpanded()
{
    if (nullptr == widgetContext)
    {
        //nothing to do here
    }
    else
    {
        const QList<QPersistentModelIndex> indexList = widgetContext->GetData("expanded").value<QList<QPersistentModelIndex> >();
        for (const auto &index : indexList)
        {
            if (index.isValid())
            {
                ui->treeView->setExpanded(index, true);
            }
        }
    }
}

void PackageWidget::UpdateFilterString()
{
    if (nullptr == widgetContext)
    {
        ui->filterLine->clear();
    }
    else
    {
        ui->filterLine->setText(widgetContext->GetData("filterString").toString());
    }
}

void PackageWidget::SaveSelection()
{
    widgetContext->SetData(QVariant::fromValue(ui->treeView->selectionModel()->selection()), "selection");
}

void PackageWidget::SaveExpanded()
{
    widgetContext->SetData(QVariant::fromValue(GetExpandedIndexes()), "expanded");
}

void PackageWidget::SaveFilterString()
{
    widgetContext->SetData(ui->filterLine->text(), "filterString");
}

void PackageWidget::RefreshActions(const QModelIndexList &indexList)
{
    bool canInsert = !indexList.empty();
    bool canRemove = !indexList.empty();
    bool canCopy = !indexList.empty();
    
    for(const auto &index : indexList)
    {
        PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
        if (!node->CanCopy())
            canCopy = false;

        if (!node->IsInsertingSupported())
            canInsert = false;

        if (!node->CanRemove())
            canRemove = false;
    }
    
    RefreshAction(copyAction, canCopy, true);
    RefreshAction(pasteAction, canInsert, true);
    RefreshAction(cutAction, canCopy && canRemove, true);
    RefreshAction(delAction, canRemove, true);

    RefreshAction(importPackageAction, false, false);

}

void PackageWidget::RefreshAction( QAction *action, bool enabled, bool visible )
{
    action->setDisabled(!enabled);
    action->setVisible(visible);
}

void PackageWidget::CollectSelectedNodes(Vector<ControlNode*> &nodes)
{
    QItemSelection selected = proxyModel->mapSelectionToSource(ui->treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    
    if (!selectedIndexList.empty())
    {
        for (QModelIndex &index : selectedIndexList)
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
            
            if (controlNode && controlNode->CanCopy())
                nodes.push_back(controlNode);
        }
    }
}

void PackageWidget::CopyNodesToClipboard(const DAVA::Vector<ControlNode*> &nodes)
{
    QClipboard *clipboard = QApplication::clipboard();
    if (!nodes.empty())
    {
        YamlPackageSerializer serializer;
        Document* doc = widgetContext->GetDocument();
        PackageNode *pac = doc->GetPackage();
        pac->Serialize(&serializer, nodes);//TODO - this is deprecated
        String str = serializer.WriteToString();
        QMimeData data;
        data.setText(QString(str.c_str()));
        clipboard->setMimeData(&data);
    }
}

void PackageWidget::RemoveNodes(const DAVA::Vector<ControlNode*> &nodes)
{
    widgetContext->GetDocument()->GetCommandExecutor()->RemoveControls(nodes);
}

void PackageWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    if (proxyModel.isNull())
    {
        return;
    }
    QList<ControlNode*> selectedRootControl;
    QList<ControlNode*> deselectedRootControl;
    
    QList<ControlNode*> selectedControl;
    QList<ControlNode*> deselectedControl;

    QItemSelection selected = proxyModel->mapSelectionToSource(proxySelected);
    QItemSelection deselected = proxyModel->mapSelectionToSource(proxyDeselected);

    QModelIndexList selectedIndexList = selected.indexes();

    if (!selectedIndexList.empty())
    {
        for(QModelIndex &index : selectedIndexList)
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
    if (!deselectedIndexList.empty())
    {
        for (QModelIndex &index : deselectedIndexList)
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->GetControl())
            {
                deselectedControl.push_back(static_cast<ControlNode*>(node));

                while (node->GetParent() && node->GetParent()->GetControl())
                    node = node->GetParent();

                if (deselectedRootControl.indexOf(static_cast<ControlNode*>(node)) < 0)
                    deselectedRootControl.push_back(static_cast<ControlNode*>(node));
            }
        }
    }

    RefreshActions(selectedIndexList);
    widgetContext->SetData(QVariant::fromValue(selectedControl), "activatedControls");
    widgetContext->SetData(QVariant::fromValue(deselectedControl), "deactivatedControls");
}

void PackageWidget::OnCopy()
{
    Vector<ControlNode*> nodes;
    CollectSelectedNodes(nodes);
    CopyNodesToClipboard(nodes);
}

void PackageWidget::OnPaste()
{
    QItemSelection selected = proxyModel->mapSelectionToSource(ui->treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    QClipboard *clipboard = QApplication::clipboard();
    
    if (!selectedIndexList.empty() && clipboard && clipboard->mimeData())
    {
        QModelIndex &index = selectedIndexList.first();
        
        ControlsContainerNode *node = dynamic_cast<ControlsContainerNode*>(static_cast<PackageBaseNode*>(index.internalPointer()));
        
        if (nullptr != node && (node->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
        {
            String string = clipboard->mimeData()->text().toStdString();
            Document *doc = widgetContext->GetDocument();
            doc->GetCommandExecutor()->Paste(doc->GetPackage(), node, -1, string);
        }
    }
}

void PackageWidget::OnCut()
{
    Vector<ControlNode*> nodes;
    CollectSelectedNodes(nodes);
    CopyNodesToClipboard(nodes);
    RemoveNodes(nodes);
}

void PackageWidget::OnDelete()
{
    Vector<ControlNode*> nodes;
    CollectSelectedNodes(nodes);
    RemoveNodes(nodes);
}

void PackageWidget::filterTextChanged(const QString &filterText)
{
    if (nullptr != widgetContext)
    {
        static_cast<QSortFilterProxyModel*>(ui->treeView->model())->setFilterFixedString(filterText);
        ui->treeView->expandAll();
    }
}

void PackageWidget::OnControlSelectedInEditor(ControlNode *node)
{
    PackageModel *packageModel = widgetContext->GetData("packageModel").value<PackageModel*>();
    QModelIndex srcIndex = packageModel->indexByNode(node);
    QModelIndex dstIndex = proxyModel->mapFromSource(srcIndex);
    ui->treeView->selectionModel()->select(dstIndex, QItemSelectionModel::ClearAndSelect);
    ui->treeView->expand(dstIndex);
    ui->treeView->scrollTo(dstIndex);
}

void PackageWidget::OnAllControlsDeselectedInEditor()
{
    
}

QList<QPersistentModelIndex> PackageWidget::GetExpandedIndexes() const
{
    QList<QPersistentModelIndex> retval;
    QModelIndex index = ui->treeView->model()->index(0, 0);
    while (index.isValid())
    {
        if (ui->treeView->isExpanded(index))
        {
            retval << QPersistentModelIndex(index);
        }
        index = ui->treeView->indexBelow(index);
    }

    return retval;
}
