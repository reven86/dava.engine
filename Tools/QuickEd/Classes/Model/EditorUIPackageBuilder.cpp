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
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageCommandExecutor.h"
#include "UI/UIPackage.h"

using namespace DAVA;

const String EXCEPTION_CLASS_UI_TEXT_FIELD = "UITextField";
const String EXCEPTION_CLASS_UI_LIST = "UIList";

EditorUIPackageBuilder::EditorUIPackageBuilder()
    : currentObject(nullptr)
    , currentSection(nullptr)
{
}

EditorUIPackageBuilder::~EditorUIPackageBuilder()
{
    for (PackageNode *importedPackage : importedPackages)
        importedPackage->Release();
    importedPackages.clear();
    
    for (ControlNode *control : rootControls)
        control->Release();
    rootControls.clear();
}

void EditorUIPackageBuilder::BeginPackage(const FilePath &aPackagePath)
{
    DVASSERT(packagePath.IsEmpty());
    packagePath = aPackagePath;
}

void EditorUIPackageBuilder::EndPackage()
{
}

bool EditorUIPackageBuilder::ProcessImportedPackage(const String &packagePathStr, AbstractUIPackageLoader *loader)
{
    FilePath packagePath(packagePathStr);
    for (PackageNode *package : importedPackages)
    {
        if (package->GetPath() == packagePath)
            return true;
    }
    
    EditorUIPackageBuilder builder;
    if (loader->LoadPackage(packagePath, &builder))
    {
        RefPtr<PackageNode> importedPackage = builder.BuildPackage();
        importedPackages.push_back(SafeRetain(importedPackage.Get()));
        return true;
    }
    
    return false;
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
    ControlNode *prototypeNode = nullptr;
    if (packageName.empty())
    {
        prototypeNode = FindRootControl(prototypeName);
        if (prototypeNode == nullptr)
        {
            if (loader->LoadControlByName(prototypeName, this))
                prototypeNode = FindRootControl(prototypeName);
        }
    }
    else
    {
        for (PackageNode *importedPackage : importedPackages)
        {
            if (importedPackage->GetName() == packageName)
            {
                prototypeNode = importedPackage->GetPackageControlsNode()->FindControlNodeByName(prototypeName);
                break;
            }
        }
    }
    
    DVASSERT(prototypeNode);
    ControlNode *node = ControlNode::CreateFromPrototype(prototypeNode);
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
            rootControls.push_back(SafeRetain(lastControl));
        else
            controlsStack.back().node->Add(lastControl);
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

RefPtr<PackageNode> EditorUIPackageBuilder::BuildPackage() const
{
    DVASSERT(!packagePath.IsEmpty());
    RefPtr<PackageNode> package(new PackageNode(packagePath));
    
    for (PackageNode *importedPackage : importedPackages)
    {
        package->GetImportedPackagesNode()->Add(importedPackage);
    }
    
    for (ControlNode *control : rootControls)
    {
        package->GetPackageControlsNode()->Add(control);
    }
    
    return package;
}

const Vector<ControlNode*> &EditorUIPackageBuilder::GetRootControls() const
{
    return rootControls;
}

const Vector<PackageNode*> &EditorUIPackageBuilder::GetImportedPackages() const
{
    return importedPackages;
}

void EditorUIPackageBuilder::AddImportedPackage(PackageNode *node)
{
    importedPackages.push_back(SafeRetain(node));
}

ControlNode *EditorUIPackageBuilder::FindRootControl(const DAVA::String &name) const
{
    for (ControlNode *control : rootControls)
    {
        if (control->GetName() == name)
            return control;
    }
    return nullptr;
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
