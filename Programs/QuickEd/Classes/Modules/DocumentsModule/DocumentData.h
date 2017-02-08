#pragma once

#include "Model/PackageHierarchy/PackageNode.h"

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

    DAVA::RefPtr<PackageNode> package;
    std::unique_ptr<DAVA::CommandStack> commandStack;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DocumentData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<DocumentData>::Begin()
        .Field(packagePropertyName, &DocumentData::package)
        .Field(canSavePropertyName, &DocumentData::CanSave, nullptr)
        .Field(canUndoPropertyName, &DocumentData::CanUndo, nullptr)
        .Field(canRedoPropertyName, &DocumentData::CanRedo, nullptr)
        .Field(undoTextPropertyName, &DocumentData::GetUndoText, nullptr)
        .Field(redoTextPropertyName, &DocumentData::GetRedoText, nullptr)
        .Field(canClosePropertyName, &DocumentData::canClosePropertyName)
        .End();
    }
};
