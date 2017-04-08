#pragma once

#include "Model/PackageHierarchy/PackageNode.h"
#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataNode.h>
#include <TArc/Qt/QtString.h>

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>

namespace DAVA
{
class Command;
class CommandStack;
}
class PackageNode;

class DocumentData : public DAVA::TArc::DataNode
{
public:
    DocumentData(const DAVA::RefPtr<PackageNode>& package);
    ~DocumentData() override;

    PackageNode* GetPackageNode() const;

    void ExecCommand(std::unique_ptr<DAVA::Command>&& command);
    void BeginBatch(const DAVA::String& batchName, DAVA::uint32 commandsCount = 1);
    void EndBatch();
    template <typename T, typename... Arguments>
    std::unique_ptr<T> CreateCommand(Arguments&&... args) const;
    template <typename T, typename... Arguments>
    void ExecCommand(Arguments&&... args);

    const SelectedNodes& GetSelectedNodes() const;

    QString GetName() const;
    QString GetPackageAbsolutePath() const;
    DAVA::FilePath GetPackagePath() const;

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
std::unique_ptr<T> DocumentData::CreateCommand(Arguments&&... args) const
{
    return std::make_unique<T>(package.Get(), std::forward<Arguments>(args)...);
}

template <typename T, typename... Arguments>
void DocumentData::ExecCommand(Arguments&&... args)
{
    std::unique_ptr<DAVA::Command> command = CreateCommand<T>(std::forward<Arguments>(args)...);
    ExecCommand(std::move(command));
}
