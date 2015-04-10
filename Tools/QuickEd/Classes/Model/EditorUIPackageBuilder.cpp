#include "EditorUIPackageBuilder.h"

#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/ControlPropertiesSection.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "Model/ControlProperties/BackgroundPropertiesSection.h"
#include "Model/ControlProperties/InternalControlPropertiesSection.h"
#include "Model/ControlProperties/ValueProperty.h"
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

UIPackage * EditorUIPackageBuilder::FindInCache(const String &packagePath) const
{
    return nullptr;
}

RefPtr<UIPackage> EditorUIPackageBuilder::BeginPackage(const FilePath &packagePath)
{
    DVASSERT(packageNode == nullptr);
    SafeRelease(packageNode);
    
    if (basePackage) // if there is exists base package we skip creating new one
    {
        RefPtr<UIPackage> package(SafeRetain(basePackage->GetPackageRef()->GetPackage()));
        packageNode = SafeRetain(basePackage);
        return package;
    }
    else
    {
        RefPtr<UIPackage> package(new UIPackage());
        ScopedPtr<PackageRef> ref(new PackageRef(packagePath, package.Get()));
        packageNode = new PackageNode(ref);
        return package;
    }
}

void EditorUIPackageBuilder::EndPackage()
{
    DVASSERT(packageNode != nullptr);
}

RefPtr<UIPackage> EditorUIPackageBuilder::ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader)
{
    PackageControlsNode *importedPackage = packageNode->FindImportedPackage(packagePath);
    if (importedPackage == nullptr)
    {
        // store state
        PackageNode *prevPackageNode = packageNode;
        PackageNode *prevBasePackage = basePackage;
        ControlsContainerNode *prevInsertingTarget = insertingTarget;
        int32 prevInsertingIndex = insertingIndex;
        DAVA::List<ControlDescr> prevControlsStack = controlsStack;
        
        DAVA::BaseObject *prevObj = currentObject;
        PropertiesSection *prevSect = currentSection;

        // clear state
        packageNode = nullptr;
        insertingTarget = nullptr;
        insertingIndex = -1;
        basePackage = nullptr;
        controlsStack.clear();
        currentObject = nullptr;
        currentSection = nullptr;
        
        // load package
        RefPtr<UIPackage> result(loader->LoadPackage(packagePath));
        PackageControlsNode *controlsNode = SafeRetain(packageNode->GetPackageControlsNode());
        controlsNode->SetName(packageNode->GetName());
        SafeRelease(packageNode);
        
        if (prevBasePackage)
            commandExecutor->AddImportedPackageIntoPackage(controlsNode, prevPackageNode);
        else
            prevPackageNode->GetImportedPackagesNode()->Add(controlsNode);
        SafeRelease(controlsNode);

        // restore state
        packageNode = prevPackageNode;
        basePackage = prevBasePackage;
        insertingTarget = prevInsertingTarget;
        insertingIndex = prevInsertingIndex;
        controlsStack = prevControlsStack;
        currentObject = prevObj;
        currentSection = prevSect;

        return result;
    }
    return RefPtr<UIPackage>(SafeRetain(importedPackage->GetPackageRef()->GetPackage()));
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
    control->SetCustomControlClassName(customClassName);
    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    controlsStack.push_back(ControlDescr(ControlNode::CreateFromControl(control), true));
    return control;
}

UIControl *EditorUIPackageBuilder::BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String &customClassName, AbstractUIPackageLoader *loader)
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
            if (loader->LoadControlByName(prototypeName))
                prototypeNode = controlsNode->FindControlNodeByName(prototypeName);
        }
        
    }
    
    DVASSERT(prototypeNode);
    ControlNode *node = ControlNode::CreateFromPrototype(prototypeNode, controlsNode->GetPackageRef());
    node->GetControl()->SetCustomControlClassName(customClassName);
    controlsStack.push_back(ControlDescr(node, true));

    return node->GetControl();
}

UIControl *EditorUIPackageBuilder::BeginControlWithPath(const String &pathName)
{
    ControlNode *control = NULL;
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
        return NULL;

    return control->GetControl();
}

UIControl *EditorUIPackageBuilder::BeginUnknownControl(const YamlNode *node)
{
    DVASSERT(false);
    return NULL;
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
    currentSection = controlsStack.back().node->GetPropertiesRoot()->GetControlPropertiesSection(name);
    currentObject = controlsStack.back().node->GetControl();
}

void EditorUIPackageBuilder::EndControlPropertiesSection()
{
    currentSection = NULL;
    currentObject = NULL;
}

UIComponent *EditorUIPackageBuilder::BeginComponentPropertiesSecion(uint32 componentType)
{
    ControlNode *node = controlsStack.back().node;
    ComponentPropertiesSection * section = node->GetPropertiesRoot()->AddComponentPropertiesSection(componentType);
    currentObject = section->GetComponent();
    currentSection = section;
    return section->GetComponent();
}

void EditorUIPackageBuilder::EndComponentPropertiesSecion()
{
    currentSection = NULL;
    currentObject = NULL;
}

UIControlBackground *EditorUIPackageBuilder::BeginBgPropertiesSection(int index, bool sectionHasProperties)
{
    ControlNode *node = controlsStack.back().node;
    BackgroundPropertiesSection *section = node->GetPropertiesRoot()->GetBackgroundPropertiesSection(index);
    if (section && sectionHasProperties)
    {
        if (section->GetBg() == NULL)
            section->CreateControlBackground();

        if (section->GetBg())
        {
            currentObject = section->GetBg();
            currentSection = section;
            return section->GetBg();
        }
    }
    
    return NULL;
}

void EditorUIPackageBuilder::EndBgPropertiesSection()
{
    currentSection = NULL;
    currentObject = NULL;
}

UIControl *EditorUIPackageBuilder::BeginInternalControlSection(int index, bool sectionHasProperties)
{
    ControlNode *node = controlsStack.back().node;
    InternalControlPropertiesSection *section = node->GetPropertiesRoot()->GetInternalControlPropertiesSection(index);
    if (section && sectionHasProperties)
    {
        if (section->GetInternalControl() == NULL)
            section->CreateInternalControl();
        
        if (section->GetInternalControl())
        {
            currentObject = section->GetInternalControl();
            currentSection = section;
            return section->GetInternalControl();
        }
    }
    
    return NULL;
}

void EditorUIPackageBuilder::EndInternalControlSection()
{
    currentSection = NULL;
    currentObject = NULL;
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
EditorUIPackageBuilder::ControlDescr::ControlDescr() : node(NULL), addToParent(false)
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
