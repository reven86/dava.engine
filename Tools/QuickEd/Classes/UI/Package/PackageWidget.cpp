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
#include "EditorCore.h"
#include "Document.h"
#include "UI/Package/FilteredPackageModel.h"
#include "UI/Package/PackageModel.h"

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
        QString filterString;
    };
    
    template <typename NodeType>
    void CollectSelectedNodes(const QItemSelection &selected, Vector<NodeType*> &nodes, bool forCopy, bool forRemove)
    {
        QModelIndexList selectedIndexList = selected.indexes();
        
        if (!selectedIndexList.empty())
        {
            for (QModelIndex &index : selectedIndexList)
            {
                PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
                NodeType *convertedNode = dynamic_cast<NodeType*>(node);
                
                if (convertedNode && node->GetParent() != nullptr)
                {
                    if ((!forCopy || convertedNode->CanCopy()) &&
                        (!forRemove || convertedNode->CanRemove()))
                    {
                        nodes.push_back(convertedNode);
                    }
                }
            }
        }
    }

}



PackageWidget::PackageWidget(QWidget *parent)
    : QDockWidget(parent)
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
    
    renameAction = new QAction(tr("Rename"), this);
    connect(renameAction, &QAction::triggered, this, &PackageWidget::OnRename);
    
    treeView->addAction(importPackageAction);
    treeView->addAction(copyAction);
    treeView->addAction(pasteAction);
    treeView->addAction(cutAction);
    treeView->addAction(delAction);
    treeView->addAction(renameAction);
}

void PackageWidget::OnDocumentChanged(Document *arg)
{
    treeView->setUpdatesEnabled(false);

    SaveContext();
    document = arg;
    LoadContext();

    treeView->setColumnWidth(0, treeView->size().width()); // TODO Check this
    treeView->setUpdatesEnabled(true);
}

void PackageWidget::LoadContext()
{
    delete treeView->selectionModel();
    if (nullptr == document)
    {
        treeView->setModel(nullptr);
        packageModel = nullptr;
        filteredPackageModel = nullptr;
    }
    else
    {
        //restore context
        PackageContext *context = reinterpret_cast<PackageContext*>(document->GetContext(this));
        if (nullptr == context)
        {
            context = new PackageContext(document);
            document->SetContext(this, context);
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
        filterLine->setText(context->filterString);
    }

}

void PackageWidget::SaveContext()
{
    if (nullptr == document)
    {
        return;
    }
    PackageContext *context = reinterpret_cast<PackageContext*>(document->GetContext(this));
    context->expandedIndexes = GetExpandedIndexes();
    context->filterString = filterLine->text();
}

void PackageWidget::RefreshActions()
{
    SelectedNodes nodes;
    QModelIndexList indexes = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection()).indexes();
    for (const QModelIndex &index : indexes)
        nodes.insert(static_cast<PackageBaseNode*>(index.internalPointer()));

    bool canInsertControls = !nodes.empty();
    bool canInsertPackages = !nodes.empty();
    bool canRemove = !nodes.empty();
    bool canCopy = !nodes.empty();
    bool canEdit = nodes.size() == 1 && (*nodes.begin())->IsEditingSupported();
    
    for(const PackageBaseNode *node : nodes)
    {
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
    RefreshAction(renameAction, canEdit, true);
}

void PackageWidget::RefreshAction( QAction *action, bool enabled, bool visible )
{
    action->setDisabled(!enabled);
    action->setVisible(visible);
}

void PackageWidget::CollectSelectedControls(Vector<ControlNode*> &nodes, bool forCopy, bool forRemove)
{
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
    CollectSelectedNodes(selected, nodes, forCopy, forRemove);
}

void PackageWidget::CollectSelectedImportedPackages(Vector<PackageNode*> &nodes, bool forCopy, bool forRemove)
{
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
    CollectSelectedNodes(selected, nodes, forCopy, forRemove);
}

void PackageWidget::CopyNodesToClipboard(const DAVA::Vector<ControlNode*> &nodes)
{
    QClipboard *clipboard = QApplication::clipboard();
    if (!nodes.empty())
    {
        YamlPackageSerializer serializer;
        serializer.SerializePackageNodes(document->GetPackage(), nodes);
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
    if (nullptr == filteredPackageModel)
        return;

    RefreshActions();
    SelectedNodes selected;
    SelectedNodes deselected;
    
    QItemSelection selectedIndexes = filteredPackageModel->mapSelectionToSource(proxySelected);
    QItemSelection deselectedIndexes = filteredPackageModel->mapSelectionToSource(proxyDeselected);
    
    for(const auto index : selectedIndexes.indexes())
    {
        selected.insert(static_cast<PackageBaseNode*>(index.internalPointer()));
    }
    for(const auto index : deselectedIndexes.indexes())
    {
        deselected.insert(static_cast<PackageBaseNode*>(index.internalPointer()));
    }
    emit SelectedNodesChanged(selected, deselected);
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
        , QString::fromStdString(document->GetPackageFilePath().GetDirectory().GetStringValue())
        , "Packages (*.yaml)"
        );
    if (fileNames.isEmpty())
    {
        return;
    }

    PackageNode *root = document->GetPackage();
    Vector<FilePath> packages;
    for (const auto &fileName : fileNames)
    {
        packages.push_back(FilePath(fileName.toStdString()));
    }
    
    if (!packages.empty())
    {
        document->GetCommandExecutor()->AddImportedPackagesIntoPackage(packages, root);
    }
}

void PackageWidget::OnCopy()
{
    Vector<ControlNode*> nodes;
    CollectSelectedControls(nodes, true, false);
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
            document->GetCommandExecutor()->Paste(document->GetPackage(), node, node->GetCount(), string);
        }
    }
}

void PackageWidget::OnCut()
{
    Vector<ControlNode*> nodes;
    CollectSelectedControls(nodes, true, true);
    CopyNodesToClipboard(nodes);
    RemoveNodes(nodes);
}

void PackageWidget::OnDelete()
{
    Vector<ControlNode*> nodes;
    CollectSelectedControls(nodes, false, true);
    if (!nodes.empty())
    {
        RemoveNodes(nodes);
    }
    else
    {
        Vector<PackageNode*> packages;
        CollectSelectedImportedPackages(packages, false, true);
        document->GetCommandExecutor()->RemoveImportedPackagesFromPackage(packages, document->GetPackage());
    }
}

void PackageWidget::OnRename()
{
    const auto &selected = treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    treeView->edit(selected.first());
}

void PackageWidget::filterTextChanged(const QString &filterText)
{
    if (nullptr != document)
    {
        static_cast<QSortFilterProxyModel*>(treeView->model())->setFilterFixedString(filterText);
        treeView->expandAll();
    }
}

void PackageWidget::OnSelectedNodesChanged(const SelectedNodes &selected, const SelectedNodes &deselected)
{
    for (auto &node : deselected)
    {
        QModelIndex srcIndex = packageModel->indexByNode(node);
        QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
        treeView->selectionModel()->select(dstIndex, QItemSelectionModel::Deselect);
    }
    for (auto &node : selected)
    {
        QModelIndex srcIndex = packageModel->indexByNode(node);
        QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
        treeView->selectionModel()->select(dstIndex, QItemSelectionModel::Select);
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
