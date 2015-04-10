#include "PackageWidget.h"

#include <QClipboard>
#include <QFileDialog>

#include "ui_PackageWidget.h"
#include "PackageModel.h"

#include "UI/QtModelPackageCommandExecutor.h"

#include "UI/Package/FilteredPackageModel.h"
#include "UI/Document.h"
#include "UI/PackageContext.h"
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

using namespace DAVA;

PackageWidget::PackageWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PackageWidget())
    , document(nullptr)
{
    ui->setupUi(this);
    ui->treeView->header()->setSectionResizeMode/*setResizeMode*/(QHeaderView::ResizeToContents);

    connect(ui->filterLine, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged(const QString &)));

    importPackageAction = new QAction(tr("Import package"), this);
    connect(importPackageAction, SIGNAL(triggered()), this, SLOT(OnImport()));

    cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence(QKeySequence::Cut));
    cutAction->setShortcutContext(Qt::WidgetShortcut);
    connect(cutAction, SIGNAL(triggered()), this, SLOT(OnCut()));

    copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence(QKeySequence::Copy));
    copyAction->setShortcutContext(Qt::WidgetShortcut);
    connect(copyAction, SIGNAL(triggered()), this, SLOT(OnCopy()));

    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence(QKeySequence::Paste));
    pasteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(OnPaste()));

    delAction = new QAction(tr("Delete"), this);
    delAction->setShortcut(QKeySequence(QKeySequence::Delete));
    delAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delAction, SIGNAL(triggered()), this, SLOT(OnDelete()));

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

void PackageWidget::SetDocument(Document *newDocument)
{
    if (document)
    {
        document->GetPackageContext()->SetCurrentItemSelection(ui->treeView->selectionModel()->selection());
        document->GetPackageContext()->SetExpandedIndexes(GetExpandedIndexes());
        disconnect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        ui->treeView->setModel(nullptr);
    }
    
    document = newDocument;
    
    if (document)
    {
        ui->treeView->setUpdatesEnabled(false);
        ui->treeView->setModel(document->GetPackageContext()->GetFilterProxyModel());
        for(const auto &index : document->GetPackageContext()->GetExpandedIndexes())
        {
            if (index.isValid())
            {
                ui->treeView->setExpanded(index, true);
            }
        }
        ui->treeView->setColumnWidth(0, ui->treeView->size().width());
        ui->filterLine->setText(document->GetPackageContext()->GetFilterString());
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        ui->treeView->selectionModel()->select(document->GetPackageContext()->GetCurrentItemSelection(), QItemSelectionModel::ClearAndSelect);
        ui->treeView->setUpdatesEnabled(true);
    }
}

void PackageWidget::RefreshActions(const QModelIndexList &indexList)
{
//    bool editActionEnabled = !indexList.empty();
//    bool editActionVisible = editActionEnabled;
//
//    bool editImportPackageEnabled = !indexList.empty();
//    bool editImportPackageVisible = editImportPackageEnabled;
//
//    bool editControlsEnabled = !indexList.empty();
    //bool editControlsVisible = editControlsEnabled;

//    for (QModelIndex index : indexList)
//    {
//        PackageBaseNode *node  = static_cast<PackageBaseNode*>(index.internalPointer());
//        if (!node->GetControl())
//        {
//            editActionEnabled &= false;
//            editActionVisible &= false;
//        }
//        else
//        {
//            if ((node->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) != 0)
//            {
//                editActionEnabled &= false;
//            }
//        }
//
//        ImportedPackagesNode *importNode = dynamic_cast<ImportedPackagesNode *>(node);
//        if (!importNode)
//        {
//            editImportPackageEnabled &= false;
//            editImportPackageVisible &= false;
//        }
//
//        PackageControlsNode *controlsNode = dynamic_cast<PackageControlsNode *>(node);
//        if (controlsNode)
//        {
//            editControlsEnabled &= false;
//            editControlsEnabled &= false;
//        }
//    }

//    RefreshAction(copyAction , editActionEnabled, editActionVisible);
//    RefreshAction(pasteAction, editActionEnabled, editActionVisible);
//    RefreshAction(cutAction  , editActionEnabled, editActionVisible);
//    RefreshAction(delAction  , editActionEnabled, editActionVisible);
//
//    RefreshAction(importPackageAction, editImportPackageEnabled, editImportPackageVisible);
    
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
    QItemSelection selected = document->GetPackageContext()->GetFilterProxyModel()->mapSelectionToSource(ui->treeView->selectionModel()->selection());
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
        document->GetPackage()->Serialize(&serializer, nodes);
        String str = serializer.WriteToString();
        QMimeData *data = new QMimeData();
        data->setText(QString(str.c_str()));
        clipboard->setMimeData(data);
    }
}

void PackageWidget::RemoveNodes(const DAVA::Vector<ControlNode*> &nodes)
{
    document->GetCommandExecutor()->RemoveControls(nodes);
}

void PackageWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    QList<ControlNode*> selectedRootControl;
    QList<ControlNode*> deselectedRootControl;
    
    QList<ControlNode*> selectedControl;
    QList<ControlNode*> deselectedControl;
    
    QItemSelection selected = document->GetPackageContext()->GetFilterProxyModel()->mapSelectionToSource(proxySelected);
    QItemSelection deselected = document->GetPackageContext()->GetFilterProxyModel()->mapSelectionToSource(proxyDeselected);
    
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

    if (!selectedRootControl.empty() || !deselectedRootControl.empty())
    {
        emit SelectionRootControlChanged(selectedRootControl, deselectedRootControl);
    }
    emit SelectionControlChanged(selectedControl, deselectedControl);
}

void PackageWidget::OnImport()
{
    return;
    QString dir;

    const DAVA::FilePath &filePath = document->PackageFilePath();

    if (!filePath.IsEmpty())
    {
        dir = StringToQString(filePath.GetDirectory().GetAbsolutePathname());
    }
}

void PackageWidget::OnCopy()
{
    Vector<ControlNode*> nodes;
    CollectSelectedNodes(nodes);
    CopyNodesToClipboard(nodes);
}

void PackageWidget::OnPaste()
{
    QItemSelection selected = document->GetPackageContext()->GetFilterProxyModel()->mapSelectionToSource(ui->treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    QClipboard *clipboard = QApplication::clipboard();
    
    if (!selectedIndexList.empty() && clipboard && clipboard->mimeData())
    {
        const QModelIndex &index = selectedIndexList.first();
        
        PackageBaseNode *baseNode = static_cast<PackageBaseNode*>(index.internalPointer());
        ControlsContainerNode *node = dynamic_cast<ControlsContainerNode*>(baseNode);
        
        if (nullptr != node && (node->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
        {
            String string = clipboard->mimeData()->text().toStdString();
            document->GetCommandExecutor()->Paste(document->GetPackage(), node, -1, string);
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
    if (document)
    {
        document->GetPackageContext()->GetFilterProxyModel()->setFilterFixedString(filterText);
        ui->treeView->expandAll();
    }
}

void PackageWidget::OnControlSelectedInEditor(ControlNode *node)
{
    QModelIndex srcIndex = document->GetPackageContext()->GetModel()->indexByNode(node);
    QModelIndex dstIndex = document->GetPackageContext()->GetFilterProxyModel()->mapFromSource(srcIndex);
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
