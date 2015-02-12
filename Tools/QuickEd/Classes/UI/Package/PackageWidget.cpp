#include "PackageWidget.h"

#include <QClipboard>
#include <QFileDialog>

#include "ui_PackageWidget.h"
#include "UIPackageModel.h"

#include "UI/QtModelPackageCommandExecutor.h"

#include "UI/Package/UIFilteredPackageModel.h"
#include "UI/Document.h"
#include "UI/PackageContext.h"
#include "UI/Package/PackageModelCommands.h"
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

PackageWidget::PackageWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PackageWidget())
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

PackageWidget::~PackageWidget()
{
    disconnect(ui->filterLine, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged(const QString &)));
    ui->treeView->setModel(NULL);
    delete ui;
    ui = NULL;
}

void PackageWidget::SetDocument(Document *newDocument)
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
        ui->treeView->setModel(document->GetPackageContext()->GetFilterProxyModel());
        ui->treeView->selectionModel()->select(*document->GetPackageContext()->GetCurrentSelection(), QItemSelectionModel::ClearAndSelect);
        ui->treeView->expandToDepth(0);
        ui->treeView->setColumnWidth(0, ui->treeView->size().width());

        ui->filterLine->setText(document->GetPackageContext()->GetFilterString());
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        //ui->filterLine->setEnabled(true);
        //ui->treeView->setEnabled(true);
    }
}

void PackageWidget::RefreshActions(const QModelIndexList &indexList)
{
    bool editActionEnabled = !indexList.empty();
    bool editActionVisible = editActionEnabled;

    bool editImportPackageEnabled = !indexList.empty();
    bool editImportPackageVisible = editImportPackageEnabled;

    bool editControlsEnabled = !indexList.empty();
    //bool editControlsVisible = editControlsEnabled;

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

void PackageWidget::RefreshAction( QAction *action, bool enabled, bool visible )
{
    action->setDisabled(!enabled);
    action->setVisible(visible);
}

void PackageWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    QList<ControlNode*> selectedRootControl;
    QList<ControlNode*> deselectedRootControl;
    
    QList<ControlNode*> selectedControl;
    QList<ControlNode*> deselectedControl;
    
    QItemSelection selected = document->GetPackageContext()->GetFilterProxyModel()->mapSelectionToSource(proxySelected);
    QItemSelection deselected = document->GetPackageContext()->GetFilterProxyModel()->mapSelectionToSource(proxyDeselected);
    
    QItemSelection *currentSelection = document->GetPackageContext()->GetCurrentSelection();
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

void PackageWidget::OnImport()
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

void PackageWidget::OnCopy()
{
    QItemSelection selected = document->GetPackageContext()->GetFilterProxyModel()->mapSelectionToSource(ui->treeView->selectionModel()->selection());
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

void PackageWidget::OnPaste()
{
    QItemSelection selected = document->GetPackageContext()->GetFilterProxyModel()->mapSelectionToSource(ui->treeView->selectionModel()->selection());
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

void PackageWidget::OnCut()
{

}

void PackageWidget::OnDelete()
{
    QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();
    if (!list.empty())
    {
        QModelIndex &index = list.first();
        QModelIndex srcIndex = document->GetPackageContext()->GetFilterProxyModel()->mapToSource(index);
        ControlNode *sourceNode = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(srcIndex.internalPointer()));
        UIPackageModel *model = document->GetPackageContext()->GetModel();
        if (sourceNode && (sourceNode->GetCreationType() == ControlNode::CREATED_FROM_CLASS || sourceNode->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE))
        {
            RemoveControlNodeCommand *cmd = new RemoveControlNodeCommand(model, srcIndex.row(), srcIndex.parent());
            document->UndoStack()->push(cmd);
        }
    }
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
