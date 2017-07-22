#include <QClipboard>

#include "PackageWidget.h"
#include "PackageModel.h"

#include "UI/CommandExecutor.h"

#include "UI/Package/FilteredPackageModel.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageIterator.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"

#include "Model/YamlPackageSerializer.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "UI/Package/FilteredPackageModel.h"
#include "UI/Package/PackageModel.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QtTools/FileDialogs/FileDialog.h>

#include <Base/Any.h>

using namespace DAVA;

namespace
{
template <typename NodeType>
void CollectSelectedNodes(const SelectedNodes& selectedNodes, Vector<NodeType*>& nodes, bool forCopy, bool forRemove)
{
    SortedPackageBaseNodeSet sortedNodes(CompareByLCA);
    std::copy_if(selectedNodes.begin(), selectedNodes.end(), std::inserter(sortedNodes, sortedNodes.end()), [](typename SelectedNodes::value_type node) {
        return (dynamic_cast<NodeType*>(node) != nullptr);
    });
    for (PackageBaseNode* node : sortedNodes)
    {
        DVASSERT(nullptr != node);
        if (node->GetParent() != nullptr)
        {
            if ((!forCopy || node->CanCopy()) &&
                (!forRemove || node->CanRemove()))
            {
                PackageBaseNode* parent = node->GetParent();
                while (nullptr != parent && sortedNodes.find(parent) == sortedNodes.end())
                {
                    parent = parent->GetParent();
                }
                if (nullptr == parent)
                {
                    nodes.push_back(DynamicTypeCheck<NodeType*>(node));
                }
            }
        }
    }
}

void AddSeparatorAction(QWidget* widget)
{
    QAction* separator = new QAction(widget);
    separator->setSeparator(true);
    widget->addAction(separator);
}

bool CanInsertControlOrStyle(const PackageBaseNode* dest, PackageBaseNode* node, int32 destIndex)
{
    if (dynamic_cast<ControlNode*>(node))
    {
        return dest->CanInsertControl(static_cast<ControlNode*>(node), destIndex);
    }
    if (dynamic_cast<StyleSheetNode*>(node))
    {
        return dest->CanInsertStyle(static_cast<StyleSheetNode*>(node), destIndex);
    }
    else
    {
        return false;
    }
}

bool CanMoveUpDown(const PackageBaseNode* dest, PackageBaseNode* node, bool up)
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != dest);

    if (dest->GetParent() != node->GetParent())
    {
        return false;
    }

    const PackageBaseNode* destParent = dest->GetParent();
    DVASSERT(nullptr != destParent);

    int destIndex = destParent->GetIndex(dest);
    if (!up)
    {
        destIndex += 1;
    }
    return CanInsertControlOrStyle(destParent, node, destIndex);
}

bool CanMoveLeft(PackageBaseNode* node)
{
    PackageBaseNode* parentNode = node->GetParent();
    DVASSERT(parentNode != nullptr);
    PackageBaseNode* grandParentNode = parentNode->GetParent();
    if (grandParentNode != nullptr)
    {
        int destIndex = grandParentNode->GetIndex(parentNode) + 1;
        return CanInsertControlOrStyle(grandParentNode, node, destIndex);
    }
    return false;
}

bool CanMoveRight(PackageBaseNode* node)
{
    PackageIterator iterUp(node, [node](const PackageBaseNode* dest) {
        return CanMoveUpDown(dest, node, true);
    });
    --iterUp;
    if (!iterUp.IsValid())
    {
        return false;
    }
    PackageBaseNode* dest = *iterUp;
    int destIndex = dest->GetCount();
    return CanInsertControlOrStyle(dest, node, destIndex);
}
} //unnamed namespace

PackageWidget::PackageWidget(QWidget* parent)
    : QDockWidget(parent)
{
    setupUi(this);
    filterLine->setEnabled(false);
    packageModel = new PackageModel(this);
    filteredPackageModel = new FilteredPackageModel(this);

    filteredPackageModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filteredPackageModel->setSourceModel(packageModel);

    treeView->setModel(filteredPackageModel);
    treeView->header()->setSectionResizeMode /*setResizeMode*/ (QHeaderView::ResizeToContents);

    connect(packageModel, &PackageModel::BeforeProcessNodes, this, &PackageWidget::OnBeforeProcessNodes);
    connect(packageModel, &PackageModel::AfterProcessNodes, this, &PackageWidget::OnAfterProcessNodes);
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChangedFromView);

    connect(filterLine, &QLineEdit::textChanged, this, &PackageWidget::OnFilterTextChanged);
    CreateActions();
    PlaceActions();
}

void PackageWidget::SetAccessor(TArc::ContextAccessor* accessor_)
{
    accessor = accessor_;
    dataWrapper = accessor->CreateWrapper(ReflectedTypeDB::Get<DocumentData>());

    packageModel->SetAccessor(accessor);
}

void PackageWidget::SetUI(DAVA::TArc::UI* ui_)
{
    ui = ui_;
    packageModel->SetUI(ui);
}

PackageWidget::~PackageWidget() = default;

PackageModel* PackageWidget::GetPackageModel() const
{
    return packageModel;
}

void PackageWidget::OnPackageChanged(PackageContext* context, PackageNode* package)
{
    widget()->setEnabled(package != nullptr);

    bool isUpdatesEnabled = treeView->updatesEnabled();
    treeView->setUpdatesEnabled(false);

    SaveContext();
    filterLine->clear(); //invalidate filter line state
    currentContext = context;
    packageModel->Reset(package);
    treeView->expandToDepth(0);
    LoadContext();

    treeView->setColumnWidth(0, treeView->size().width());
    treeView->setUpdatesEnabled(isUpdatesEnabled);
    filterLine->setEnabled(accessor->GetActiveContext() != nullptr);
}

QAction* PackageWidget::CreateAction(const QString& name, void (PackageWidget::*callback)(void), const QKeySequence& keySequence)
{
    QAction* action = new QAction(name, this);
    action->setEnabled(false);
    action->setShortcut(keySequence);
    action->setShortcutContext(Qt::WidgetShortcut);
    connect(action, &QAction::triggered, this, callback);
    return action;
}

void PackageWidget::CreateActions()
{
    addStyleAction = CreateAction(tr("Add Style"), &PackageWidget::OnAddStyle);
    importPackageAction = CreateAction(tr("Import package"), &PackageWidget::OnImport, QKeySequence::New);

    cutAction = CreateAction(tr("Cut"), &PackageWidget::OnCut, QKeySequence::Cut);
    copyAction = CreateAction(tr("Copy"), &PackageWidget::OnCopy, QKeySequence::Copy);
    pasteAction = CreateAction(tr("Paste"), &PackageWidget::OnPaste, QKeySequence::Paste);
    delAction = CreateAction(tr("Delete"), &PackageWidget::OnDelete, QKeySequence::Delete);
#if defined Q_OS_MAC
    delAction->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
#endif // platform
    duplicateControlsAction = CreateAction(tr("Duplicate"), &PackageWidget::OnDuplicate, QKeySequence(Qt::CTRL + Qt::Key_D));

    renameAction = CreateAction(tr("Rename"), &PackageWidget::OnRename);

    copyControlPathAction = CreateAction(tr("Copy Control Path"), &PackageWidget::OnCopyControlPath);

    moveUpAction = CreateAction(tr("Move up"), &PackageWidget::OnMoveUp, Qt::ControlModifier + Qt::Key_Up);
    moveDownAction = CreateAction(tr("Move down"), &PackageWidget::OnMoveDown, Qt::ControlModifier + Qt::Key_Down);
    moveLeftAction = CreateAction(tr("Move left"), &PackageWidget::OnMoveLeft, Qt::ControlModifier + Qt::Key_Left);
    moveRightAction = CreateAction(tr("Move right"), &PackageWidget::OnMoveRight, Qt::ControlModifier + Qt::Key_Right);
}

void PackageWidget::PlaceActions()
{
    treeView->addAction(importPackageAction);
    treeView->addAction(addStyleAction);
    AddSeparatorAction(treeView);

    treeView->addAction(cutAction);
    treeView->addAction(copyAction);
    treeView->addAction(pasteAction);
    treeView->addAction(duplicateControlsAction);
    AddSeparatorAction(treeView);

    treeView->addAction(copyControlPathAction);
    AddSeparatorAction(treeView);

    treeView->addAction(renameAction);
    AddSeparatorAction(treeView);

    treeView->addAction(delAction);

    AddSeparatorAction(treeView);
    treeView->addAction(moveUpAction);
    treeView->addAction(moveDownAction);
    treeView->addAction(moveLeftAction);
    treeView->addAction(moveRightAction);
    AddSeparatorAction(treeView);
}

void PackageWidget::LoadContext()
{
    if (nullptr != currentContext)
    {
        //restore expanded indexes
        RestoreExpandedIndexes(currentContext->expandedIndexes);
        //restore filter line
        filterLine->setText(currentContext->filterString);
    }
}

void PackageWidget::SaveContext()
{
    if (currentContext == nullptr)
    {
        return;
    }
    if (filterLine->text().isEmpty())
    {
        currentContext->expandedIndexes = GetExpandedIndexes();
    }
    else
    {
        currentContext->filterString = filterLine->text();
    }
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
    bool containControlNodes = false;
    bool canDuplicate = false;

    if (nodes.size() == 1)
    {
        PackageBaseNode* node = *nodes.begin();
        DVASSERT(node != nullptr);
        canInsertControls = node->IsInsertingControlsSupported();
        canInsertPackages = node->IsInsertingPackagesSupported();
        canInsertStyles = node->IsInsertingStylesSupported();
        canEdit = node->IsEditingSupported();

        if (node->GetControl() != nullptr)
        {
            containControlNodes = true;
        }

        if (node->CanRemove())
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
    if (nodes.empty() == false)
    {
        PackageBaseNode* parent = nullptr;
        for (auto iter = nodes.cbegin(); iter != nodes.cend(); ++iter)
        {
            PackageBaseNode* node = *iter;
            canCopy |= node->CanCopy();
            canRemove |= node->CanRemove();

            if (parent == nullptr)
            {
                parent = node->GetParent();
                canDuplicate = parent->IsInsertingControlsSupported();
            }
            else if (canDuplicate)
            {
                canDuplicate &= (parent == node->GetParent());
            }

            if (node->GetControl() != nullptr)
            {
                containControlNodes = true;
            }
        }
    }

    copyControlPathAction->setEnabled(containControlNodes);
    copyAction->setEnabled(canCopy);
    pasteAction->setEnabled(canInsertControls || canInsertStyles);
    cutAction->setEnabled(canCopy && canRemove);
    delAction->setEnabled(canRemove);
    duplicateControlsAction->setEnabled(canDuplicate);

    importPackageAction->setEnabled(canInsertPackages);
    addStyleAction->setEnabled(canInsertStyles);
    renameAction->setEnabled(canEdit);

    moveUpAction->setEnabled(canMoveUp);
    moveDownAction->setEnabled(canMoveDown);
    moveRightAction->setEnabled(canMoveRight);
    moveLeftAction->setEnabled(canMoveLeft);
}

void PackageWidget::CollectSelectedControls(Vector<ControlNode*>& nodes, bool forCopy, bool forRemove)
{
    CollectSelectedNodes(selectionContainer.selectedNodes, nodes, forCopy, forRemove);
}

void PackageWidget::CollectSelectedImportedPackages(Vector<PackageNode*>& nodes, bool forCopy, bool forRemove)
{
    CollectSelectedNodes(selectionContainer.selectedNodes, nodes, forCopy, forRemove);
}

void PackageWidget::CollectSelectedStyles(Vector<StyleSheetNode*>& nodes, bool forCopy, bool forRemove)
{
    CollectSelectedNodes(selectionContainer.selectedNodes, nodes, forCopy, forRemove);
}

void PackageWidget::CopyNodesToClipboard(const Vector<ControlNode*>& controls, const Vector<StyleSheetNode*>& styles)
{
    QClipboard* clipboard = QApplication::clipboard();
    if (!controls.empty() || !styles.empty())
    {
        YamlPackageSerializer serializer;

        TArc::DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        DVASSERT(documentData != nullptr);
        PackageNode* package = documentData->GetPackageNode();

        serializer.SerializePackageNodes(package, controls, styles);
        String str = serializer.WriteToString();
        QMimeData* data = new QMimeData();
        data->setText(QString(str.c_str()));
        clipboard->setMimeData(data);
    }
}

void PackageWidget::OnSelectionChangedFromView(const QItemSelection& proxySelected, const QItemSelection& proxyDeselected)
{
    if (nullptr == filteredPackageModel)
    {
        return;
    }

    SelectedNodes selected;
    SelectedNodes deselected;

    QItemSelection selectedIndexes = filteredPackageModel->mapSelectionToSource(proxySelected);
    QItemSelection deselectedIndexes = filteredPackageModel->mapSelectionToSource(proxyDeselected);

    SelectedNodes selection = selectionContainer.selectedNodes;
    for (const auto& index : deselectedIndexes.indexes())
    {
        selection.erase(static_cast<PackageBaseNode*>(index.internalPointer()));
    }
    for (const auto& index : selectedIndexes.indexes())
    {
        selection.insert(static_cast<PackageBaseNode*>(index.internalPointer()));
    }

    SetSelectedNodes(selection);

    emit SelectedNodesChanged(selection);
}

void PackageWidget::OnImport()
{
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    if (selectedIndexList.isEmpty())
    {
        return;
    }
    TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    DVASSERT(documentData != nullptr);
    QStringList fileNames = FileDialog::getOpenFileNames(qApp->activeWindow(),
                                                         tr("Select one or move files to import"),
                                                         QString::fromStdString(documentData->GetPackagePath().GetDirectory().GetStringValue()),
                                                         "Packages (*.yaml)");
    if (fileNames.isEmpty())
    {
        return;
    }

    Vector<FilePath> packages;
    for (const auto& fileName : fileNames)
    {
        packages.push_back(FilePath(fileName.toStdString()));
    }
    DVASSERT(!packages.empty());
    CommandExecutor commandExecutor(accessor, ui);
    commandExecutor.AddImportedPackagesIntoPackage(packages, documentData->GetPackageNode());
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
    QClipboard* clipboard = QApplication::clipboard();

    if (!selectedIndexList.empty() && clipboard && clipboard->mimeData())
    {
        const QModelIndex& index = selectedIndexList.first();

        PackageBaseNode* baseNode = static_cast<PackageBaseNode*>(index.internalPointer());

        if (!baseNode->IsReadOnly())
        {
            String string = clipboard->mimeData()->text().toStdString();
            DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
            DVASSERT(activeContext != nullptr);
            DocumentData* documentData = activeContext->GetData<DocumentData>();
            DVASSERT(nullptr != documentData);
            PackageNode* package = documentData->GetPackageNode();
            CommandExecutor executor(accessor, ui);
            SelectedNodes selection = executor.Paste(package, baseNode, baseNode->GetCount(), string);
            if (selection.empty() == false)
            {
                dataWrapper.SetFieldValue(DocumentData::selectionPropertyName, selection);
            }
        }
    }
}

void PackageWidget::OnDuplicate()
{
    //TODO: remove this block when package widget will be refactored
    TArc::DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    if (documentData == nullptr)
    {
        return;
    }
    SelectedNodes nodes = documentData->GetSelectedNodes();
    if (nodes.empty())
    {
        return;
    }
    QApplication::clipboard()->clear();

    OnCopy();

    QClipboard* clipboard = QApplication::clipboard();

    if (clipboard && clipboard->mimeData())
    {
        Vector<PackageBaseNode*> sortedSelection(nodes.begin(), nodes.end());
        std::sort(sortedSelection.begin(), sortedSelection.end(), CompareByLCA);
        PackageBaseNode* parent = sortedSelection.front()->GetParent();
        if (parent->IsReadOnly() == false)
        {
            String string = clipboard->mimeData()->text().toStdString();

            PackageNode* package = documentData->GetPackageNode();
            CommandExecutor executor(accessor, ui);

            PackageBaseNode* lastSelected = sortedSelection.back();
            int index = parent->GetIndex(lastSelected);
            SelectedNodes selection = executor.Paste(package, parent, index + 1, string);
            if (selection.empty() == false)
            {
                dataWrapper.SetFieldValue(DocumentData::selectionPropertyName, selection);
            }
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

    CommandExecutor executor(accessor, ui);
    executor.Remove(controls, styles);
}

void PackageWidget::OnDelete()
{
    if (accessor->GetActiveContext() == nullptr)
    {
        return;
    }
    CommandExecutor executor(accessor, ui);

    Vector<ControlNode*> controls;
    CollectSelectedControls(controls, false, true);

    Vector<StyleSheetNode*> styles;
    CollectSelectedStyles(styles, false, true);

    if (!controls.empty() || !styles.empty())
    {
        executor.Remove(controls, styles);
    }
    else
    {
        Vector<PackageNode*> packages;
        CollectSelectedImportedPackages(packages, false, true);

        TArc::DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        DVASSERT(nullptr != documentData);

        PackageNode* package = documentData->GetPackageNode();
        executor.RemoveImportedPackagesFromPackage(packages, package);
    }
}

void PackageWidget::OnRename()
{
    const auto& selected = treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    treeView->edit(selected.first());
}

void PackageWidget::OnAddStyle()
{
    Vector<UIStyleSheetSelectorChain> selectorChains;
    selectorChains.push_back(UIStyleSheetSelectorChain("?"));
    const Vector<UIStyleSheetProperty> properties;

    TArc::DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    DVASSERT(documentData != nullptr);
    ScopedPtr<StyleSheetNode> style(new StyleSheetNode(UIStyleSheetSourceInfo(documentData->GetPackagePath()), selectorChains, properties));
    PackageNode* package = documentData->GetPackageNode();
    CommandExecutor commandExecutor(accessor, ui);
    StyleSheetsNode* styleSheets = package->GetStyleSheets();
    commandExecutor.InsertStyle(style, styleSheets, styleSheets->GetCount());
}

void PackageWidget::OnCopyControlPath()
{
    Vector<ControlNode*> controlNodes;
    CollectSelectedControls(controlNodes, false, false);

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* data = new QMimeData();

    QString str;
    for (ControlNode* controlNode : controlNodes)
    {
        PackageControlsNode* controlsRoot = controlNode->GetPackage()->GetPackageControlsNode();
        PackageControlsNode* prototypesRoot = controlNode->GetPackage()->GetPrototypes();
        QString path;
        for (PackageBaseNode* node = controlNode; node != controlsRoot && node != prototypesRoot && node != nullptr; node = node->GetParent())
        {
            if (!path.isEmpty())
            {
                path.prepend("/");
            }
            path.prepend(QString::fromStdString(node->GetName()));
        }
        str += path;
        str += "\n";
    }

    data->setText(str);
    clipboard->setMimeData(data);
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
    DVASSERT((dynamic_cast<ControlNode*>(node) != nullptr && dynamic_cast<const ControlNode*>(nextNode) != nullptr) || (dynamic_cast<StyleSheetNode*>(node) != nullptr && dynamic_cast<const StyleSheetNode*>(nextNode) != nullptr));
    PackageBaseNode* nextNodeParent = nextNode->GetParent();
    DVASSERT(nextNodeParent != nullptr);
    int destIndex = nextNodeParent->GetIndex(nextNode);
    if (!up)
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

    PackageBaseNode* parentNode = node->GetParent();
    DVASSERT(parentNode != nullptr);
    PackageBaseNode* grandParentNode = parentNode->GetParent();
    DVASSERT(grandParentNode != nullptr);
    int destIndex = grandParentNode->GetIndex(parentNode) + 1;

    MoveNodeImpl(node, grandParentNode, destIndex);
}

void PackageWidget::OnMoveRight()
{
    const SelectedNodes& nodes = selectionContainer.selectedNodes;
    DVASSERT(nodes.size() == 1);
    PackageBaseNode* node = *nodes.begin();

    PackageIterator iterUp(node, [node](const PackageBaseNode* dest) {
        return CanMoveUpDown(dest, node, true);
    });
    --iterUp;

    DVASSERT(iterUp.IsValid());

    PackageBaseNode* dest = *iterUp;

    MoveNodeImpl(node, dest, dest->GetCount());
}

void PackageWidget::MoveNodeImpl(PackageBaseNode* node, PackageBaseNode* dest, uint32 destIndex)
{
    CommandExecutor executor(accessor, ui);
    if (dynamic_cast<ControlNode*>(node) != nullptr)
    {
        Vector<ControlNode*> nodes = { static_cast<ControlNode*>(node) };
        ControlsContainerNode* nextControlNode = dynamic_cast<ControlsContainerNode*>(dest);
        OnBeforeProcessNodes(SelectedNodes(nodes.begin(), nodes.end()));
        executor.MoveControls(nodes, nextControlNode, destIndex);
        OnAfterProcessNodes(SelectedNodes(nodes.begin(), nodes.end()));
    }
    else if (dynamic_cast<StyleSheetNode*>(node) != nullptr)
    {
        Vector<StyleSheetNode*> nodes = { static_cast<StyleSheetNode*>(node) };
        StyleSheetsNode* nextStyleSheetNode = dynamic_cast<StyleSheetsNode*>(dest);
        OnBeforeProcessNodes(SelectedNodes(nodes.begin(), nodes.end()));
        executor.MoveStyles(nodes, nextStyleSheetNode, destIndex);
        OnAfterProcessNodes(SelectedNodes(nodes.begin(), nodes.end()));
    }
    else
    {
        DVASSERT(0 && "invalid type of moved node");
    }
}

void PackageWidget::OnFilterTextChanged(const QString& filterText)
{
    if (currentContext != nullptr)
    {
        if (lastFilterTextEmpty)
        {
            currentContext->expandedIndexes = GetExpandedIndexes();
        }
        filteredPackageModel->setFilterFixedString(filterText);

        if (filterText.isEmpty())
        {
            treeView->collapseAll();
            RestoreExpandedIndexes(currentContext->expandedIndexes);
        }
        else
        {
            treeView->expandAll();
        }
        lastFilterTextEmpty = filterText.isEmpty();
    }
}

void PackageWidget::CollectExpandedIndexes(PackageBaseNode* node)
{
    QModelIndex srcIndex = packageModel->indexByNode(node);
    QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
    if (treeView->isExpanded(dstIndex))
    {
        expandedNodes.insert(node);
    }
    for (int i = 0, count = node->GetCount(); i < count; ++i)
    {
        CollectExpandedIndexes(node->Get(i));
    }
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

void PackageWidget::OnBeforeProcessNodes(const SelectedNodes& nodes)
{
    for (const auto& node : nodes)
    {
        CollectExpandedIndexes(node);
    }
}

void PackageWidget::OnAfterProcessNodes(const SelectedNodes& nodes)
{
    if (nodes.empty())
    {
        return;
    }
    OnSelectionChanged(nodes);
    emit SelectedNodesChanged(selectionContainer.selectedNodes); //this is only way to select manually in package widget
    for (const auto& node : expandedNodes)
    {
        QModelIndex srcIndex = packageModel->indexByNode(node);
        QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
        treeView->expand(dstIndex);
    }
    expandedNodes.clear();
}

void PackageWidget::DeselectNodeImpl(PackageBaseNode* node)
{
    QModelIndex srcIndex = packageModel->indexByNode(node);
    DVASSERT(srcIndex.isValid());
    QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
    if (dstIndex.isValid())
    {
        treeView->selectionModel()->select(dstIndex, QItemSelectionModel::Deselect);
    }
}

void PackageWidget::SelectNodeImpl(PackageBaseNode* node)
{
    QModelIndex srcIndex = packageModel->indexByNode(node);
    DVASSERT(srcIndex.isValid());

    QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
    if (dstIndex.isValid())
    {
        QItemSelectionModel* selectionModel = treeView->selectionModel();
        selectionModel->setCurrentIndex(dstIndex, QItemSelectionModel::Select);
        treeView->scrollTo(dstIndex);
    }
}

void PackageWidget::RestoreExpandedIndexes(const ExpandedIndexes& indexes)
{
    for (auto& index : indexes)
    {
        QModelIndex mappedIndex = filteredPackageModel->mapFromSource(index);
        if (mappedIndex.isValid())
        {
            treeView->setExpanded(mappedIndex, true);
        }
    }
}

void PackageWidget::OnSelectionChanged(const Any& selectionValue)
{
    disconnect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChangedFromView);
    SelectedNodes selection = selectionValue.Cast<SelectedNodes>(SelectedNodes());
    SetSelectedNodes(selection);
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChangedFromView);
}

void PackageWidget::SetSelectedNodes(const SelectedNodes& selection)
{
    RefreshActions();

    //this code is used to synchronize last selected item with properties model
    SelectedNodes selected;
    SelectedNodes deselected;

    selectionContainer.GetNotExistedItems(selection, selected);
    SelectionContainer tmpContainer = { selection };
    tmpContainer.GetNotExistedItems(selectionContainer.selectedNodes, deselected);

    selectionContainer.selectedNodes = selection;

    for (PackageBaseNode* node : deselected)
    {
        DeselectNodeImpl(node);
    }

    for (PackageBaseNode* node : selected)
    {
        SelectNodeImpl(node);
    }

    //TODO: remove refreshActions and all selection logic when Package will be a TArc module
    RefreshActions();
}
