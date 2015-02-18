#include "QtModelPackageCommandExecutor.h"

#include "Document.h"
#include "PackageContext.h"

#include "UI/Commands/ChangePropertyValueCommand.h"
#include "UI/Commands/ChangeDefaultValueCommand.h"
#include "UI/Commands/InsertControlCommand.h"
#include "UI/Commands/RemoveControlCommand.h"
#include "UI/Commands/InsertImportedPackageCommand.h"

#include "UI/Package/PackageModel.h"

#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"

using namespace DAVA;

QtModelPackageCommandExecutor::QtModelPackageCommandExecutor(Document *_document)
    : document(_document)
{
    
}

QtModelPackageCommandExecutor::~QtModelPackageCommandExecutor()
{
    document = nullptr;
}

void QtModelPackageCommandExecutor::AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package)
{
    PackageModel *model = document->GetPackageContext()->GetModel();
    PushCommand(new InsertImportedPackageCommand(model, importedPackageControls, package, package->GetImportedPackagesNode()->GetCount()));
}

void QtModelPackageCommandExecutor::ChangeProperty(ControlNode *node, BaseProperty *property, const DAVA::VariantType &value)
{
    BeginMacro("Change Property");
    PushCommand(new ChangePropertyValueCommand(document, node, property, value));
    ChangeDefaultProperties(node->GetInstances(), property, value);
    EndMacro();
}

void QtModelPackageCommandExecutor::ResetProperty(ControlNode *node, BaseProperty *property)
{
    BeginMacro("Reset Property");
    PushCommand(new ChangePropertyValueCommand(document, node, property));
    ChangeDefaultProperties(node->GetInstances(), property, property->GetDefaultValue());
    EndMacro();
}

void QtModelPackageCommandExecutor::InsertControl(ControlNode *control, ControlsContainerNode *dest, DAVA::int32 destIndex)
{
    BeginMacro("Insert Control");
    PackageModel *model = document->GetPackageContext()->GetModel();
    PushCommand(new InsertControlCommand(model, control, dest, destIndex));
    
    ControlNode *destControl = dynamic_cast<ControlNode*>(dest);
    if (destControl)
    {
        const Vector<ControlNode*> &instances = destControl->GetInstances();
        for (ControlNode *instance : instances)
        {
            ControlNode *copy = ControlNode::CreateFromPrototypeChild(control, document->GetPackage()->GetPackageRef());
            InsertControl(copy, instance, destIndex);
            SafeRelease(copy);
        }
    }
    EndMacro();
}

void QtModelPackageCommandExecutor::CopyControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex)
{
    if (!nodes.empty())
    {
        BeginMacro("Copy Controls");
        int index = destIndex;
        for (ControlNode *node : nodes)
        {
            ControlNode *copy = node->Clone();
            InsertControl(copy, dest, index);
            SafeRelease(copy);
            index++;
        }
        
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::MoveControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex)
{
    if (!nodes.empty())
    {
        BeginMacro("Move Controls");
        int index = destIndex;
        for (ControlNode *node : nodes)
        {
            ControlsContainerNode *src = dynamic_cast<ControlsContainerNode*>(node->GetParent());
            if (src)
            {
                int32 srcIndex = src->GetIndex(node);
                
                if (src == dest && index > srcIndex)
                    index--;

                node->Retain();
                RemoveControl(node);
                InsertControl(node, dest, index);
                node->Release();
                
                index++;
            }
            else
            {
                DVASSERT(false);
            }
        }
        
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::RemoveControl(ControlNode* node)
{
    PackageModel *model = document->GetPackageContext()->GetModel();

    ControlsContainerNode *src = dynamic_cast<ControlsContainerNode*>(node->GetParent());
    if (src)
    {
        ControlNode *srcControl = dynamic_cast<ControlNode*>(src);
        if (srcControl == nullptr || srcControl->GetCreationType() == ControlNode::CREATED_FROM_CLASS || srcControl->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
        {
            int32 srcIndex = src->GetIndex(node);
            node->Retain();
            PushCommand(new RemoveControlCommand(model, node, src, srcIndex));
            
            const Vector<ControlNode*> &instances = node->GetInstances();
            RemoveControls(instances);
            
            node->Release();
        }
        else
        {
            // do nothing
        }
    }
    else
    {
        DVASSERT(false);
    }

}

void QtModelPackageCommandExecutor::RemoveControls(const DAVA::Vector<ControlNode*> &nodes)
{
    if (!nodes.empty())
    {
        BeginMacro("Remove Controls");
        for (ControlNode *node : nodes)
            RemoveControl(node);
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::ChangeDefaultProperties(const DAVA::Vector<ControlNode *> &instances, BaseProperty *property, const DAVA::VariantType &value)
{
    for (ControlNode *instance : instances)
    {
        Vector<String> path = property->GetPath();
        BaseProperty *nodeProperty = instance->GetPropertyByPath(path);
        if (nodeProperty)
        {
            PushCommand(new ChangeDefaultValueCommand(document, instance, nodeProperty, value));
            ChangeDefaultProperties(instance->GetInstances(), nodeProperty, nodeProperty->GetValue());
        }
        else
        {
            DVASSERT(false);
        }
    }
}

QUndoStack *QtModelPackageCommandExecutor::GetUndoStack()
{
    return document->UndoStack();
}

void QtModelPackageCommandExecutor::PushCommand(QUndoCommand *cmd)
{
    GetUndoStack()->push(cmd);
}

void QtModelPackageCommandExecutor::BeginMacro(const QString &name)
{
    GetUndoStack()->beginMacro(name);
}

void QtModelPackageCommandExecutor::EndMacro()
{
    GetUndoStack()->endMacro();
}
