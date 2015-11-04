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
#include "UI/Commands/InsertControlCommand.h"
#include "UI/Commands/RemoveControlCommand.h"
#include "UI/Commands/InsertImportedPackageCommand.h"
#include "UI/Commands/RemoveImportedPackageCommand.h"
#include "UI/Commands/AddComponentCommand.h"
#include "UI/Commands/RemoveComponentCommand.h"
#include "UI/Commands/AttachComponentPrototypeSectionCommand.h"
#include "UI/Commands/InsertRemoveStyleCommand.h"
#include "UI/Commands/AddRemoveStylePropertyCommand.h"
#include "UI/Commands/AddRemoveStyleSelectorCommand.h"

#include "UI/Commands/ChangeStylePropertyCommand.h"

#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"

#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/StyleSheetSelectorProperty.h"

#include "Model/YamlPackageSerializer.h"
#include "Model/EditorUIPackageBuilder.h"

#include "UI/UIControl.h"
#include "UI/UIPackageLoader.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

#include "QtTools/ConsoleWidget/PointerSerializer.h"

using namespace DAVA;

namespace
{
    template<typename T>
    String FormatNodeNames(const DAVA::Vector<T*> &nodes)
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
}

QtModelPackageCommandExecutor::QtModelPackageCommandExecutor(Document *_document)
    : document(_document)
{
}

QtModelPackageCommandExecutor::~QtModelPackageCommandExecutor()
{
    document = nullptr;
}

void QtModelPackageCommandExecutor::AddImportedPackagesIntoPackage(const DAVA::Vector<DAVA::FilePath> packagePaths, PackageNode *package)
{
    Vector<PackageNode*> importedPackages;
    for (const FilePath &path : packagePaths)
    {
        if (package->FindImportedPackage(path) == nullptr && package->GetPath().GetFrameworkPath() != path.GetFrameworkPath())
        {
            EditorUIPackageBuilder builder;
            if (UIPackageLoader().LoadPackage(path, &builder))
            {
                RefPtr<PackageNode> importedPackage = builder.BuildPackage();
                if (package->GetImportedPackagesNode()->CanInsertImportedPackage(importedPackage.Get()))
                {
                    importedPackages.push_back(SafeRetain(importedPackage.Get()));
                }
            }
        }
    }
    
    if (!importedPackages.empty())
    {
        BeginMacro("Insert Packages");
        for (PackageNode *importedPackage : importedPackages)
        {
            AddImportedPackageIntoPackageImpl(importedPackage, package);
            SafeRelease(importedPackage);
        }
        importedPackages.clear();
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::RemoveImportedPackagesFromPackage(const DAVA::Vector<PackageNode*> &importedPackages, PackageNode *package)
{
    DAVA::Vector<PackageNode*> checkedPackages;
    for (PackageNode *testPackage : importedPackages)
    {
        bool canRemove = true;
        for (int i = 0; i < package->GetPackageControlsNode()->GetCount(); i++)
        {
            ControlNode *control = package->GetPackageControlsNode()->Get(i);
            if (control->IsDependsOnPackage(testPackage))
            {
                canRemove = false;
                break;
            }
        }
        if (canRemove)
            checkedPackages.push_back(testPackage);
    }
    
    if (!checkedPackages.empty())
    {
        BeginMacro("Remove Imported Packages");
        for (PackageNode *importedPackage : checkedPackages)
        {
            PushCommand(new RemoveImportedPackageCommand(package, importedPackage));
        }
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::ChangeProperty(const Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>>& properties, size_t hash)
{
    Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>> propertiesToChange;
    for (const auto& property : properties)
    {
        if (!std::get<1>(property)->IsReadOnly())
        {
            propertiesToChange.emplace_back(property);
        }
    }
    if (!propertiesToChange.empty())
    {
        QUndoCommand* command = new ChangePropertyValueCommand(document->GetPackage(), propertiesToChange, hash);
        PushCommand(command);
    }
}

void QtModelPackageCommandExecutor::ChangeProperty(ControlNode* node, AbstractProperty* property, const VariantType& value, size_t hash)
{
    if (!property->IsReadOnly())
    {
        QUndoCommand* command = new ChangePropertyValueCommand(document->GetPackage(), node, property, value, hash);
        PushCommand(command);
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
        int32 index = node->GetControl()->GetComponentCount(componentType);
        AddComponentImpl(node, componentType, index, nullptr);
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

void QtModelPackageCommandExecutor::ChangeProperty(StyleSheetNode *node, AbstractProperty *property, const DAVA::VariantType &value)
{
    if (!property->IsReadOnly())
    {
        PushCommand(new ChangeStylePropertyCommand(document->GetPackage(), node, property, value));
    }
}

void QtModelPackageCommandExecutor::AddStyleProperty(StyleSheetNode *node, uint32 propertyIndex)
{
    if (node->GetRootProperty()->CanAddProperty(propertyIndex))
    {
        UIStyleSheetProperty prop(propertyIndex, UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex).defaultValue);
        ScopedPtr<StyleSheetProperty> property(new StyleSheetProperty(prop));
        PushCommand(new AddRemoveStylePropertyCommand(document->GetPackage(), node, property, true));
    }
}

void QtModelPackageCommandExecutor::RemoveStyleProperty(StyleSheetNode *node, DAVA::uint32 propertyIndex)
{
    if (node->GetRootProperty()->CanRemoveProperty(propertyIndex))
    {
        StyleSheetProperty *property = node->GetRootProperty()->FindPropertyByPropertyIndex(propertyIndex);
        if (property)
        {
            PushCommand(new AddRemoveStylePropertyCommand(document->GetPackage(), node, property, false));
        }
    }
}

void QtModelPackageCommandExecutor::AddStyleSelector(StyleSheetNode *node)
{
    if (node->GetRootProperty()->CanAddSelector())
    {
        UIStyleSheetSelectorChain chain;
        ScopedPtr<StyleSheetSelectorProperty> property(new StyleSheetSelectorProperty(chain));
        PushCommand(new AddRemoveStyleSelectorCommand(document->GetPackage(), node, property, true));
    }
}

void QtModelPackageCommandExecutor::RemoveStyleSelector(StyleSheetNode *node, DAVA::int32 selectorIndex)
{
    if (node->GetRootProperty()->CanRemoveSelector())
    {
        UIStyleSheetSelectorChain chain;
        StyleSheetSelectorProperty *property = node->GetRootProperty()->GetSelectorAtIndex(selectorIndex);
        PushCommand(new AddRemoveStyleSelectorCommand(document->GetPackage(), node, property, false));
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
        Logger::Warning("%s", String("Can not insert control!" + PointerSerializer::FromPointer(control)).c_str());
    }
    return resultList;
}

void QtModelPackageCommandExecutor::InsertInstances(const DAVA::Vector<ControlNode*> &controls, ControlsContainerNode *dest, DAVA::int32 destIndex)
{
    Vector<ControlNode*> nodesToInsert;
    for (ControlNode *node : controls)
    {
        if (node->CanCopy() && dest->CanInsertControl(node, destIndex))
            nodesToInsert.push_back(node);
    }
    
    if (!nodesToInsert.empty())
    {
        BeginMacro(Format("Instance Controls %s", FormatNodeNames(nodesToInsert).c_str()).c_str());
        
        int index = destIndex;
        for (ControlNode *node : nodesToInsert)
        {
            ControlNode *copy = ControlNode::CreateFromPrototype(node);
            InsertControlImpl(copy, dest, index);
            SafeRelease(copy);
            index++;
        }
        
        EndMacro();
    }

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
        BeginMacro(Format("Copy Controls %s", FormatNodeNames(nodes).c_str()).c_str());
        
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
        BeginMacro(Format("Move Controls %s", FormatNodeNames(nodes).c_str()).c_str());
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

ResultList QtModelPackageCommandExecutor::InsertStyle(StyleSheetNode *styleSheetNode, StyleSheetsNode *dest, DAVA::int32 destIndex)
{
    ResultList resultList;
    if (dest->CanInsertStyle(styleSheetNode, destIndex))
    {
        PushCommand(new InsertRemoveStyleCommand(document->GetPackage(), styleSheetNode, dest, destIndex, true));
    }
    else
    {
        resultList.AddResult(Result::RESULT_ERROR, "Can not instert style sheet!");
    }
    
    return resultList;
}

void QtModelPackageCommandExecutor::CopyStyles(const DAVA::Vector<StyleSheetNode*> &nodes, StyleSheetsNode *dest, DAVA::int32 destIndex)
{
    Vector<StyleSheetNode*> nodesToCopy;
    for (StyleSheetNode *node : nodes)
    {
        if (node->CanCopy() && dest->CanInsertStyle(node, destIndex))
            nodesToCopy.push_back(node);
    }
    
    if (!nodesToCopy.empty())
    {
        BeginMacro(Format("Copy Styles %s", FormatNodeNames(nodes).c_str()).c_str());
        
        int index = destIndex;
        for (StyleSheetNode *node : nodesToCopy)
        {
            StyleSheetNode *copy = node->Clone();
            PushCommand(new InsertRemoveStyleCommand(document->GetPackage(), copy, dest, index, true));
            SafeRelease(copy);
            index++;
        }
        
        EndMacro();
    }

}

void QtModelPackageCommandExecutor::MoveStyles(const DAVA::Vector<StyleSheetNode*> &nodes, StyleSheetsNode *dest, DAVA::int32 destIndex)
{
    Vector<StyleSheetNode*> nodesToMove;
    for (StyleSheetNode *node : nodes)
    {
        if (node->CanRemove() && dest->CanInsertStyle(node, destIndex))
            nodesToMove.push_back(node);
    }
    
    if (!nodesToMove.empty())
    {
        BeginMacro(Format("Move Styles %s", FormatNodeNames(nodes).c_str()).c_str());
        int index = destIndex;
        for (StyleSheetNode *node : nodesToMove)
        {
            StyleSheetsNode *src = dynamic_cast<StyleSheetsNode*>(node->GetParent());
            if (src)
            {
                int32 srcIndex = src->GetIndex(node);
                
                if (src == dest && index > srcIndex)
                    index--;
                
                node->Retain();
                PushCommand(new InsertRemoveStyleCommand(document->GetPackage(), node, src, srcIndex, false));
                if (IsNodeInHierarchy(dest))
                {
                    PushCommand(new InsertRemoveStyleCommand(document->GetPackage(), node, dest, index, true));
                }
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

void QtModelPackageCommandExecutor::Remove(const Vector<ControlNode*> &controls, const Vector<StyleSheetNode*> &styles)
{
    Vector<PackageBaseNode*> nodesToRemove;
    
    Vector<ControlNode*> controlsToRemove;
    for (ControlNode *control : controls)
    {
        if (control->CanRemove())
        {
            controlsToRemove.push_back(control);
            nodesToRemove.push_back(control);
        }
    }
    
    Vector<StyleSheetNode*> stylesToRemove;
    for (StyleSheetNode *style : styles)
    {
        if (style->CanRemove())
        {
            stylesToRemove.push_back(style);
            nodesToRemove.push_back(style);
        }
    }
    
    if (!nodesToRemove.empty())
    {
        BeginMacro(Format("Remove %s", FormatNodeNames(nodesToRemove).c_str()).c_str());
        for (ControlNode *control : controlsToRemove)
            RemoveControlImpl(control);
        for (StyleSheetNode *style : stylesToRemove)
        {
            StyleSheetsNode *src = dynamic_cast<StyleSheetsNode*>(style->GetParent());
            if (src)
            {
                int32 srcIndex = src->GetIndex(style);
                PushCommand(new InsertRemoveStyleCommand(document->GetPackage(), style, src, srcIndex, false));
            }
        }
        EndMacro();
    }
}

bool QtModelPackageCommandExecutor::Paste(PackageNode *root, PackageBaseNode *dest, int32 destIndex, const DAVA::String &data)
{
    if (dest->IsReadOnly())
        return false;
    
    ControlsContainerNode *controlsDest = dynamic_cast<ControlsContainerNode*>(dest);
    StyleSheetsNode *stylesDest = dynamic_cast<StyleSheetsNode*>(dest);
    
    if (controlsDest == nullptr && stylesDest == nullptr)
        return false;

    RefPtr<YamlParser> parser(YamlParser::CreateAndParseString(data));
    if (parser.Valid() && parser->GetRootNode())
    {
        EditorUIPackageBuilder builder;
        
        builder.AddImportedPackage(root);
        for (int32 i = 0; i < root->GetImportedPackagesNode()->GetCount(); i++)
        {
            builder.AddImportedPackage(root->GetImportedPackagesNode()->GetImportedPackage(i));
        }
        
        if (UIPackageLoader().LoadPackage(parser->GetRootNode(), "", &builder))
        {
            const Vector<PackageNode*> &importedPackages = builder.GetImportedPackages();
            const Vector<ControlNode*> &controls = builder.GetRootControls();
            const Vector<StyleSheetNode*> &styles = builder.GetStyles();

            if (controlsDest != nullptr)
            {
                Vector<ControlNode*> acceptedControls;
                Vector<PackageNode*> acceptedPackages;
                Vector<PackageNode*> declinedPackages;
                
                for (PackageNode *importedPackage : importedPackages)
                {
                    if (importedPackage != root && importedPackage->GetParent() != root->GetImportedPackagesNode())
                    {
                        if (root->GetImportedPackagesNode()->CanInsertImportedPackage(importedPackage))
                            acceptedPackages.push_back(importedPackage);
                        else
                            declinedPackages.push_back(importedPackage);
                    }
                }
                
                for (ControlNode *control : controls)
                {
                    if (dest->CanInsertControl(control, destIndex))
                    {
                        bool canInsert = true;
                        for (PackageNode *declinedPackage : declinedPackages)
                        {
                            if (control->IsDependsOnPackage(declinedPackage))
                            {
                                canInsert = false;
                                break;
                            }
                        }
                        
                        if (canInsert)
                        {
                            acceptedControls.push_back(control);
                        }
                    }
                }

                if (!acceptedControls.empty())
                {
                    BeginMacro("Paste");
                    for (PackageNode *importedPackage : acceptedPackages)
                    {
                        AddImportedPackageIntoPackageImpl(importedPackage, root);
                    }
                    
                    int32 index = destIndex;
                    for (ControlNode *control : acceptedControls)
                    {
                        InsertControl(control, controlsDest, index);
                        index++;
                    }
                    
                    EndMacro();
                    return true;
                }
            }
            else if (stylesDest != nullptr && !styles.empty())
            {
                BeginMacro("Paste");
                int32 index = destIndex;
                for (StyleSheetNode *style : styles)
                {
                    PushCommand(new InsertRemoveStyleCommand(document->GetPackage(), style, stylesDest, index, true));
                    index++;
                }
                
                EndMacro();
                return true;
            }
            
        }
        
    }
    return false;
}

void QtModelPackageCommandExecutor::AddImportedPackageIntoPackageImpl(PackageNode *importedPackage, PackageNode *package)
{
    PushCommand(new InsertImportedPackageCommand(package, importedPackage, package->GetImportedPackagesNode()->GetCount()));
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
            ControlNode *copy = ControlNode::CreateFromPrototypeChild(control);
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

void QtModelPackageCommandExecutor::AddComponentImpl(ControlNode *node, int32 typeIndex, int32 index, ComponentPropertiesSection *prototypeSection)
{
    UIComponent::eType type = static_cast<UIComponent::eType>(typeIndex);
    
    ComponentPropertiesSection *destSection = nullptr;
    if (!UIComponent::IsMultiple(type))
    {        
        destSection = node->GetRootProperty()->FindComponentPropertiesSection(type, index);
        if (destSection)
        {
            PushCommand(new AttachComponentPrototypeSectionCommand(document->GetPackage(), node, destSection, prototypeSection));
        }
    }
    
    if (destSection == nullptr)
    {
        ComponentPropertiesSection *section = new ComponentPropertiesSection(node->GetControl(), type, index, prototypeSection, prototypeSection ? AbstractProperty::CT_INHERIT : AbstractProperty::CT_COPY);
        PushCommand(new AddComponentCommand(document->GetPackage(), node, section));
        
        for (ControlNode *instance : node->GetInstances())
            AddComponentImpl(instance, type, index, section);
        
        SafeRelease(section);
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

