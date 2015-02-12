//
//  PackageTreeWidget.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 10.9.14.
//
//

#include "PackageDockWidget.h"

#include <QClipboard>
#include <QFileDialog>

#include "ui_PackageDockWidget.h"
#include "UIPackageModel.h"

#include "UI/QtModelPackageCommandExecutor.h"

#include "UI/PackageView/UIFilteredPackageModel.h"
#include "UI/PackageDocument.h"
#include "UI/PackageView/PackageModelCommands.h"
#include "UIControls/PackageHierarchy/PackageBaseNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/PackageNode.h"
#include "UIControls/PackageHierarchy/ImportedPackagesNode.h"
#include "UIControls/PackageHierarchy/PackageControlsNode.h"
#include "UIControls/PackageHierarchy/PackageRef.h"
#include "UIControls/YamlPackageSerializer.h"
#include "UIControls/EditorUIPackageBuilder.h"

#include "Project.h"
#include "Utils/QtDavaConvertion.h"

using namespace DAVA;

PackageDockWidget::PackageDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PackageDockWidget())
    , document(NULL)
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
        ui->treeView->selectionModel()->select(*document->GetTreeContext()->currentSelection, QItemSelectionModel::ClearAndSelect);
        ui->treeView->expandToDepth(0);
        ui->treeView->setColumnWidth(0, ui->treeView->size().width());

        ui->filterLine->setText(document->GetTreeContext()->filterString);
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        //ui->filterLine->setEnabled(true);
        //ui->treeView->setEnabled(true);
    }
}

void PackageDockWidget::RefreshActions(const QModelIndexList &indexList)
{
    bool editActionEnabled = !indexList.empty();
    bool editActionVisible = editActionEnabled;

    bool editImportPackageEnabled = !indexList.empty();
    bool editImportPackageVisible = editImportPackageEnabled;

    bool editControlsEnabled = !indexList.empty();
    bool editControlsVisible = editControlsEnabled;

    foreach(QModelIndex index, indexList)
    {
        PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());

        if (!node->GetControl())
        {
            editActionEnabled &= false;
            editActionVisible &= false;
        }
        else
        {
            if ((node->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) != 0)
            {
                editActionEnabled &= false;
            }
        }

        ImportedPackagesNode *importNode = dynamic_cast<ImportedPackagesNode *>(node);
        if (!importNode)
        {
            editImportPackageEnabled &= false;
            editImportPackageVisible &= false;
        }

        PackageControlsNode *controlsNode = dynamic_cast<PackageControlsNode *>(node);
        if (controlsNode)
        {
            editControlsEnabled &= false;
            editControlsEnabled &= false;
        }
    }

    RefreshAction(copyAction , editActionEnabled, editActionVisible);
    RefreshAction(pasteAction, editActionEnabled, editActionVisible);
    RefreshAction(cutAction  , editActionEnabled, editActionVisible);
    RefreshAction(delAction  , editActionEnabled, editActionVisible);

    RefreshAction(importPackageAction, editImportPackageEnabled, editImportPackageVisible);
}

void PackageDockWidget::RefreshAction( QAction *action, bool enabled, bool visible )
{
    action->setDisabled(!enabled);
    action->setVisible(visible);
}

void PackageDockWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    QList<ControlNode*> selectedRootControl;
    QList<ControlNode*> deselectedRootControl;
    
    QList<ControlNode*> selectedControl;
    QList<ControlNode*> deselectedControl;
    
    QItemSelection selected = document->GetTreeContext()->proxyModel->mapSelectionToSource(proxySelected);
    QItemSelection deselected = document->GetTreeContext()->proxyModel->mapSelectionToSource(proxyDeselected);
    
    QItemSelection *currentSelection = document->GetTreeContext()->currentSelection;
    currentSelection->merge(deselected, QItemSelectionModel::Deselect);
    currentSelection->merge(selected, QItemSelectionModel::Select);
    
    QModelIndexList selectedIndexList = currentSelection->indexes();
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

//    QModelIndexList deselectedIndexList = deselected.indexes();
//    if (!selectedIndexList.empty())
//    {
//        foreach(QModelIndex index, deselectedIndexList)
//        {
//            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
//            if (node->GetControl())
//            {
//                deselectedControl.push_back(static_cast<ControlNode*>(node));
//                
//                while(node->GetParent() && node->GetParent()->GetControl())
//                    node = node->GetParent();
//                
//                if (deselectedRootControl.indexOf(static_cast<ControlNode*>(node)) < 0)
//                    deselectedRootControl.push_back(static_cast<ControlNode*>(node));
//            }
//        }
//    }

    RefreshActions(selectedIndexList);

    if (selectedRootControl != deselectedRootControl)
    {
        emit SelectionRootControlChanged(selectedRootControl, deselectedRootControl);
    }
    emit SelectionControlChanged(selectedControl, deselectedControl);
}

void PackageDockWidget::OnImport()
{
    return;
    QString dir;

    //QString pathText = lineEdit->text();
    const DAVA::FilePath &filePath = document->PackageFilePath();

    if (!filePath.IsEmpty())
    {
        dir = StringToQString(filePath.GetDirectory().GetAbsolutePathname());
    }
    else
    {
        //dir = ResourcesManageHelper::GetSpritesDirectory();
    }

    QString filePathText = QFileDialog::getOpenFileName(this, tr("Select package to import"), dir, QString("*.yaml"));
    if (!filePathText.isEmpty())
    {
        //ImportedPackagesNode *node = document->GetPackage()->GetImportedPackagesNode();
        //node->Add(NULL);
    }
}

void PackageDockWidget::OnCopy()
{
    QItemSelection selected = document->GetTreeContext()->proxyModel->mapSelectionToSource(ui->treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    QClipboard *clipboard = QApplication::clipboard();

    Vector<ControlNode*> nodes;
    if (!selectedIndexList.empty())
    {
        for (QModelIndex &index : selectedIndexList)
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
            
            if (controlNode && controlNode->GetCreationType() != ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
                nodes.push_back(controlNode);
        }

        YamlPackageSerializer serializer;
        document->GetPackage()->Serialize(&serializer, nodes);
        String str = serializer.WriteToString();
        QMimeData data;
        data.setText(QString(str.c_str()));
        clipboard->setMimeData(&data);
    }
}

void PackageDockWidget::OnPaste()
{
    QItemSelection selected = document->GetTreeContext()->proxyModel->mapSelectionToSource(ui->treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    QClipboard *clipboard = QApplication::clipboard();
    
    if (!selectedIndexList.empty() && clipboard && clipboard->mimeData())
    {
        QModelIndex &index = selectedIndexList.first();
        
        PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
        ControlNode *controlNode = dynamic_cast<ControlNode*>(node); // control node may be null
        
        String string = clipboard->mimeData()->text().toStdString();
        RefPtr<YamlParser> parser(YamlParser::CreateAndParseString(string));
        
        if (parser.Valid() && parser->GetRootNode())
        {
            document->UndoStack()->beginMacro("Paste");
            EditorUIPackageBuilder builder(document->GetPackage(), controlNode, document->GetCommandExecutor());
            UIPackage *newPackage = UIPackageLoader(&builder).LoadPackage(parser->GetRootNode(), "");
            SafeRelease(newPackage);
            document->UndoStack()->endMacro();
        }
    }
}

void PackageDockWidget::OnCut()
{

}

void PackageDockWidget::OnDelete()
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

void PackageDockWidget::OnControlSelectedInEditor(ControlNode *node)
{
    QModelIndex srcIndex = document->GetTreeContext()->model->indexByNode(node);
    QModelIndex dstIndex = document->GetTreeContext()->proxyModel->mapFromSource(srcIndex);
    ui->treeView->selectionModel()->select(dstIndex, QItemSelectionModel::ClearAndSelect);
    ui->treeView->expand(dstIndex);
    ui->treeView->scrollTo(dstIndex);
}

void PackageDockWidget::OnAllControlsDeselectedInEditor()
{
    
}
