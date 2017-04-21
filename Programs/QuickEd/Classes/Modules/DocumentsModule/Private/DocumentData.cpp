#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include <Command/Command.h>
#include <Command/CommandStack.h>

#include <QFileInfo>

DAVA_VIRTUAL_REFLECTION_IMPL(DocumentData)
{
    DAVA::ReflectionRegistrator<DocumentData>::Begin()
    .Field(packagePropertyName, &DocumentData::GetPackageNode, nullptr)
    .Field(canSavePropertyName, &DocumentData::CanSave, nullptr)
    .Field(canUndoPropertyName, &DocumentData::CanUndo, nullptr)
    .Field(canRedoPropertyName, &DocumentData::CanRedo, nullptr)
    .Field(undoTextPropertyName, &DocumentData::GetUndoText, nullptr)
    .Field(redoTextPropertyName, &DocumentData::GetRedoText, nullptr)
    .Field(selectionPropertyName, &DocumentData::GetSelectedNodes, &DocumentData::SetSelectedNodes)
    .Field(editedRootControlsPropertyName, &DocumentData::GetEditedRootControls, &DocumentData::SetEditedRootControls)
    .End();
}

DocumentData::DocumentData(const DAVA::RefPtr<PackageNode>& package_)
    : package(package_)
    , commandStack(new DAVA::CommandStack())
{
}

DocumentData::~DocumentData() = default;

void DocumentData::ExecCommand(std::unique_ptr<DAVA::Command>&& command)
{
    commandStack->Exec(std::move(command));
}

void DocumentData::BeginBatch(const DAVA::String& batchName, DAVA::uint32 commandsCount)
{
    commandStack->BeginBatch(batchName, commandsCount);
}

void DocumentData::EndBatch()
{
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

const SortedControlNodeSet& DocumentData::GetEditedRootControls() const
{
    return editedRootControls;
}

void DocumentData::SetSelectedNodes(const SelectedNodes& nodes)
{
    selection.selectedNodes = nodes;
}

void DocumentData::SetEditedRootControls(const SortedControlNodeSet& controls)
{
    editedRootControls = controls;
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

const char* DocumentData::packagePropertyName = "package";
const char* DocumentData::canSavePropertyName = "can save";
const char* DocumentData::canUndoPropertyName = "can undo";
const char* DocumentData::canRedoPropertyName = "can redo";
const char* DocumentData::undoTextPropertyName = "undo text";
const char* DocumentData::redoTextPropertyName = "redo text";
const char* DocumentData::selectionPropertyName = "selection";
const char* DocumentData::editedRootControlsPropertyName = "edited root controls";
