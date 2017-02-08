#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include <Command/Command.h>
#include <Command/CommandStack.h>

#include <QFileInfo>

DocumentData::DocumentData(const DAVA::RefPtr<PackageNode>& package_)
    : package(package_)
    , commandStack(new DAVA::CommandStack())
{
}

DocumentData::~DocumentData() = default;

QString DocumentData::GetName() const
{
    QFileInfo fileInfo(GetPackageAbsolutePath());
    return fileInfo.fileName();
}

QString DocumentData::GetPackageAbsolutePath() const
{
    return QString::fromStdString(package->GetPath().GetAbsolutePathname());
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
}

const char* DocumentData::packagePropertyName = "package";
const char* DocumentData::canSavePropertyName = "can save";
const char* DocumentData::canUndoPropertyName = "can undo";
const char* DocumentData::canRedoPropertyName = "can redo";
const char* DocumentData::undoTextPropertyName = "undo text";
const char* DocumentData::redoTextPropertyName = "redo text";
const char* DocumentData::canClosePropertyName = "can close";
