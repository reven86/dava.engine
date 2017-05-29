#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include <Command/Command.h>
#include <Command/CommandStack.h>

#include <QFileInfo>

DAVA_VIRTUAL_REFLECTION_IMPL(DocumentData)
{
    DAVA::ReflectionRegistrator<DocumentData>::Begin()
    .Field(packagePropertyName.c_str(), &DocumentData::GetPackageNode, nullptr)
    .Field(canSavePropertyName.c_str(), &DocumentData::CanSave, nullptr)
    .Field(canUndoPropertyName.c_str(), &DocumentData::CanUndo, nullptr)
    .Field(canRedoPropertyName.c_str(), &DocumentData::CanRedo, nullptr)
    .Field(undoTextPropertyName.c_str(), &DocumentData::GetUndoText, nullptr)
    .Field(redoTextPropertyName.c_str(), &DocumentData::GetRedoText, nullptr)
    .Field(selectionPropertyName.c_str(), &DocumentData::GetSelectedNodes, &DocumentData::SetSelectedNodes)
    .Field(displayedRootControlsPropertyName.c_str(), &DocumentData::GetDisplayedRootControls, &DocumentData::SetDisplayedRootControls)
    .End();
}

DocumentData::DocumentData(const DAVA::RefPtr<PackageNode>& package_)
    : package(package_)
    , commandStack(new DAVA::CommandStack())
    , displayedRootControls(CompareByLCA)
{
    PackageControlsNode* controlsNode = package->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        displayedRootControls.insert(controlsNode->Get(index));
    }
}

DocumentData::~DocumentData() = default;

void DocumentData::ExecCommand(std::unique_ptr<DAVA::Command>&& command)
{
    commandStack->Exec(std::move(command));
}

void DocumentData::BeginBatch(const DAVA::String& batchName, DAVA::uint32 commandsCount)
{
    startedBatches++;
    commandStack->BeginBatch(batchName, commandsCount);
}

void DocumentData::EndBatch()
{
    DVASSERT(startedBatches != 0);
    startedBatches--;
    commandStack->EndBatch();
}

PackageNode* DocumentData::GetPackageNode() const
{
    return package.Get();
}

const SelectedNodes& DocumentData::GetSelectedNodes() const
{
    return selection.selectedNodes;
}

const SortedControlNodeSet& DocumentData::GetDisplayedRootControls() const
{
    return displayedRootControls;
}

void DocumentData::SetSelectedNodes(const SelectedNodes& nodes)
{
    selection.selectedNodes = nodes;

    SortedControlNodeSet newDisplayedRootControls(CompareByLCA);
    for (PackageBaseNode* selectedNode : selection.selectedNodes)
    {
        if (dynamic_cast<ControlNode*>(selectedNode) == nullptr)
        {
            continue;
        }
        PackageBaseNode* root = selectedNode;
        while (nullptr != root->GetParent() && nullptr != root->GetParent()->GetControl())
        {
            root = root->GetParent();
        }
        if (nullptr != root)
        {
            ControlNode* rootControl = dynamic_cast<ControlNode*>(root);
            DVASSERT(rootControl != nullptr);
            newDisplayedRootControls.insert(rootControl);
        }
    }
    if (newDisplayedRootControls.empty() == false)
    {
        displayedRootControls = newDisplayedRootControls;
    }
}

void DocumentData::SetDisplayedRootControls(const SortedControlNodeSet& controls)
{
    displayedRootControls = controls;
}

QString DocumentData::GetName() const
{
    QFileInfo fileInfo(GetPackageAbsolutePath());
    return fileInfo.fileName();
}

QString DocumentData::GetPackageAbsolutePath() const
{
    return QString::fromStdString(package->GetPath().GetAbsolutePathname());
}

DAVA::FilePath DocumentData::GetPackagePath() const
{
    return package->GetPath();
}

bool DocumentData::CanSave() const
{
    return (documentExists == false || commandStack->IsClean() == false);
}

bool DocumentData::CanUndo() const
{
    return commandStack->CanUndo();
}

bool DocumentData::CanRedo() const
{
    return commandStack->CanRedo();
}

bool DocumentData::CanClose() const
{
    return startedBatches == 0;
}

QString DocumentData::GetRedoText() const
{
    const DAVA::Command* command = commandStack->GetRedoCommand();
    DAVA::String text = (command != nullptr ? command->GetDescription() : "");
    return QString::fromStdString(text);
}

bool DocumentData::IsDocumentExists() const
{
    return documentExists;
}

QString DocumentData::GetUndoText() const
{
    const DAVA::Command* command = commandStack->GetUndoCommand();
    DAVA::String text = (command != nullptr ? command->GetDescription() : "");
    return QString::fromStdString(text);
}

void DocumentData::RefreshLayout()
{
    package->RefreshPackageStylesAndLayout(true);
}

void DocumentData::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
    package->GetPrototypes()->RefreshControlProperties();
}

DAVA::FastName DocumentData::packagePropertyName{ "package" };
DAVA::FastName DocumentData::canSavePropertyName{ "can save" };
DAVA::FastName DocumentData::canUndoPropertyName{ "can undo" };
DAVA::FastName DocumentData::canRedoPropertyName{ "can redo" };
DAVA::FastName DocumentData::undoTextPropertyName{ "undo text" };
DAVA::FastName DocumentData::redoTextPropertyName{ "redo text" };
DAVA::FastName DocumentData::selectionPropertyName{ "selection" };
DAVA::FastName DocumentData::displayedRootControlsPropertyName{ "displayed root controls" };
