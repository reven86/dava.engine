/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "QtModelPackageCommandExecutor.h"

#include "Document.h"

#include "UI/Commands/ChangePropertyValueCommand.h"
#include "UI/Commands/ChangeDefaultValueCommand.h"
#include "UI/Commands/InsertControlCommand.h"
#include "UI/Commands/RemoveControlCommand.h"
#include "UI/Commands/InsertImportedPackageCommand.h"
#include "UI/Commands/AddComponentCommand.h"
#include "UI/Commands/RemoveComponentCommand.h"

#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"

#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "Model/ControlProperties/RootProperty.h"

#include "Model/YamlPackageSerializer.h"
#include "Model/EditorUIPackageBuilder.h"

#include "Base/Result.h"

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
    PushCommand(new InsertImportedPackageCommand(package, importedPackageControls, package->GetImportedPackagesNode()->GetCount()));
}

void QtModelPackageCommandExecutor::ChangeProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &value)
{
    if (!property->IsReadOnly())
    {
        PushCommand(new ChangePropertyValueCommand(document->GetPackage(), node, property, value));
    }
}

void QtModelPackageCommandExecutor::ResetProperty(ControlNode *node, AbstractProperty *property)
{
    if (!property->IsReadOnly())
    {
        PushCommand(new ChangePropertyValueCommand(document->GetPackage(), node, property));
    }
}

void QtModelPackageCommandExecutor::AddComponent(ControlNode *node, uint32 componentType)
{
    if (node->GetRootProperty()->CanAddComponent(componentType))
    {
        const char *componentName = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(componentType);
        BeginMacro(Format("Add Component %s", componentName).c_str());
        UIComponent::eType type = static_cast<UIComponent::eType>(componentType);
        int32 index = node->GetControl()->GetComponentCount(componentType);
        ComponentPropertiesSection *section = new ComponentPropertiesSection(node->GetControl(), type, index, nullptr, AbstractProperty::CT_COPY);
        AddComponentImpl(node, section);
        SafeRelease(section);
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::RemoveComponent(ControlNode *node, uint32 componentType, DAVA::uint32 componentIndex)
{
    if (node->GetRootProperty()->CanRemoveComponent(componentType))
    {
        ComponentPropertiesSection *section = node->GetRootProperty()->FindComponentPropertiesSection(componentType, componentIndex);
        if (section)
        {
            const char *componentName = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(componentType);
            BeginMacro(Format("Remove Component %s", componentName).c_str());
            RemoveComponentImpl(node, section);
            EndMacro();
        }
    }
}

ResultList QtModelPackageCommandExecutor::InsertControl(ControlNode *control, ControlsContainerNode *dest, DAVA::int32 destIndex)
{
    ResultList resultList;
    if (dest->CanInsertControl(control, destIndex))
    {
        BeginMacro(Format("Insert Control %s(%s)", control->GetName().c_str(), control->GetClassName().c_str()).c_str());
        InsertControlImpl(control, dest, destIndex);
        EndMacro();
    }
    else
    {
        resultList.AddResult(Result::RESULT_ERROR, "Can not inster control!", VariantType(reinterpret_cast<int64>(control)));
    }
    return resultList;
}

void QtModelPackageCommandExecutor::CopyControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex)
{
    Vector<ControlNode*> nodesToCopy;
    for (ControlNode *node : nodes)
    {
        if (node->CanCopy() && dest->CanInsertControl(node, destIndex))
            nodesToCopy.push_back(node);
    }

    if (!nodesToCopy.empty())
    {
        BeginMacro(Format("Copy Controls %s", FormatControlNames(nodes).c_str()).c_str());
        
        int index = destIndex;
        for (ControlNode *node : nodesToCopy)
        {
            ControlNode *copy = node->Clone();
            InsertControlImpl(copy, dest, index);
            SafeRelease(copy);
            index++;
        }
        
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::MoveControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex)
{
    Vector<ControlNode*> nodesToMove;
    for (ControlNode *node : nodes)
    {
        if (node->CanRemove() && dest->CanInsertControl(node, destIndex))
            nodesToMove.push_back(node);
    }

    if (!nodesToMove.empty())
    {
        BeginMacro(Format("Move Controls %s", FormatControlNames(nodes).c_str()).c_str());
        int index = destIndex;
        for (ControlNode *node : nodesToMove)
        {
            ControlsContainerNode *src = dynamic_cast<ControlsContainerNode*>(node->GetParent());
            if (src)
            {
                int32 srcIndex = src->GetIndex(node);
                
                if (src == dest && index > srcIndex)
                    index--;

                node->Retain();
                RemoveControlImpl(node);
                if (IsNodeInHierarchy(dest))
                    InsertControlImpl(node, dest, index);
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

void QtModelPackageCommandExecutor::RemoveControls(const DAVA::Vector<ControlNode*> &nodes)
{
    Vector<ControlNode*> nodesToRemove;
    for (ControlNode *node : nodes)
    {
        if (node->CanRemove())
            nodesToRemove.push_back(node);
    }
    
    if (!nodesToRemove.empty())
    {
        BeginMacro(Format("Remove Controls %s", FormatControlNames(nodes).c_str()).c_str());
        for (ControlNode *node : nodesToRemove)
            RemoveControlImpl(node);
        EndMacro();
    }
}

bool QtModelPackageCommandExecutor::Paste(PackageNode *root, ControlsContainerNode *dest, int32 destIndex, const DAVA::String &data)
{
    RefPtr<YamlParser> parser(YamlParser::CreateAndParseString(data));
    if (parser.Valid() && parser->GetRootNode())
    {
        BeginMacro("Paste");
        EditorUIPackageBuilder builder(root, dest, destIndex, this);
        UIPackage *newPackage = UIPackageLoader(&builder).LoadPackage(parser->GetRootNode(), "");
        bool completed = newPackage != nullptr;
        SafeRelease(newPackage);
        EndMacro();
        
        if (completed)
            return true;
    }
    return false;
}

void QtModelPackageCommandExecutor::InsertControlImpl(ControlNode *control, ControlsContainerNode *dest, DAVA::int32 destIndex)
{
    PushCommand(new InsertControlCommand(document->GetPackage(), control, dest, destIndex));
    
    ControlNode *destControl = dynamic_cast<ControlNode*>(dest);
    if (destControl)
    {
        const Vector<ControlNode*> &instances = destControl->GetInstances();
        for (ControlNode *instance : instances)
        {
            ControlNode *copy = ControlNode::CreateFromPrototypeChild(control, document->GetPackage()->GetPackageRef());
            InsertControlImpl(copy, instance, destIndex);
            SafeRelease(copy);
        }
    }
}

void QtModelPackageCommandExecutor::RemoveControlImpl(ControlNode* node)
{
    ControlsContainerNode *src = dynamic_cast<ControlsContainerNode*>(node->GetParent());
    if (src)
    {
        int32 srcIndex = src->GetIndex(node);
        node->Retain();
        PushCommand(new RemoveControlCommand(document->GetPackage(), node, src, srcIndex));
        
        Vector<ControlNode*> instances = node->GetInstances();
        for (ControlNode *instance : instances)
            RemoveControlImpl(instance);

        node->Release();
    }
    else
    {
        DVASSERT(false);
    }
    
}

void QtModelPackageCommandExecutor::AddComponentImpl(ControlNode *node, ComponentPropertiesSection *section)
{
    PushCommand(new AddComponentCommand(document->GetPackage(), node, section));
    Vector<ControlNode*> instances = node->GetInstances();
    for (ControlNode *instance : instances)
    {
        UIComponent::eType type = static_cast<UIComponent::eType>(section->GetComponentType());
        int32 index = section->GetComponentIndex();
        ComponentPropertiesSection *instanceSection = new ComponentPropertiesSection(instance->GetControl(), type, index, section, AbstractProperty::CT_INHERIT);
        AddComponentImpl(instance, instanceSection);
        SafeRelease(instanceSection);
    }
}

void QtModelPackageCommandExecutor::RemoveComponentImpl(ControlNode *node, ComponentPropertiesSection *section)
{
    PushCommand(new RemoveComponentCommand(document->GetPackage(), node, section));
    Vector<ControlNode*> instances = node->GetInstances();
    for (ControlNode *instance : instances)
    {
        ComponentPropertiesSection *instanceSection = instance->GetRootProperty()->FindComponentPropertiesSection(section->GetComponentType(), section->GetComponentIndex());
        RemoveComponentImpl(instance, instanceSection);
    }
}

bool QtModelPackageCommandExecutor::IsNodeInHierarchy(const PackageBaseNode *node) const
{
    PackageBaseNode *p = node->GetParent();
    PackageNode *root = document->GetPackage();
    while (p)
    {
        if (p == root)
            return true;
        p = p->GetParent();
    }
    return false;
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

QUndoStack *QtModelPackageCommandExecutor::GetUndoStack()
{
    return document->GetUndoStack();
}

String QtModelPackageCommandExecutor::FormatControlNames(const DAVA::Vector<ControlNode*> &nodes)
{
    const size_t maxControlNames = 3;
    String list;
    for (size_t i = 0; i < nodes.size() && i < maxControlNames; i++)
    {
        if (i != 0)
            list += ", ";
        list += nodes[i]->GetName();
    }
    
    if (nodes.size() > maxControlNames)
        list += ", etc.";
    
    return list;
}
