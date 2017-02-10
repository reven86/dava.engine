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
class CommandStack;
}
class PackageNode;

class DocumentData : public DAVA::TArc::DataNode
{
public:
    DocumentData(const DAVA::RefPtr<PackageNode>& package);
    ~DocumentData() override;

    const PackageNode* GetPackageNode() const;
    DAVA_DEPRECATED(DAVA::CommandStack* GetCommandStack() const;)

    const SelectedNodes& GetSelectedNodes() const;

    QString GetName() const;
    QString GetPackageAbsolutePath() const;

    bool CanSave() const;
    bool CanUndo() const;
    bool CanRedo() const;

    QString GetUndoText() const;
    QString GetRedoText() const;

    bool IsDocumentExists() const;
    bool CanClose() const;

    DAVA_DEPRECATED(void RefreshLayout();)
    DAVA_DEPRECATED(void RefreshAllControlProperties());

    static const char* packagePropertyName;
    static const char* canSavePropertyName;
    static const char* canUndoPropertyName;
    static const char* canRedoPropertyName;
    static const char* undoTextPropertyName;
    static const char* redoTextPropertyName;
    static const char* canClosePropertyName;
    static const char* selectionPropertyName;

private:
    friend class DocumentsModule;
    void SetSelectedNodes(const SelectedNodes& selection);

    DAVA::RefPtr<PackageNode> package;
    std::unique_ptr<DAVA::CommandStack> commandStack;
    SelectionContainer selection;

    bool documentExists = true;
    bool canClose = true;

    DAVA_VIRTUAL_REFLECTION(DocumentData, DAVA::TArc::DataNode);
};
