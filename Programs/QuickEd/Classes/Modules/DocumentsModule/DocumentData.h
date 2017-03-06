#pragma once

#include "Model/PackageHierarchy/PackageNode.h"
#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataNode.h>
#include <TArc/DataProcessing/AnySupport/AnyQStringCompare.h>

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>

#include <QString>

namespace DAVA
{
class Command;
class CommandStack;
}
class PackageNode;
class QEPackageCommand;

class DocumentData : public DAVA::TArc::DataNode
{
public:
    DocumentData(const DAVA::RefPtr<PackageNode>& package);
    ~DocumentData() override;

    const PackageNode* GetPackageNode() const;
    const DAVA::CommandStack* GetCommandStack() const;

    template <typename T, typename... Arguments>
    std::unique_ptr<T> CreateQECommand(Arguments&&... args) const;

    void ExecCommand(std::unique_ptr<DAVA::Command>&& command);

    const SelectedNodes& GetSelectedNodes() const;

    QString GetName() const;
    QString GetPackageAbsolutePath() const;

    bool CanSave() const;
    bool CanUndo() const;
    bool CanRedo() const;

    QString GetUndoText() const;
    QString GetRedoText() const;

    bool IsDocumentExists() const;

    DAVA_DEPRECATED(void RefreshLayout());
    DAVA_DEPRECATED(void RefreshAllControlProperties());

    static const char* packagePropertyName;
    static const char* canSavePropertyName;
    static const char* canUndoPropertyName;
    static const char* canRedoPropertyName;
    static const char* undoTextPropertyName;
    static const char* redoTextPropertyName;
    static const char* selectionPropertyName;

private:
    friend class DocumentsModule;

    void SetSelectedNodes(const SelectedNodes& selection);

    DAVA::RefPtr<PackageNode> package;
    std::unique_ptr<DAVA::CommandStack> commandStack;
    SelectionContainer selection;

    bool documentExists = true;

    DAVA_VIRTUAL_REFLECTION(DocumentData, DAVA::TArc::DataNode);
};

template <typename T, typename... Arguments>
std::unique_ptr<T> DocumentData::CreateQECommand(Arguments&&... args) const
{
    static_assert(std::is_base_of<QEPackageCommand, T>::value, "T must be a class derived from QECommand!");
    return std::make_unique<T>(package.Get(), std::forward<Arguments>(args)...);
}
