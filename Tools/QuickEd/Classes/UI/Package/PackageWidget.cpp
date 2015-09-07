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
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
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
        PackageWidget::ExpandedNodes expandedIndexes;
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

    addStyleAction = new QAction(tr("Add Style"), this);
    connect(addStyleAction, &QAction::triggered, this, &PackageWidget::OnAddStyle);
    
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

    renameAction = new QAction(tr("Rename"), this);
    connect(renameAction, &QAction::triggered, this, &PackageWidget::OnRename);
    
    delAction = new QAction(tr("Delete"), this);
    delAction->setShortcut(QKeySequence::Delete);
    delAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delAction, &QAction::triggered, this, &PackageWidget::OnDelete);
    
    treeView->addAction(importPackageAction);
    treeView->addAction(addStyleAction);
    treeView->addAction(CreateSeparator());

    treeView->addAction(cutAction);
    treeView->addAction(copyAction);
    treeView->addAction(pasteAction);
    treeView->addAction(CreateSeparator());
    
    treeView->addAction(renameAction);
    treeView->addAction(CreateSeparator());
    
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
    if (role == "editorActiveControls")
    {
        OnControlSelectedInEditor(sharedData->GetData("editorActiveControls").value<QList<ControlNode*> >());
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
        RestoreExpandedIndexes(context->expandedIndexes);
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

void PackageWidget::RefreshActions(const QList<PackageBaseNode*> &nodes)
{
    bool canInsertControls = nodes.size() == 1 && nodes[0]->IsInsertingControlsSupported();
    bool canInsertPackages = nodes.size() == 1 && nodes[0]->IsInsertingPackagesSupported();
    bool canInsertStyles = nodes.size() == 1 && nodes[0]->IsInsertingStylesSupported();
    bool canRemove = !nodes.empty();
    bool canCopy = false;
    bool canEdit = nodes.size() == 1 && nodes.first()->IsEditingSupported();
    
    for(const PackageBaseNode *node : nodes)
    {
        canCopy |= node->CanCopy();
        canRemove |= node->CanRemove();

        if (canCopy && canRemove)
            break;
    }
    
    RefreshAction(copyAction, canCopy, true);
    RefreshAction(pasteAction, canInsertControls || canInsertStyles, true);
    RefreshAction(cutAction, canCopy && canRemove, true);
    RefreshAction(delAction, canRemove, true);

    RefreshAction(importPackageAction, canInsertPackages, true);
    RefreshAction(addStyleAction, canInsertStyles, true);
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

void PackageWidget::CollectSelectedStyles(DAVA::Vector<StyleSheetNode*> &nodes, bool forCopy, bool forRemove)
{
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
    CollectSelectedNodes(selected, nodes, forCopy, forRemove);
}

void PackageWidget::CopyNodesToClipboard(const Vector<ControlNode*> &controls, const Vector<StyleSheetNode*> &styles)
{
    QClipboard *clipboard = QApplication::clipboard();
    if (!controls.empty() || !styles.empty())
    {
        YamlPackageSerializer serializer;
        serializer.SerializePackageNodes(sharedData->GetDocument()->GetPackage(), controls, styles);
        String str = serializer.WriteToString();
        QMimeData *data = new QMimeData();
        data->setText(QString(str.c_str()));
        clipboard->setMimeData(data);
    }
}

template <typename NodeType>
void PackageWidget::CollectSelectedNodes(const QItemSelection &selected, Vector<NodeType*> &nodes, bool forCopy, bool forRemove)
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



void PackageWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    if (filteredPackageModel.isNull())
        return;

    QList<PackageBaseNode*> selectedNodes;
    QModelIndexList indexes = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection()).indexes();
    for (const QModelIndex &index : indexes)
        selectedNodes.push_back(static_cast<PackageBaseNode*>(index.internalPointer()));

    RefreshActions(selectedNodes);
    sharedData->SetSelection(selectedNodes);
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
    Vector<FilePath> packages;
    for (const auto &fileName : fileNames)
    {
        packages.push_back(FilePath(fileName.toStdString()));
    }
    
    if (!packages.empty())
    {
        doc->GetCommandExecutor()->AddImportedPackagesIntoPackage(packages, root);
    }
}

void PackageWidget::OnCopy()
{
    Vector<ControlNode*> controls;
    CollectSelectedControls(controls, true, false);

    Vector<StyleSheetNode*> styles;
    CollectSelectedStyles(styles, true, false);
    
    CopyNodesToClipboard(controls, styles);
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
        
        if (!baseNode->IsReadOnly())
        {
            String string = clipboard->mimeData()->text().toStdString();
            Document *doc = sharedData->GetDocument();
            doc->GetCommandExecutor()->Paste(doc->GetPackage(), baseNode, baseNode->GetCount(), string);
        }
    }
}

void PackageWidget::OnCut()
{
    Vector<ControlNode*> controls;
    CollectSelectedControls(controls, true, true);

    Vector<StyleSheetNode*> styles;
    CollectSelectedStyles(styles, true, true);
    
    CopyNodesToClipboard(controls, styles);
    
    sharedData->GetDocument()->GetCommandExecutor()->Remove(controls, styles);
}

void PackageWidget::OnDelete()
{
    Vector<ControlNode*> controls;
    CollectSelectedControls(controls, false, true);
    
    Vector<StyleSheetNode*> styles;
    CollectSelectedStyles(styles, false, true);
    if (!controls.empty() || !styles.empty())
    {
        sharedData->GetDocument()->GetCommandExecutor()->Remove(controls, styles);
    }
    else
    {
        Vector<PackageNode*> packages;
        CollectSelectedImportedPackages(packages, false, true);
        Document *doc = sharedData->GetDocument();
        sharedData->GetDocument()->GetCommandExecutor()->RemoveImportedPackagesFromPackage(packages, doc->GetPackage());
    }
}

void PackageWidget::OnRename()
{
    const auto &selected = treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    treeView->edit(selected.first());
}

void PackageWidget::OnAddStyle()
{
    DAVA::Vector<DAVA::UIStyleSheetSelectorChain> selectorChains;
    selectorChains.push_back(UIStyleSheetSelectorChain("?"));
    const DAVA::Vector<DAVA::UIStyleSheetProperty> properties;
    
    ScopedPtr<StyleSheetNode> style(new StyleSheetNode(selectorChains, properties));
    Document *doc = sharedData->GetDocument();
    StyleSheetsNode *styleSheets = doc->GetPackage()->GetStyleSheets();
    doc->GetCommandExecutor()->InsertStyle(style, styleSheets, styleSheets->GetCount());
   
}

void PackageWidget::filterTextChanged(const QString &filterText)
{
    if (nullptr != sharedData)
    {
        if (lastFilterText.isEmpty())
        {
            expandedIndexes = GetExpandedIndexes();
        }
        filteredPackageModel->setFilterFixedString(filterText);

        if (filterText.isEmpty())
        {
            treeView->collapseAll();
            RestoreExpandedIndexes(expandedIndexes);
        }
        else
        {
            treeView->expandAll();
        }
        lastFilterText = filterText;
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

PackageWidget::ExpandedNodes PackageWidget::GetExpandedIndexes() const
{
    ExpandedNodes retval;
    QModelIndex index = treeView->model()->index(0, 0);
    while (index.isValid())
    {
        if (treeView->isExpanded(index))
        {
            retval << filteredPackageModel->mapToSource(index);
        }
        index = treeView->indexBelow(index);
    }
    
    return retval;
}

void PackageWidget::RestoreExpandedIndexes(const ExpandedNodes& indexes)
{
    for (auto &index : indexes)
    {
        QModelIndex mappedIndex = filteredPackageModel->mapFromSource(index);
        if (mappedIndex.isValid())
        {
            treeView->setExpanded(mappedIndex, true);
        }
    }
}

QAction *PackageWidget::CreateSeparator()
{
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    return separator;
}
