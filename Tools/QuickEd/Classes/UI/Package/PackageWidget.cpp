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
#include "Model/PackageHierarchy/PackageIterator.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
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
        PackageWidget::ExpandedIndexes expandedIndexes;
        QString filterString;
    };

    void AddSeparatorAction(QWidget* widget)
    {
        QAction* separator = new QAction(widget);
        separator->setSeparator(true);
        widget->addAction(separator);
    }
    
    bool CanInsertControlOrStyle(const PackageBaseNode *dest, PackageBaseNode *node, DAVA::int32 destIndex)
    {
        if(dynamic_cast<ControlNode*>(node))
        {
            return dest->CanInsertControl(static_cast<ControlNode*>(node), destIndex);
        }
        if(dynamic_cast<StyleSheetNode*>(node))
        {
            return dest->CanInsertStyle(static_cast<StyleSheetNode*>(node), destIndex);
        }
        else
        {
            return false;
        }
    }
    
    bool CanMoveUpDown(const PackageBaseNode *dest, PackageBaseNode *node, bool up)
    {
        DVASSERT(nullptr != node);
        DVASSERT(nullptr != dest);

        if(dest->GetParent() != node->GetParent())
        {
            return false;
        }

        const PackageBaseNode *destParent = dest->GetParent();
        DVASSERT(nullptr != destParent);
        
        int destIndex = destParent->GetIndex(dest);
        if(!up)
        {
            destIndex += 1;
        }
        return CanInsertControlOrStyle(destParent, node, destIndex);
    }
    
    bool CanMoveLeft(PackageBaseNode *node)
    {
        PackageBaseNode *parentNode = node->GetParent();
        DVASSERT(parentNode != nullptr);
        PackageBaseNode *grandParentNode = parentNode->GetParent();
        if(grandParentNode != nullptr)
        {
            int destIndex = grandParentNode->GetIndex(parentNode) + 1;
            return CanInsertControlOrStyle(grandParentNode, node, destIndex);
        }
        return false;
    }
    
    bool CanMoveRight(PackageBaseNode *node)
    {
        PackageIterator iterUp(node, [node](const PackageBaseNode* dest){
            return CanMoveUpDown(dest, node, true);
        });
        --iterUp;
        if(!iterUp.IsValid())
        {
            return false;
        }
        PackageBaseNode *dest = *iterUp;
        int destIndex = dest->GetCount();
        return CanInsertControlOrStyle(dest, node, destIndex);
    }
}

PackageWidget::PackageWidget(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi(this);
    treeView->header()->setSectionResizeMode/*setResizeMode*/(QHeaderView::ResizeToContents);

    connect(filterLine, &QLineEdit::textChanged, this, &PackageWidget::filterTextChanged);

    CreateActions();
    PlaceActions();
}

void PackageWidget::OnDocumentChanged(Document* arg)
{
    bool isUpdatesEnabled = treeView->updatesEnabled();
    treeView->setUpdatesEnabled(false);

    SaveContext();
    document = arg;

    LoadContext();

    treeView->setColumnWidth(0, treeView->size().width());
    treeView->setUpdatesEnabled(isUpdatesEnabled);
}

void PackageWidget::CreateActions()
{
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

    moveUpAction = new QAction(tr("Move up"), this);
    moveUpAction->setShortcut(Qt::ControlModifier + Qt::Key_Up);
    moveUpAction->setShortcutContext(Qt::WidgetShortcut);
    connect(moveUpAction, &QAction::triggered, this, &PackageWidget::OnMoveUp);

    moveDownAction = new QAction(tr("Move down"), this);
    moveDownAction->setShortcut(Qt::ControlModifier + Qt::Key_Down);
    moveDownAction->setShortcutContext(Qt::WidgetShortcut);
    connect(moveDownAction, &QAction::triggered, this, &PackageWidget::OnMoveDown);
    
    moveLeftAction = new QAction(tr("Move left"), this);
    moveLeftAction->setShortcut(Qt::ControlModifier + Qt::Key_Left);
    moveLeftAction->setShortcutContext(Qt::WidgetShortcut);
    connect(moveLeftAction, &QAction::triggered, this, &PackageWidget::OnMoveLeft);
    
    moveRightAction = new QAction(tr("Move right"), this);
    moveRightAction->setShortcut(Qt::ControlModifier + Qt::Key_Right);
    moveRightAction->setShortcutContext(Qt::WidgetShortcut);
    connect(moveRightAction, &QAction::triggered, this, &PackageWidget::OnMoveRight);
}

void PackageWidget::PlaceActions()
{
    treeView->addAction(importPackageAction);
    treeView->addAction(addStyleAction);
    AddSeparatorAction(treeView);

    treeView->addAction(cutAction);
    treeView->addAction(copyAction);
    treeView->addAction(pasteAction);
    AddSeparatorAction(treeView);

    treeView->addAction(renameAction);
    AddSeparatorAction(treeView);

    treeView->addAction(delAction);

    AddSeparatorAction(treeView);
    treeView->addAction(moveUpAction);
    treeView->addAction(moveDownAction);
    treeView->addAction(moveLeftAction);
    treeView->addAction(moveRightAction);
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
        PackageContext* context = dynamic_cast<PackageContext*>(document->GetContext(this));
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
        RestoreExpandedIndexes(context->expandedIndexes);
        //restore filter line
        filterLine->setText(context->filterString);
    }
}

void PackageWidget::SaveContext()
{
    if (nullptr == document)
    {
        return;
    }
    PackageContext* context = dynamic_cast<PackageContext*>(document->GetContext(this));
    context->expandedIndexes = GetExpandedIndexes();
    context->filterString = filterLine->text();
}

void PackageWidget::RefreshActions()
{
    const SelectedNodes& nodes = selectionContainer.selectedNodes;
    bool canInsertControls = false;
    bool canInsertPackages = false;
    bool canInsertStyles = false;
    bool canRemove = !nodes.empty();
    bool canCopy = false;
    bool canEdit = false;
    bool canMoveUp = false;
    bool canMoveDown = false;
    bool canMoveLeft = false;
    bool canMoveRight = false;

    if (nodes.size() == 1)
    {
        PackageBaseNode* node = *nodes.begin();
        DVASSERT(node != nullptr);
        canInsertControls = node->IsInsertingControlsSupported();
        canInsertPackages = node->IsInsertingPackagesSupported();
        canInsertStyles = node->IsInsertingStylesSupported();
        canEdit = node->IsEditingSupported();
        
        if(node->CanRemove())
        {
            PackageIterator iterUp(node, [node](const PackageBaseNode* dest) -> bool {
                return CanMoveUpDown(dest, node, true);
            });
            --iterUp;
            canMoveUp = iterUp.IsValid();
            
            PackageIterator iterDown(node, [node](const PackageBaseNode* dest) -> bool {
                return CanMoveUpDown(dest, node, false);
            });
            ++iterDown;
            canMoveDown = iterDown.IsValid();
            canMoveLeft = CanMoveLeft(node);
            canMoveRight = CanMoveRight(node);
        }
        
        
    }
    for (auto iter = nodes.cbegin(); iter != nodes.cend() && (!canCopy || !canRemove); ++iter)
    {
        canCopy |= (*iter)->CanCopy();
        canRemove |= (*iter)->CanRemove();
    }

    copyAction->setEnabled(canCopy);
    pasteAction->setEnabled(canInsertControls || canInsertStyles);
    cutAction->setEnabled(canCopy && canRemove);
    delAction->setEnabled(canRemove);

    importPackageAction->setEnabled(canInsertPackages);
    addStyleAction->setEnabled(canInsertStyles);
    renameAction->setEnabled(canEdit);

    moveUpAction->setEnabled(canMoveUp);
    moveDownAction->setEnabled(canMoveDown);
    moveRightAction->setEnabled(canMoveRight);
    moveLeftAction->setEnabled(canMoveLeft);
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
        serializer.SerializePackageNodes(document->GetPackage(), controls, styles);
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
    if (nullptr == filteredPackageModel)
    {
        return;
    }

    SelectedNodes selected;
    SelectedNodes deselected;

    QItemSelection selectedIndexes = filteredPackageModel->mapSelectionToSource(proxySelected);
    QItemSelection deselectedIndexes = filteredPackageModel->mapSelectionToSource(proxyDeselected);

    for (const auto index : selectedIndexes.indexes())
    {
        selected.insert(static_cast<PackageBaseNode*>(index.internalPointer()));
    }
    for (const auto index : deselectedIndexes.indexes())
    {
        deselected.insert(static_cast<PackageBaseNode*>(index.internalPointer()));
    }
    SetSelectedNodes(selected, deselected);
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
    qApp->activeWindow(), tr("Select one or move files to import"), QString::fromStdString(document->GetPackageFilePath().GetDirectory().GetStringValue()), "Packages (*.yaml)");
    if (fileNames.isEmpty())
    {
        return;
    }

    PackageNode* root = document->GetPackage();
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
            document->GetCommandExecutor()->Paste(document->GetPackage(), baseNode, baseNode->GetCount(), string);
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

    document->GetCommandExecutor()->Remove(controls, styles);
}

void PackageWidget::OnDelete()
{
    Vector<ControlNode*> controls;
    CollectSelectedControls(controls, false, true);
    
    Vector<StyleSheetNode*> styles;
    CollectSelectedStyles(styles, false, true);
    if (!controls.empty() || !styles.empty())
    {
        document->GetCommandExecutor()->Remove(controls, styles);
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

void PackageWidget::OnAddStyle()
{
    DAVA::Vector<DAVA::UIStyleSheetSelectorChain> selectorChains;
    selectorChains.push_back(UIStyleSheetSelectorChain("?"));
    const DAVA::Vector<DAVA::UIStyleSheetProperty> properties;
    
    ScopedPtr<StyleSheetNode> style(new StyleSheetNode(selectorChains, properties));
    StyleSheetsNode* styleSheets = document->GetPackage()->GetStyleSheets();
    document->GetCommandExecutor()->InsertStyle(style, styleSheets, styleSheets->GetCount());
}

void PackageWidget::OnMoveUp()
{
    MoveNodeUpDown(true);
}

void PackageWidget::OnMoveDown()
{
    MoveNodeUpDown(false);
}

void PackageWidget::MoveNodeUpDown(bool up)
{
    const SelectedNodes& nodes = selectionContainer.selectedNodes;
    DVASSERT(nodes.size() == 1);
    PackageBaseNode* node = *nodes.begin();
    PackageIterator iter(node, [up, node](const PackageBaseNode* dest) -> bool {
        return CanMoveUpDown(dest, node, up);
    });
    PackageBaseNode* nextNode = up ? *(--iter) : *(++iter);
    DVASSERT(nullptr != nextNode);
    DVASSERT(!(dynamic_cast<ControlNode*>(node) != nullptr ^ dynamic_cast<const ControlNode*>(nextNode) != nullptr)
             && !(dynamic_cast<StyleSheetsNode*>(node) != nullptr ^ dynamic_cast<const StyleSheetsNode*>(nextNode) != nullptr));
    PackageBaseNode* nextNodeParent = nextNode->GetParent();
    DVASSERT(nextNodeParent != nullptr);
    int destIndex = nextNodeParent->GetIndex(nextNode);
    if(!up)
    {
        ++destIndex;
    }
    MoveNodeImpl(node, nextNodeParent, destIndex);
}

void PackageWidget::OnMoveLeft()
{
    const SelectedNodes& nodes = selectionContainer.selectedNodes;
    DVASSERT(nodes.size() == 1);
    PackageBaseNode* node = *nodes.begin();
    
    PackageBaseNode *parentNode = node->GetParent();
    DVASSERT(parentNode != nullptr);
    PackageBaseNode *grandParentNode = parentNode->GetParent();
    DVASSERT(grandParentNode != nullptr);
    int destIndex = grandParentNode->GetIndex(parentNode) + 1;
    
    MoveNodeImpl(node, grandParentNode, destIndex);
}

void PackageWidget::OnMoveRight()
{
    const SelectedNodes& nodes = selectionContainer.selectedNodes;
    DVASSERT(nodes.size() == 1);
    PackageBaseNode* node = *nodes.begin();
    
    PackageIterator iterUp(node, [node](const PackageBaseNode* dest){
        return CanMoveUpDown(dest, node, true);
    });
    --iterUp;
    
    DVASSERT(iterUp.IsValid());
    
    PackageBaseNode *dest = *iterUp;
    
    MoveNodeImpl(node, dest, dest->GetCount());
}

void PackageWidget::MoveNodeImpl(PackageBaseNode *node, PackageBaseNode *dest, DAVA::uint32 destIndex)
{
    auto commandExecutor = document->GetCommandExecutor();
    if (dynamic_cast<ControlNode*>(node) != nullptr)
    {
        DAVA::Vector<ControlNode*> nodes = { static_cast<ControlNode*>(node) };
        ControlsContainerNode* nextControlNode = dynamic_cast<ControlsContainerNode*>(dest);
        commandExecutor->MoveControls(nodes, nextControlNode, destIndex);
    }
    else if (dynamic_cast<StyleSheetNode*>(node) != nullptr)
    {
        DAVA::Vector<StyleSheetNode*> nodes = { static_cast<StyleSheetNode*>(node) };
        StyleSheetsNode* nextStyleSheetNode = dynamic_cast<StyleSheetsNode*>(dest);
        commandExecutor->MoveStyles(nodes, nextStyleSheetNode, destIndex);
    }
    else
    {
        DVASSERT(0 && "invalid type of moved node");
    }
    SelectNode(node);
}

void PackageWidget::filterTextChanged(const QString &filterText)
{
    if (nullptr != document)
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

void PackageWidget::SelectNode(PackageBaseNode *node)
{
    treeView->selectionModel()->clear();
    QModelIndex srcIndex = packageModel->indexByNode(node);
    QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
    treeView->selectionModel()->select(dstIndex, QItemSelectionModel::Select);
    treeView->scrollTo(dstIndex);
}

PackageWidget::ExpandedIndexes PackageWidget::GetExpandedIndexes() const
{
    ExpandedIndexes retval;
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

void PackageWidget::RestoreExpandedIndexes(const ExpandedIndexes& indexes)
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

void PackageWidget::SetSelectedNodes(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectedNodes reallySelected;
    SelectedNodes reallyDeselected;

    selectionContainer.GetOnlyExistedItems(deselected, reallyDeselected);
    selectionContainer.GetNotExistedItems(selected, reallySelected);
    selectionContainer.MergeSelection(reallySelected, reallyDeselected);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        RefreshActions();

        disconnect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChanged);

        for (const auto& node : reallyDeselected)
        {
            QModelIndex srcIndex = packageModel->indexByNode(node);
            QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
            treeView->selectionModel()->select(dstIndex, QItemSelectionModel::Deselect);
        }
        for (const auto& node : reallySelected)
        {
            QModelIndex srcIndex = packageModel->indexByNode(node);
            QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
            treeView->selectionModel()->select(dstIndex, QItemSelectionModel::Select);
            treeView->scrollTo(dstIndex);
        }

        connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChanged);
        emit SelectedNodesChanged(reallySelected, reallyDeselected);
    }
}
