#include "EditorUIPackageBuilder.h"

#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/ControlPropertiesSection.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "Model/ControlProperties/BackgroundPropertiesSection.h"
#include "Model/ControlProperties/InternalControlPropertiesSection.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "Model/ControlProperties/CustomClassProperty.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/ControlPrototype.h"
#include "Model/PackageHierarchy/PackageRef.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageCommandExecutor.h"
#include "UI/UIPackage.h"

using namespace DAVA;

const String EXCEPTION_CLASS_UI_TEXT_FIELD = "UITextField";
const String EXCEPTION_CLASS_UI_LIST = "UIList";

EditorUIPackageBuilder::EditorUIPackageBuilder(PackageNode *_basePackage, ControlsContainerNode *_insertingTarget, int32 _insertingIndex, PackageCommandExecutor *_commandExecutor)
    : packageNode(nullptr)
    , basePackage(_basePackage)
    , insertingTarget(_insertingTarget)
    , insertingIndex(_insertingIndex)
    , currentObject(nullptr)
    , commandExecutor(SafeRetain(_commandExecutor))
{
    if (!commandExecutor)
        commandExecutor = new DefaultPackageCommandExecutor();
}

EditorUIPackageBuilder::~EditorUIPackageBuilder()
{
    SafeRelease(commandExecutor);
    SafeRelease(packageNode);
    basePackage = nullptr;
    insertingTarget = nullptr;
}

void EditorUIPackageBuilder::BeginPackage(const FilePath &packagePath)
{
    DVASSERT(packageNode == nullptr);
    SafeRelease(packageNode);
    
    if (basePackage) // if there is exists base package we skip creating new one
    {
        RefPtr<UIPackage> package(SafeRetain(basePackage->GetPackageRef()->GetPackage()));
        packageNode = SafeRetain(basePackage);
    }
    else
    {
        RefPtr<UIPackage> package(new UIPackage());
        ScopedPtr<PackageRef> ref(new PackageRef(packagePath, package.Get()));
        packageNode = new PackageNode(ref);
    }
}

void EditorUIPackageBuilder::EndPackage()
{
    DVASSERT(packageNode != nullptr);
}

bool EditorUIPackageBuilder::ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader)
{
    PackageControlsNode *importedPackage = packageNode->FindImportedPackage(packagePath);
    if (importedPackage)
        return true;
    
    EditorUIPackageBuilder *builder = new EditorUIPackageBuilder();
    PackageControlsNode *controlsNode = nullptr;
    if (loader->LoadPackage(packagePath, builder))
    {
        controlsNode = SafeRetain(builder->packageNode->GetPackageControlsNode());
        controlsNode->SetName(builder->packageNode->GetName());
    }
    SafeDelete(builder);
    
    if (controlsNode)
    {
        if (basePackage)
            commandExecutor->AddImportedPackageIntoPackage(controlsNode, packageNode);
        else
            packageNode->GetImportedPackagesNode()->Add(controlsNode);
        
        SafeRelease(controlsNode);

        return true;
    }
    else
    {
        DVASSERT(false);
        return false;
    }
}

UIControl *EditorUIPackageBuilder::BeginControlWithClass(const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    controlsStack.push_back(ControlDescr(ControlNode::CreateFromControl(control), true));
    return control;
}

UIControl *EditorUIPackageBuilder::BeginControlWithCustomClass(const String &customClassName, const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);

    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    ControlNode *node = ControlNode::CreateFromControl(control);
    node->GetRootProperty()->GetCustomClassProperty()->SetValue(VariantType(customClassName));
    controlsStack.push_back(ControlDescr(node, true));
    return control;
}

UIControl *EditorUIPackageBuilder::BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String *customClassName, AbstractUIPackageLoader *loader)
{
    PackageControlsNode *controlsNode = nullptr;
    if (packageName.empty())
        controlsNode = packageNode->GetPackageControlsNode();
    else
        controlsNode = packageNode->GetImportedPackagesNode()->FindPackageControlsNodeByName(packageName);

    ControlNode *prototypeNode = nullptr;
    if (controlsNode)
    {
        prototypeNode = controlsNode->FindControlNodeByName(prototypeName);
        if (prototypeNode == nullptr && packageName.empty())
        {
            if (loader->LoadControlByName(prototypeName, this))
                prototypeNode = controlsNode->FindControlNodeByName(prototypeName);
        }
        
    }
    
    DVASSERT(prototypeNode);
    ControlNode *node = ControlNode::CreateFromPrototype(prototypeNode, controlsNode->GetPackageRef());
    if (customClassName)
        node->GetRootProperty()->GetCustomClassProperty()->SetValue(VariantType(*customClassName));
    controlsStack.push_back(ControlDescr(node, true));

    return node->GetControl();
}

UIControl *EditorUIPackageBuilder::BeginControlWithPath(const String &pathName)
{
    ControlNode *control = nullptr;
    if (!controlsStack.empty())
    {
        control = controlsStack.back().node;
        Vector<String> controlNames;
        Split(pathName, "/", controlNames, false, true);
        for (Vector<String>::const_iterator iter = controlNames.begin(); iter!=controlNames.end(); ++iter)
        {
            control = control->FindByName(*iter);
            if (!control)
                break;
        }
    }

    controlsStack.push_back(ControlDescr(SafeRetain(control), false));

    if (!control)
        return nullptr;

    return control->GetControl();
}

UIControl *EditorUIPackageBuilder::BeginUnknownControl(const YamlNode *node)
{
    DVASSERT(false);
    return nullptr;
}

void EditorUIPackageBuilder::EndControl(bool isRoot)
{
    ControlNode *lastControl = SafeRetain(controlsStack.back().node);
    bool addToParent = controlsStack.back().addToParent;
    controlsStack.pop_back();
    
    if (addToParent)
    {
        if (controlsStack.empty() || isRoot)
        {
            if (basePackage)
            {
                ControlsContainerNode *container;
                if (insertingTarget)
                    container = insertingTarget;
                else
                    container = packageNode->GetPackageControlsNode();
                
                int32 index = insertingIndex;
                if (index == -1)
                    index = container->GetCount();
                commandExecutor->InsertControl(lastControl, container, index);
            }
            else
            {
                packageNode->GetPackageControlsNode()->Add(lastControl);
            }
        }
        else
        {
            controlsStack.back().node->Add(lastControl);
        }
    }
    SafeRelease(lastControl);
}

void EditorUIPackageBuilder::BeginControlPropertiesSection(const String &name)
{
    currentSection = controlsStack.back().node->GetRootProperty()->GetControlPropertiesSection(name);
    currentObject = controlsStack.back().node->GetControl();
}

void EditorUIPackageBuilder::EndControlPropertiesSection()
{
    currentSection = nullptr;
    currentObject = nullptr;
}

UIComponent *EditorUIPackageBuilder::BeginComponentPropertiesSection(uint32 componentType, DAVA::uint32 componentIndex)
{
    ControlNode *node = controlsStack.back().node;
    ComponentPropertiesSection * section;
    section = node->GetRootProperty()->FindComponentPropertiesSection(componentType, componentIndex);
    if (section == nullptr)
        section = node->GetRootProperty()->AddComponentPropertiesSection(componentType);
    currentObject = section->GetComponent();
    currentSection = section;
    return section->GetComponent();
}

void EditorUIPackageBuilder::EndComponentPropertiesSection()
{
    currentSection = nullptr;
    currentObject = nullptr;
}

UIControlBackground *EditorUIPackageBuilder::BeginBgPropertiesSection(int index, bool sectionHasProperties)
{
    ControlNode *node = controlsStack.back().node;
    BackgroundPropertiesSection *section = node->GetRootProperty()->GetBackgroundPropertiesSection(index);
    if (section && sectionHasProperties)
    {
        if (section->GetBg() == nullptr)
            section->CreateControlBackground();

        if (section->GetBg())
        {
            currentObject = section->GetBg();
            currentSection = section;
            return section->GetBg();
        }
    }
    
    return nullptr;
}

void EditorUIPackageBuilder::EndBgPropertiesSection()
{
    currentSection = nullptr;
    currentObject = nullptr;
}

UIControl *EditorUIPackageBuilder::BeginInternalControlSection(int index, bool sectionHasProperties)
{
    ControlNode *node = controlsStack.back().node;
    InternalControlPropertiesSection *section = node->GetRootProperty()->GetInternalControlPropertiesSection(index);
    if (section && sectionHasProperties)
    {
        if (section->GetInternalControl() == nullptr)
            section->CreateInternalControl();
        
        if (section->GetInternalControl())
        {
            currentObject = section->GetInternalControl();
            currentSection = section;
            return section->GetInternalControl();
        }
    }
    
    return nullptr;
}

void EditorUIPackageBuilder::EndInternalControlSection()
{
    currentSection = nullptr;
    currentObject = nullptr;
}

void EditorUIPackageBuilder::ProcessProperty(const InspMember *member, const VariantType &value)
{
    if (currentObject && currentSection && (member->Flags() & I_EDIT))
    {
        ValueProperty *property = currentSection->FindProperty(member);
        if (property && value.GetType() != VariantType::TYPE_NONE)
            property->SetValue(value);
    }
}

RefPtr<PackageNode> EditorUIPackageBuilder::GetPackageNode() const
{
    return DAVA::RefPtr<PackageNode>(SafeRetain(packageNode));
}

////////////////////////////////////////////////////////////////////////////////
// ControlDescr
////////////////////////////////////////////////////////////////////////////////
EditorUIPackageBuilder::ControlDescr::ControlDescr() : node(nullptr), addToParent(false)
{
}

EditorUIPackageBuilder::ControlDescr::ControlDescr(ControlNode *node, bool addToParent) : node(node), addToParent(addToParent)
{
}

EditorUIPackageBuilder::ControlDescr::ControlDescr(const ControlDescr &descr)
{
    node = DAVA::SafeRetain(descr.node);
    addToParent = descr.addToParent;
}

EditorUIPackageBuilder::ControlDescr::~ControlDescr()
{
    DAVA::SafeRelease(node);
}

EditorUIPackageBuilder::ControlDescr &EditorUIPackageBuilder::ControlDescr::operator=(const ControlDescr &descr)
{
    DAVA::SafeRetain(descr.node);
    DAVA::SafeRelease(node);
    
    node = descr.node;
    addToParent = descr.addToParent;
    return *this;
}
