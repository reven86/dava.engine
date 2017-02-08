#pragma once

#include "Model/PackageHierarchy/PackageNode.h"
#include "Application/AnyHelpers.h"
#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataNode.h>

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>

#include <QString>

namespace DAVA
{
class CommandStack;
}
class PackageNode;

struct DocumentData : public DAVA::TArc::DataNode
{
    DocumentData(const DAVA::RefPtr<PackageNode>& package);
    ~DocumentData() override;

    QString GetName() const;
    QString GetPackageAbsolutePath() const;

    bool CanSave() const;
    bool CanUndo() const;
    bool CanRedo() const;

    QString GetUndoText() const;
    QString GetRedoText() const;

    void RefreshLayout();
    void RefreshAllControlProperties();

    bool documentExists = true;

    bool canClose = true;

    static const char* packagePropertyName;
    static const char* canSavePropertyName;
    static const char* canUndoPropertyName;
    static const char* canRedoPropertyName;
    static const char* undoTextPropertyName;
    static const char* redoTextPropertyName;
    static const char* canClosePropertyName;
    static const char* selectionPropertyName;

    DAVA::RefPtr<PackageNode> package;
    std::unique_ptr<DAVA::CommandStack> commandStack;
    SelectedNodes selection;

    DAVA_VIRTUAL_REFLECTION(DocumentData, DAVA::TArc::DataNode);
};
