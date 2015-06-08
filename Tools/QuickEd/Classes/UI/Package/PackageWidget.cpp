/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include <QClipboard>

#include "PackageWidget.h"
#include "PackageModel.h"

#include "UI/QtModelPackageCommandExecutor.h"

#include "UI/Package/FilteredPackageModel.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/YamlPackageSerializer.h"
#include "SharedData.h"
#include "EditorCore.h"
#include "Document.h"

#include "QtTools/FileDialog/FileDialog.h"

using namespace DAVA;

namespace
{
    struct PackageContext : public WidgetContext
    {
        PackageContext(Document *document)
        {
            DVASSERT(nullptr != document);
            packageModel = new PackageModel(document->GetPackage(), document->GetCommandExecutor(), document);
            filteredPackageModel = new FilteredPackageModel(document);
            filteredPackageModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            filteredPackageModel->setSourceModel(packageModel);
        }
        PackageModel *packageModel;
        FilteredPackageModel *filteredPackageModel;
        QList<QPersistentModelIndex> expandedIndexes;
        QItemSelection selection;
        QString filterString;
    };
}

PackageWidget::PackageWidget(QWidget *parent)
    : QDockWidget(parent)
    , sharedData(nullptr)
    , filteredPackageModel(nullptr)
    , packageModel(nullptr)
{
    setupUi(this);
    treeView->header()->setSectionResizeMode/*setResizeMode*/(QHeaderView::ResizeToContents);

    connect(filterLine, &QLineEdit::textChanged, this, &PackageWidget::filterTextChanged);

    importPackageAction = new QAction(tr("Import package"), this);
    importPackageAction->setShortcut(QKeySequence::New);
    importPackageAction->setShortcutContext(Qt::WidgetShortcut);
    connect(importPackageAction, &QAction::triggered, this, &PackageWidget::OnImport);


    cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setShortcutContext(Qt::WidgetShortcut);
    connect(cutAction, &QAction::triggered, this, &PackageWidget::OnCut);

    copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setShortcutContext(Qt::WidgetShortcut);
    connect(copyAction, &QAction::triggered, this, &PackageWidget::OnCopy);

    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(pasteAction, &QAction::triggered, this, &PackageWidget::OnPaste);

    delAction = new QAction(tr("Delete"), this);
    delAction->setShortcut(QKeySequence::Delete);
    delAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delAction, &QAction::triggered, this, &PackageWidget::OnDelete);

    treeView->addAction(importPackageAction);
    treeView->addAction(copyAction);
    treeView->addAction(pasteAction);
    treeView->addAction(cutAction);
    treeView->addAction(delAction);
}

void PackageWidget::OnDocumentChanged(SharedData *context)
{
    treeView->setUpdatesEnabled(false);

    SaveContext();
    sharedData = context;

    LoadContext();

    treeView->setColumnWidth(0, treeView->size().width()); // TODO Check this
    treeView->setUpdatesEnabled(true);
}

void PackageWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "selectedNodes")
    {
        OnControlSelectedInEditor(sharedData->GetData("selectedNodes").value<QList<ControlNode*> >());
    }
}

void PackageWidget::LoadContext()
{
    delete treeView->selectionModel();
    if (nullptr == sharedData)
    {
        treeView->setModel(nullptr);
        packageModel = nullptr;
        filteredPackageModel = nullptr;
    }
    else
    {
        //restore context
        PackageContext *context = reinterpret_cast<PackageContext*>(sharedData->GetContext(this));
        if (nullptr == context)
        {
            context = new PackageContext(qobject_cast<Document*>(sharedData->parent()));
            connect(context->packageModel, &PackageModel::rowsInserted, this, &PackageWidget::OnRowsInserted);
            connect(context->packageModel, &PackageModel::rowsAboutToBeRemoved, this, &PackageWidget::OnRowsAboutToBeRemoved);
            sharedData->SetContext(this, context);
        }
        //store model to work with indexes
        packageModel = context->packageModel;
        filteredPackageModel = context->filteredPackageModel;

        //restore model
        treeView->setModel(context->filteredPackageModel);
        treeView->expandToDepth(0);
        connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChanged);
        //restore expanded indexes
        for (const auto &index : context->expandedIndexes)
        {
            if (index.isValid())
            {
                treeView->setExpanded(index, true);
            }
        }
        //restore selection
        treeView->selectionModel()->select(context->selection, QItemSelectionModel::ClearAndSelect);
        //restore filter line
        filterLine->setText(context->filterString);
    }

}

void PackageWidget::SaveContext()
{
    if (nullptr == sharedData)
    {
        return;
    }
    PackageContext *context = reinterpret_cast<PackageContext*>(sharedData->GetContext(this));
    context->expandedIndexes = GetExpandedIndexes();
    context->selection = treeView->selectionModel()->selection();
    context->filterString = filterLine->text();
}

void PackageWidget::RefreshActions(const QModelIndexList &indexList)
{
    bool canInsertControls = !indexList.empty();
    bool canInsertPackages = !indexList.empty();
    bool canRemove = !indexList.empty();
    bool canCopy = !indexList.empty();
    
    for(const auto &index : indexList)
    {
        PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
        canCopy &= node->CanCopy();
        canInsertControls &= node->IsInsertingControlsSupported();
        canInsertPackages &= node->IsInsertingPackagesSupported();
        canRemove &= node->CanRemove();
        if (!canCopy && !canInsertControls && !canRemove && !canInsertPackages)
        {
            break;
        }
    }
    
    RefreshAction(copyAction, canCopy, true);
    RefreshAction(pasteAction, canInsertControls, true);
    RefreshAction(cutAction, canCopy && canRemove, true);
    RefreshAction(delAction, canRemove, true);

    RefreshAction(importPackageAction, canInsertPackages, true);

}

void PackageWidget::RefreshAction( QAction *action, bool enabled, bool visible )
{
    action->setDisabled(!enabled);
    action->setVisible(visible);
}

void PackageWidget::CollectSelectedNodes(Vector<ControlNode*> &nodes)
{
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
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
        serializer.SerializePackageNodes(sharedData->GetDocument()->GetPackage(), nodes);
        String str = serializer.WriteToString();
        QMimeData *data = new QMimeData();
        data->setText(QString(str.c_str()));
        clipboard->setMimeData(data);
    }
}

void PackageWidget::RemoveNodes(const DAVA::Vector<ControlNode*> &nodes)
{
    sharedData->GetDocument()->GetCommandExecutor()->RemoveControls(nodes);
}

void PackageWidget::OnRowsInserted(const QModelIndex &parent, int first, int last)
{
    ControlsContainerNode *parentNode = dynamic_cast<ControlsContainerNode*>(static_cast<PackageBaseNode*>(parent.internalPointer()));
    PackageControlsNode* packageControlsNode = dynamic_cast<PackageControlsNode*>(parentNode);
    if (nullptr != packageControlsNode)
    {
        QList<ControlNode*> insertedNodes;
        for (int i = first; i < last; ++i)
        {
            insertedNodes.append(packageControlsNode->Get(i));
        }
        sharedData->SetData("activeRootControls", QVariant::fromValue(insertedNodes));
        OnControlSelectedInEditor(insertedNodes);
    }
}

void PackageWidget::OnRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    ControlsContainerNode *parentNode = dynamic_cast<ControlsContainerNode*>(static_cast<PackageBaseNode*>(parent.internalPointer()));
    PackageControlsNode* packageControlsNode = dynamic_cast<PackageControlsNode*>(parentNode);
    if (nullptr != packageControlsNode)
    {
        QList<ControlNode*> rootControls = sharedData->GetData("activeRootControls").value<QList<ControlNode*> >();
        for (int i = first; i < last; ++i)
        {
            rootControls.removeAt(first);
        }
        sharedData->SetData("activeRootControls", QVariant::fromValue(rootControls));
    }
}

void PackageWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    if (filteredPackageModel.isNull())
    {
        return;
    }
    //check selected root controls
    QList<ControlNode*> selectedRootControls;
    const QItemSelection &allSelectedElements = treeView->selectionModel()->selection();
    const QItemSelection &filteredSelectedElements = filteredPackageModel->mapSelectionToSource(allSelectedElements);
    for (const QModelIndex &index : filteredSelectedElements.indexes())
    {
        PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
        if (node->GetControl())
        {
            while (node->GetParent() && node->GetParent()->GetControl())
                node = node->GetParent();
            if (selectedRootControls.indexOf(static_cast<ControlNode*>(node)) < 0)
                selectedRootControls.push_back(static_cast<ControlNode*>(node));
        }
    }

    QList<ControlNode*> selectedControls;
    QList<ControlNode*> deselectedControls;

    QItemSelection selected = filteredPackageModel->mapSelectionToSource(proxySelected);
    QItemSelection deselected = filteredPackageModel->mapSelectionToSource(proxyDeselected);

    QModelIndexList deselectedIndexList = deselected.indexes();
    for (QModelIndex &index : deselectedIndexList)
    {
        PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
        if (node->GetControl())
        {
            deselectedControls.push_back(static_cast<ControlNode*>(node));
        }
    }

    QModelIndexList selectedIndexList = selected.indexes();
    for (QModelIndex &index : selectedIndexList)
    {
        PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
        if (node->GetControl())
        {
            selectedControls.push_back(static_cast<ControlNode*>(node));
        }
    }

    RefreshActions(selectedIndexList);
    sharedData->SetData("activeRootControls", QVariant::fromValue(selectedRootControls));
    sharedData->SetData("activatedControls", QVariant::fromValue(selectedControls));
    sharedData->SetData("deactivatedControls", QVariant::fromValue(deselectedControls));
}

void PackageWidget::OnImport()
{
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    if (selectedIndexList.isEmpty())
    {
        return;
    }
    QStringList fileNames = FileDialog::getOpenFileNames(
        qApp->activeWindow()
        , tr("Select one or move files to import")
        , QString::fromStdString(sharedData->GetDocument()->GetPackageFilePath().GetDirectory().GetStringValue())
        , "Packages (*.yaml)"
        );
    if (fileNames.isEmpty())
    {
        return;
    }

    Document *doc = sharedData->GetDocument();
    PackageNode *root = doc->GetPackage();
    for (const auto &fileName : fileNames)
    {
        FilePath path(fileName.toStdString());
        doc->GetCommandExecutor()->AddImportedPackageIntoPackage(path, root);
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
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    QClipboard *clipboard = QApplication::clipboard();
    
    if (!selectedIndexList.empty() && clipboard && clipboard->mimeData())
    {
        const QModelIndex &index = selectedIndexList.first();
        
        PackageBaseNode *baseNode = static_cast<PackageBaseNode*>(index.internalPointer());
        ControlsContainerNode *node = dynamic_cast<ControlsContainerNode*>(baseNode);
        
        if (node != nullptr && !node->IsReadOnly())
        {
            String string = clipboard->mimeData()->text().toStdString();
            Document *doc = sharedData->GetDocument();
            doc->GetCommandExecutor()->Paste(doc->GetPackage(), node, node->GetCount(), string);
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
    if (nullptr != sharedData)
    {
        static_cast<QSortFilterProxyModel*>(treeView->model())->setFilterFixedString(filterText);
        treeView->expandAll();
    }
}

void PackageWidget::OnControlSelectedInEditor(const QList<ControlNode *> &selectedNodes)
{
    treeView->selectionModel()->clear();
    for (auto &node : selectedNodes)
    {
        QModelIndex srcIndex = packageModel->indexByNode(node);
        QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
        treeView->selectionModel()->select(dstIndex, QItemSelectionModel::Select);
        treeView->expand(dstIndex);
        treeView->scrollTo(dstIndex);
    }
}

QList<QPersistentModelIndex> PackageWidget::GetExpandedIndexes() const
{
    QList<QPersistentModelIndex> retval;
    QModelIndex index = treeView->model()->index(0, 0);
    while (index.isValid())
    {
        if (treeView->isExpanded(index))
        {
            retval << QPersistentModelIndex(index);
        }
        index = treeView->indexBelow(index);
    }

    return retval;
}
