#include "QtModelPackageCommandExecutor.h"

#include "Document/Document.h"
#include "Command/CommandStack.h"

#include "QECommands//ChangePropertyValueCommand.h"
#include "QECommands/InsertControlCommand.h"
#include "QECommands/RemoveControlCommand.h"
#include "QECommands/InsertImportedPackageCommand.h"
#include "QECommands/RemoveImportedPackageCommand.h"
#include "QECommands/AddComponentCommand.h"
#include "QECommands/RemoveComponentCommand.h"
#include "QECommands/AttachComponentPrototypeSectionCommand.h"
#include "QECommands/InsertRemoveStyleCommand.h"
#include "QECommands/AddRemoveStylePropertyCommand.h"
#include "QECommands/AddRemoveStyleSelectorCommand.h"

#include "QECommands/ChangeStylePropertyCommand.h"

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
#include "Model/QuickEdPackageBuilder.h"

#include "Project/Project.h"

#include "UI/UIControl.h"
#include "UI/UIPackageLoader.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

#include "QtTools/ConsoleWidget/PointerSerializer.h"

#include "Logger/Logger.h"
#include "Utils/StringFormat.h"

using namespace DAVA;

namespace
{
template <typename T>
String FormatNodeNames(const DAVA::Vector<T*>& nodes)
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

QtModelPackageCommandExecutor::QtModelPackageCommandExecutor(Project* project_, Document* document_)
    : project(project_)
    , document(document_)
    , packageNode(document->GetPackage())
{
}

QtModelPackageCommandExecutor::~QtModelPackageCommandExecutor()
{
    document = nullptr;
}

void QtModelPackageCommandExecutor::AddImportedPackagesIntoPackage(const DAVA::Vector<DAVA::FilePath> packagePaths, PackageNode* package)
{
    Vector<PackageNode*> importedPackages;
    for (const FilePath& path : packagePaths)
    {
        if (package->FindImportedPackage(path) == nullptr && package->GetPath().GetFrameworkPath() != path.GetFrameworkPath())
        {
            QuickEdPackageBuilder builder;
            if (UIPackageLoader(project->GetPrototypes()).LoadPackage(path, &builder))
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
        for (PackageNode* importedPackage : importedPackages)
        {
            AddImportedPackageIntoPackageImpl(importedPackage, package);
            SafeRelease(importedPackage);
        }
        importedPackages.clear();
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::RemoveImportedPackagesFromPackage(const DAVA::Vector<PackageNode*>& importedPackages, PackageNode* package)
{
    DAVA::Vector<PackageNode*> checkedPackages;
    for (PackageNode* testPackage : importedPackages)
    {
        bool canRemove = true;
        for (int i = 0; i < package->GetPackageControlsNode()->GetCount(); i++)
        {
            ControlNode* control = package->GetPackageControlsNode()->Get(i);
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
        for (PackageNode* importedPackage : checkedPackages)
        {
            ExecCommand(std::unique_ptr<Command>(new RemoveImportedPackageCommand(package, importedPackage)));
        }
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::ChangeProperty(ControlNode* node, AbstractProperty* property, const VariantType& value)
{
    if (!property->IsReadOnly())
    {
        ExecCommand(std::unique_ptr<Command>(new ChangePropertyValueCommand(packageNode, node, property, value)));
    }
}

void QtModelPackageCommandExecutor::ResetProperty(ControlNode* node, AbstractProperty* property)
{
    if (!property->IsReadOnly())
    {
        ExecCommand(std::unique_ptr<Command>(new ChangePropertyValueCommand(packageNode, node, property, VariantType())));
    }
}

void QtModelPackageCommandExecutor::AddComponent(ControlNode* node, uint32 componentType)
{
    if (node->GetRootProperty()->CanAddComponent(componentType))
    {
        const char* componentName = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(componentType);
        BeginMacro(Format("Add Component %s", componentName).c_str());
        int32 index = node->GetControl()->GetComponentCount(componentType);
        AddComponentImpl(node, componentType, index, nullptr);
        EndMacro();
    }
}

void QtModelPackageCommandExecutor::RemoveComponent(ControlNode* node, uint32 componentType, DAVA::uint32 componentIndex)
{
    if (node->GetRootProperty()->CanRemoveComponent(componentType))
    {
        ComponentPropertiesSection* section = node->GetRootProperty()->FindComponentPropertiesSection(componentType, componentIndex);
        if (section)
        {
            const char* componentName = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(componentType);
            BeginMacro(Format("Remove Component %s", componentName).c_str());
            RemoveComponentImpl(node, section);
            EndMacro();
        }
    }
}

void QtModelPackageCommandExecutor::ChangeProperty(StyleSheetNode* node, AbstractProperty* property, const DAVA::VariantType& value)
{
    if (!property->IsReadOnly())
    {
        ExecCommand(std::unique_ptr<Command>(new ChangeStylePropertyCommand(packageNode, node, property, value)));
    }
}

void QtModelPackageCommandExecutor::AddStyleProperty(StyleSheetNode* node, uint32 propertyIndex)
{
    if (node->GetRootProperty()->CanAddProperty(propertyIndex))
    {
        UIStyleSheetProperty prop(propertyIndex, UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyByIndex(propertyIndex).defaultValue);
        ScopedPtr<StyleSheetProperty> property(new StyleSheetProperty(prop));
        ExecCommand(std::unique_ptr<Command>(new AddRemoveStylePropertyCommand(packageNode, node, property, true)));
    }
}

void QtModelPackageCommandExecutor::RemoveStyleProperty(StyleSheetNode* node, DAVA::uint32 propertyIndex)
{
    if (node->GetRootProperty()->CanRemoveProperty(propertyIndex))
    {
        StyleSheetProperty* property = node->GetRootProperty()->FindPropertyByPropertyIndex(propertyIndex);
        if (property)
        {
            ExecCommand(std::unique_ptr<Command>(new AddRemoveStylePropertyCommand(packageNode, node, property, false)));
        }
    }
}

void QtModelPackageCommandExecutor::AddStyleSelector(StyleSheetNode* node)
{
    if (node->GetRootProperty()->CanAddSelector())
    {
        UIStyleSheetSelectorChain chain;
        UIStyleSheetSourceInfo sourceInfo(document->GetPackageFilePath());

        ScopedPtr<StyleSheetSelectorProperty> property(new StyleSheetSelectorProperty(chain, sourceInfo));
        ExecCommand(std::unique_ptr<Command>(new AddRemoveStyleSelectorCommand(packageNode, node, property, true)));
    }
}

void QtModelPackageCommandExecutor::RemoveStyleSelector(StyleSheetNode* node, DAVA::int32 selectorIndex)
{
    if (node->GetRootProperty()->CanRemoveSelector())
    {
        UIStyleSheetSelectorChain chain;
        StyleSheetSelectorProperty* property = node->GetRootProperty()->GetSelectorAtIndex(selectorIndex);
        ExecCommand(std::unique_ptr<Command>(new AddRemoveStyleSelectorCommand(packageNode, node, property, false)));
    }
}

ResultList QtModelPackageCommandExecutor::InsertControl(ControlNode* control, ControlsContainerNode* dest, DAVA::int32 destIndex)
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

Vector<ControlNode*> QtModelPackageCommandExecutor::InsertInstances(const DAVA::Vector<ControlNode*>& controls, ControlsContainerNode* dest, DAVA::int32 destIndex)
{
    Vector<ControlNode*> nodesToInsert;
    nodesToInsert.reserve(controls.size());
    for (ControlNode* node : controls)
    {
        if (node->CanCopy() && dest->CanInsertControl(node, destIndex))
            nodesToInsert.push_back(node);
    }
    Vector<ControlNode*> insertedNodes;
    insertedNodes.reserve(nodesToInsert.size());
    if (!nodesToInsert.empty())
    {
        BeginMacro(Format("Instance Controls %s", FormatNodeNames(nodesToInsert).c_str()).c_str());

        int index = destIndex;
        for (ControlNode* node : nodesToInsert)
        {
            ControlNode* copy = ControlNode::CreateFromPrototype(node);
            insertedNodes.push_back(copy);
            InsertControlImpl(copy, dest, index);
            SafeRelease(copy);
            index++;
        }

        EndMacro();
    }
    return insertedNodes;
}

Vector<ControlNode*> QtModelPackageCommandExecutor::CopyControls(const DAVA::Vector<ControlNode*>& nodes, ControlsContainerNode* dest, DAVA::int32 destIndex)
{
    Vector<RefPtr<ControlNode>> nodesToCopy;
    nodesToCopy.reserve(nodes.size());
    for (ControlNode* node : nodes)
    {
        RefPtr<ControlNode> copy(node->Clone());
        if (node->CanCopy() && dest->CanInsertControl(copy.Get(), destIndex))
            nodesToCopy.push_back(copy);
    }
    Vector<ControlNode*> copiedNodes;
    copiedNodes.reserve(nodesToCopy.size());
    if (!nodesToCopy.empty())
    {
        BeginMacro(Format("Copy Controls %s", FormatNodeNames(nodes).c_str()).c_str());

        int32 index = destIndex;
        for (const RefPtr<ControlNode>& copy : nodesToCopy)
        {
            copiedNodes.push_back(copy.Get());
            InsertControlImpl(copy.Get(), dest, index);
            index++;
        }
        nodesToCopy.clear();

        EndMacro();
    }
    return copiedNodes;
}

Vector<ControlNode*> QtModelPackageCommandExecutor::MoveControls(const DAVA::Vector<ControlNode*>& nodes, ControlsContainerNode* dest, DAVA::int32 destIndex)
{
    Vector<ControlNode*> nodesToMove;
    nodesToMove.reserve(nodes.size());
    for (ControlNode* node : nodes)
    {
        if (node->CanMoveTo(dest, destIndex))
            nodesToMove.push_back(node);
    }
    Vector<ControlNode*> movedNodes;
    movedNodes.reserve(nodesToMove.size());
    if (!nodesToMove.empty())
    {
        BeginMacro(Format("Move Controls %s", FormatNodeNames(nodes).c_str()).c_str());
        int index = destIndex;
        for (ControlNode* node : nodesToMove)
        {
            ControlsContainerNode* src = dynamic_cast<ControlsContainerNode*>(node->GetParent());
            if (src)
            {
                int32 srcIndex = src->GetIndex(node);

                if (src == dest && index > srcIndex)
                    index--;

                if (MoveControlImpl(node, dest, index))
                {
                    movedNodes.push_back(node);
                }

                index++;
            }
            else
            {
                DVASSERT(false);
            }
        }

        EndMacro();
    }
    return movedNodes;
}

ResultList QtModelPackageCommandExecutor::InsertStyle(StyleSheetNode* styleSheetNode, StyleSheetsNode* dest, DAVA::int32 destIndex)
{
    ResultList resultList;
    if (dest->CanInsertStyle(styleSheetNode, destIndex))
    {
        ExecCommand(std::unique_ptr<Command>(new InsertRemoveStyleCommand(packageNode, styleSheetNode, dest, destIndex, true)));
    }
    else
    {
        resultList.AddResult(Result::RESULT_ERROR, "Can not instert style sheet!");
    }

    return resultList;
}

void QtModelPackageCommandExecutor::CopyStyles(const DAVA::Vector<StyleSheetNode*>& nodes, StyleSheetsNode* dest, DAVA::int32 destIndex)
{
    Vector<StyleSheetNode*> nodesToCopy;
    for (StyleSheetNode* node : nodes)
    {
        if (node->CanCopy() && dest->CanInsertStyle(node, destIndex))
            nodesToCopy.push_back(node);
    }

    if (!nodesToCopy.empty())
    {
        BeginMacro(Format("Copy Styles %s", FormatNodeNames(nodes).c_str()).c_str());

        int index = destIndex;
        for (StyleSheetNode* node : nodesToCopy)
        {
            StyleSheetNode* copy = node->Clone();
            ExecCommand(std::unique_ptr<Command>(new InsertRemoveStyleCommand(packageNode, copy, dest, index, true)));
            SafeRelease(copy);
            index++;
        }

        EndMacro();
    }
}

void QtModelPackageCommandExecutor::MoveStyles(const DAVA::Vector<StyleSheetNode*>& nodes, StyleSheetsNode* dest, DAVA::int32 destIndex)
{
    Vector<StyleSheetNode*> nodesToMove;
    for (StyleSheetNode* node : nodes)
    {
        if (node->CanRemove() && dest->CanInsertStyle(node, destIndex))
            nodesToMove.push_back(node);
    }

    if (!nodesToMove.empty())
    {
        BeginMacro(Format("Move Styles %s", FormatNodeNames(nodes).c_str()).c_str());
        int index = destIndex;
        for (StyleSheetNode* node : nodesToMove)
        {
            StyleSheetsNode* src = dynamic_cast<StyleSheetsNode*>(node->GetParent());
            if (src)
            {
                int32 srcIndex = src->GetIndex(node);

                if (src == dest && index > srcIndex)
                    index--;

                node->Retain();
                ExecCommand(std::unique_ptr<Command>(new InsertRemoveStyleCommand(packageNode, node, src, srcIndex, false)));
                if (IsNodeInHierarchy(dest))
                {
                    ExecCommand(std::unique_ptr<Command>(new InsertRemoveStyleCommand(packageNode, node, dest, index, true)));
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

void QtModelPackageCommandExecutor::Remove(const Vector<ControlNode*>& controls, const Vector<StyleSheetNode*>& styles)
{
    Vector<PackageBaseNode*> nodesToRemove;

    Vector<ControlNode*> controlsToRemove;
    for (ControlNode* control : controls)
    {
        if (control->CanRemove())
        {
            bool hasPrototype = std::find_if(controls.begin(), controls.end(), [control](const ControlNode* otherControl) {
                                    return control->GetPrototype() == otherControl;
                                }) != controls.end();
            if (!hasPrototype)
            {
                controlsToRemove.push_back(control);
                nodesToRemove.push_back(control);
            }
        }
    }

    Vector<StyleSheetNode*> stylesToRemove;
    for (StyleSheetNode* style : styles)
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
        for (ControlNode* control : controlsToRemove)
            RemoveControlImpl(control);
        for (StyleSheetNode* style : stylesToRemove)
        {
            StyleSheetsNode* src = dynamic_cast<StyleSheetsNode*>(style->GetParent());
            if (src)
            {
                int32 srcIndex = src->GetIndex(style);
                ExecCommand(std::unique_ptr<Command>(new InsertRemoveStyleCommand(packageNode, style, src, srcIndex, false)));
            }
        }
        EndMacro();
    }
}

Vector<PackageBaseNode*> QtModelPackageCommandExecutor::Paste(PackageNode* root, PackageBaseNode* dest, int32 destIndex, const DAVA::String& data)
{
    Vector<PackageBaseNode*> createdNodes;
    if (dest->IsReadOnly())
        return createdNodes;

    ControlsContainerNode* controlsDest = dynamic_cast<ControlsContainerNode*>(dest);
    StyleSheetsNode* stylesDest = dynamic_cast<StyleSheetsNode*>(dest);

    if (controlsDest == nullptr && stylesDest == nullptr)
        return createdNodes;

    RefPtr<YamlParser> parser(YamlParser::CreateAndParseString(data));
    if (!parser.Valid() || !parser->GetRootNode())
    {
        return createdNodes;
    }

    QuickEdPackageBuilder builder;

    builder.AddImportedPackage(root);
    for (int32 i = 0; i < root->GetImportedPackagesNode()->GetCount(); i++)
    {
        builder.AddImportedPackage(root->GetImportedPackagesNode()->GetImportedPackage(i));
    }

    if (UIPackageLoader(project->GetPrototypes()).LoadPackage(parser->GetRootNode(), root->GetPath(), &builder))
    {
        const Vector<PackageNode*>& importedPackages = builder.GetImportedPackages();
        const Vector<ControlNode*>& controls = builder.GetRootControls();
        const Vector<StyleSheetNode*>& styles = builder.GetStyles();

        if (controlsDest != nullptr)
        {
            Vector<ControlNode*> acceptedControls;
            Vector<PackageNode*> acceptedPackages;
            Vector<PackageNode*> declinedPackages;

            for (PackageNode* importedPackage : importedPackages)
            {
                if (importedPackage != root && importedPackage->GetParent() != root->GetImportedPackagesNode())
                {
                    if (root->GetImportedPackagesNode()->CanInsertImportedPackage(importedPackage))
                        acceptedPackages.push_back(importedPackage);
                    else
                        declinedPackages.push_back(importedPackage);
                }
            }

            for (ControlNode* control : controls)
            {
                if (dest->CanInsertControl(control, destIndex))
                {
                    bool canInsert = true;
                    for (PackageNode* declinedPackage : declinedPackages)
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
                for (PackageNode* importedPackage : acceptedPackages)
                {
                    AddImportedPackageIntoPackageImpl(importedPackage, root);
                }

                int32 index = destIndex;
                for (ControlNode* control : acceptedControls)
                {
                    createdNodes.push_back(control);
                    InsertControl(control, controlsDest, index);
                    index++;
                }

                EndMacro();
            }
        }
        else if (stylesDest != nullptr && !styles.empty())
        {
            BeginMacro("Paste");
            int32 index = destIndex;
            for (StyleSheetNode* style : styles)
            {
                createdNodes.push_back(style);
                ExecCommand(std::unique_ptr<Command>(new InsertRemoveStyleCommand(packageNode, style, stylesDest, index, true)));
                index++;
            }

            EndMacro();
        }
    }
    return createdNodes;
}

void QtModelPackageCommandExecutor::AddImportedPackageIntoPackageImpl(PackageNode* importedPackage, PackageNode* package)
{
    ExecCommand(std::unique_ptr<Command>(new InsertImportedPackageCommand(package, importedPackage, package->GetImportedPackagesNode()->GetCount())));
}

void QtModelPackageCommandExecutor::InsertControlImpl(ControlNode* control, ControlsContainerNode* dest, DAVA::int32 destIndex)
{
    ExecCommand(std::unique_ptr<Command>(new InsertControlCommand(packageNode, control, dest, destIndex)));

    ControlNode* destControl = dynamic_cast<ControlNode*>(dest);
    if (destControl)
    {
        const Vector<ControlNode*>& instances = destControl->GetInstances();
        for (ControlNode* instance : instances)
        {
            ControlNode* copy = ControlNode::CreateFromPrototypeChild(control);
            InsertControlImpl(copy, instance, destIndex);
            SafeRelease(copy);
        }
    }
}

void QtModelPackageCommandExecutor::RemoveControlImpl(ControlNode* node)
{
    ControlsContainerNode* src = dynamic_cast<ControlsContainerNode*>(node->GetParent());
    if (src)
    {
        int32 srcIndex = src->GetIndex(node);
        node->Retain();
        ExecCommand(std::unique_ptr<Command>(new RemoveControlCommand(packageNode, node, src, srcIndex)));

        Vector<ControlNode*> instances = node->GetInstances();
        for (ControlNode* instance : instances)
            RemoveControlImpl(instance);

        node->Release();
    }
    else
    {
        DVASSERT(false);
    }
}

bool QtModelPackageCommandExecutor::MoveControlImpl(ControlNode* node, ControlsContainerNode* dest, DAVA::int32 destIndex)
{
    node->Retain();
    ControlsContainerNode* src = dynamic_cast<ControlsContainerNode*>(node->GetParent());
    bool result = false;
    if (src)
    {
        int32 srcIndex = src->GetIndex(node);
        ExecCommand(std::unique_ptr<Command>(new RemoveControlCommand(packageNode, node, src, srcIndex)));

        Vector<ControlNode*> instances = node->GetInstances();

        if (IsNodeInHierarchy(dest))
        {
            ExecCommand(std::unique_ptr<Command>(new InsertControlCommand(packageNode, node, dest, destIndex)));

            ControlNode* destControl = dynamic_cast<ControlNode*>(dest);
            if (destControl)
            {
                for (ControlNode* destInstance : destControl->GetInstances())
                {
                    auto it = std::find_if(instances.begin(), instances.end(), [destInstance](const ControlNode* node) {
                        return IsControlNodesHasSameParentControlNode(node, destInstance);
                    });

                    if (it != instances.end())
                    {
                        ControlNode* srcInstance = *it;
                        instances.erase(it);
                        MoveControlImpl(srcInstance, destInstance, destIndex);
                    }
                    else
                    {
                        ControlNode* copy = ControlNode::CreateFromPrototypeChild(node);
                        InsertControlImpl(copy, destInstance, destIndex);
                        SafeRelease(copy);
                    }
                }
            }

            result = true;
        }

        for (ControlNode* instance : instances)
            RemoveControlImpl(instance);
    }
    else
    {
        DVASSERT(false);
    }

    node->Release();
    return result;
}

void QtModelPackageCommandExecutor::AddComponentImpl(ControlNode* node, int32 typeIndex, int32 index, ComponentPropertiesSection* prototypeSection)
{
    UIComponent::eType type = static_cast<UIComponent::eType>(typeIndex);

    ComponentPropertiesSection* destSection = nullptr;
    if (!UIComponent::IsMultiple(type))
    {
        destSection = node->GetRootProperty()->FindComponentPropertiesSection(type, index);
        if (destSection)
        {
            ExecCommand(std::unique_ptr<Command>(new AttachComponentPrototypeSectionCommand(packageNode, node, destSection, prototypeSection)));
        }
    }

    if (destSection == nullptr)
    {
        ComponentPropertiesSection* section = new ComponentPropertiesSection(node->GetControl(), type, index, prototypeSection, prototypeSection ? AbstractProperty::CT_INHERIT : AbstractProperty::CT_COPY);
        ExecCommand(std::unique_ptr<Command>(new AddComponentCommand(packageNode, node, section)));

        for (ControlNode* instance : node->GetInstances())
            AddComponentImpl(instance, type, index, section);

        SafeRelease(section);
    }
}

void QtModelPackageCommandExecutor::RemoveComponentImpl(ControlNode* node, ComponentPropertiesSection* section)
{
    ExecCommand(std::unique_ptr<Command>(new RemoveComponentCommand(packageNode, node, section)));
    Vector<ControlNode*> instances = node->GetInstances();
    for (ControlNode* instance : instances)
    {
        ComponentPropertiesSection* instanceSection = instance->GetRootProperty()->FindComponentPropertiesSection(section->GetComponentType(), section->GetComponentIndex());
        RemoveComponentImpl(instance, instanceSection);
    }
}

bool QtModelPackageCommandExecutor::IsNodeInHierarchy(const PackageBaseNode* node) const
{
    PackageBaseNode* p = node->GetParent();
    PackageNode* root = packageNode;
    while (p)
    {
        if (p == root)
            return true;
        p = p->GetParent();
    }
    return false;
}

bool QtModelPackageCommandExecutor::IsControlNodesHasSameParentControlNode(const ControlNode* n1, const ControlNode* n2)
{
    for (const PackageBaseNode* t1 = n1; t1 != nullptr; t1 = t1->GetParent())
    {
        if (t1->GetControl() != nullptr)
        {
            for (const PackageBaseNode* t2 = n2; t2 != nullptr; t2 = t2->GetParent())
            {
                if (t2 == t1)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void QtModelPackageCommandExecutor::ExecCommand(std::unique_ptr<DAVA::Command>&& cmd)
{
    GetCommandStack()->Exec(std::move(cmd));
}

void QtModelPackageCommandExecutor::BeginMacro(const QString& name)
{
    GetCommandStack()->BeginBatch(name.toUtf8().data());
}

void QtModelPackageCommandExecutor::EndMacro()
{
    GetCommandStack()->EndBatch();
}

DAVA::CommandStack* QtModelPackageCommandExecutor::GetCommandStack() const
{
    return document->GetCommandStack();
}
