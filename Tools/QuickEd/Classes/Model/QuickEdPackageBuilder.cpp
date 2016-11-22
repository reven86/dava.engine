#include "QuickEdPackageBuilder.h"

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
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
#include "UI/UIPackage.h"
#include "UI/UIControl.h"
#include "UI/UIControlPackageContext.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/Styles/UIStyleSheetYamlLoader.h"
#include "Base/ObjectFactory.h"
#include "Utils/Utils.h"

using namespace DAVA;

const String EXCEPTION_CLASS_UI_TEXT_FIELD = "UITextField";
const String EXCEPTION_CLASS_UI_LIST = "UIList";

QuickEdPackageBuilder::QuickEdPackageBuilder()
    : currentObject(nullptr)
    , currentSection(nullptr)
{
}

QuickEdPackageBuilder::~QuickEdPackageBuilder()
{
    for (PackageNode* importedPackage : importedPackages)
        importedPackage->Release();
    importedPackages.clear();

    for (ControlNode* control : rootControls)
        control->Release();
    rootControls.clear();

    for (StyleSheetNode* styleSheet : styleSheets)
        styleSheet->Release();
    styleSheets.clear();
}

void QuickEdPackageBuilder::BeginPackage(const FilePath& aPackagePath)
{
    DVASSERT(packagePath.IsEmpty());
    packagePath = aPackagePath;
}

void QuickEdPackageBuilder::EndPackage()
{
}

bool QuickEdPackageBuilder::ProcessImportedPackage(const String& packagePathStr, AbstractUIPackageLoader* loader)
{
    FilePath packagePath(packagePathStr);
    for (PackageNode* package : importedPackages)
    {
        if (package->GetPath().GetFrameworkPath() == packagePath.GetFrameworkPath())
            return true;
    }

    if (std::find(declinedPackages.begin(), declinedPackages.end(), packagePath) != declinedPackages.end())
    {
        DVASSERT(false);
        return false;
    }

    QuickEdPackageBuilder builder;
    builder.declinedPackages.insert(builder.declinedPackages.end(), declinedPackages.begin(), declinedPackages.end());
    builder.declinedPackages.push_back(packagePath);

    if (loader->LoadPackage(packagePath, &builder))
    {
        RefPtr<PackageNode> importedPackage = builder.BuildPackage();
        importedPackages.push_back(SafeRetain(importedPackage.Get()));
        return true;
    }

    return false;
}

void QuickEdPackageBuilder::ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain>& selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty>& properties)
{
    StyleSheetNode* node = new StyleSheetNode(selectorChains, properties);
    styleSheets.push_back(node);
}

UIControl* QuickEdPackageBuilder::BeginControlWithClass(const String& className)
{
    RefPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(className));
    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST) //TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    controlsStack.push_back(ControlDescr(ControlNode::CreateFromControl(control.Get()), true));

    return control.Get();
}

UIControl* QuickEdPackageBuilder::BeginControlWithCustomClass(const String& customClassName, const String& className)
{
    RefPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(className));

    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST) //TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    ControlNode* node = ControlNode::CreateFromControl(control.Get());
    node->GetRootProperty()->GetCustomClassProperty()->SetValue(VariantType(customClassName));
    controlsStack.push_back(ControlDescr(node, true));

    return control.Get();
}

UIControl* QuickEdPackageBuilder::BeginControlWithPrototype(const String& packageName, const String& prototypeName, const String* customClassName, AbstractUIPackageLoader* loader)
{
    ControlNode* prototypeNode = nullptr;
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
        for (PackageNode* importedPackage : importedPackages)
        {
            if (importedPackage->GetName() == packageName)
            {
                prototypeNode = importedPackage->GetPackageControlsNode()->FindControlNodeByName(prototypeName);
                break;
            }
        }
    }

    DVASSERT(prototypeNode);
    ControlNode* node = ControlNode::CreateFromPrototype(prototypeNode);
    if (customClassName)
        node->GetRootProperty()->GetCustomClassProperty()->SetValue(VariantType(*customClassName));
    controlsStack.push_back(ControlDescr(node, true));

    return node->GetControl();
}

UIControl* QuickEdPackageBuilder::BeginControlWithPath(const String& pathName)
{
    ControlNode* control = nullptr;
    if (!controlsStack.empty())
    {
        control = controlsStack.back().node;
        Vector<String> controlNames;
        Split(pathName, "/", controlNames, false, true);
        for (Vector<String>::const_iterator iter = controlNames.begin(); iter != controlNames.end(); ++iter)
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

UIControl* QuickEdPackageBuilder::BeginUnknownControl(const YamlNode* node)
{
    DVASSERT(false);
    return nullptr;
}

void QuickEdPackageBuilder::EndControl(bool isRoot)
{
    ControlNode* lastControl = SafeRetain(controlsStack.back().node);
    bool addToParent = controlsStack.back().addToParent;
    controlsStack.pop_back();

    if (addToParent)
    {
        if (controlsStack.empty() || isRoot)
        {
            lastControl->GetControl()->UpdateLayout();
            rootControls.push_back(SafeRetain(lastControl));
        }
        else
            controlsStack.back().node->Add(lastControl);
    }
    SafeRelease(lastControl);
}

void QuickEdPackageBuilder::BeginControlPropertiesSection(const String& name)
{
    currentSection = controlsStack.back().node->GetRootProperty()->GetControlPropertiesSection(name);
    currentObject = controlsStack.back().node->GetControl();
}

void QuickEdPackageBuilder::EndControlPropertiesSection()
{
    currentSection = nullptr;
    currentObject = nullptr;
}

UIComponent* QuickEdPackageBuilder::BeginComponentPropertiesSection(uint32 componentType, DAVA::uint32 componentIndex)
{
    ControlNode* node = controlsStack.back().node;
    ComponentPropertiesSection* section;
    section = node->GetRootProperty()->FindComponentPropertiesSection(componentType, componentIndex);
    if (section == nullptr)
        section = node->GetRootProperty()->AddComponentPropertiesSection(componentType);
    currentObject = section->GetComponent();
    currentSection = section;
    return section->GetComponent();
}

void QuickEdPackageBuilder::EndComponentPropertiesSection()
{
    currentSection = nullptr;
    currentObject = nullptr;
}

UIControlBackground* QuickEdPackageBuilder::BeginBgPropertiesSection(int index, bool sectionHasProperties)
{
    ControlNode* node = controlsStack.back().node;
    BackgroundPropertiesSection* section = node->GetRootProperty()->GetBackgroundPropertiesSection(index);
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

void QuickEdPackageBuilder::EndBgPropertiesSection()
{
    currentSection = nullptr;
    currentObject = nullptr;
}

UIControl* QuickEdPackageBuilder::BeginInternalControlSection(int index, bool sectionHasProperties)
{
    ControlNode* node = controlsStack.back().node;
    InternalControlPropertiesSection* section = node->GetRootProperty()->GetInternalControlPropertiesSection(index);
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

void QuickEdPackageBuilder::EndInternalControlSection()
{
    currentSection = nullptr;
    currentObject = nullptr;
}

void QuickEdPackageBuilder::ProcessProperty(const InspMember* member, const VariantType& value)
{
    if (currentObject && currentSection && (member->Flags() & I_EDIT))
    {
        ValueProperty* property = currentSection->FindProperty(member);
        if (property && value.GetType() != VariantType::TYPE_NONE)
        {
            if (property->GetStylePropertyIndex() != -1)
                controlsStack.back().node->GetControl()->SetPropertyLocalFlag(property->GetStylePropertyIndex(), true);

            property->SetValue(value);
        }
    }
}

RefPtr<PackageNode> QuickEdPackageBuilder::BuildPackage() const
{
    DVASSERT(!packagePath.IsEmpty());
    RefPtr<PackageNode> package(new PackageNode(packagePath));

    Vector<PackageNode*> declinedPackages;
    for (PackageNode* importedPackage : importedPackages)
    {
        if (package->GetImportedPackagesNode()->CanInsertImportedPackage(importedPackage))
        {
            package->GetImportedPackagesNode()->Add(importedPackage);
        }
        else
        {
            declinedPackages.push_back(importedPackage);
        }
    }

    for (StyleSheetNode* styleSheet : styleSheets)
    {
        package->GetStyleSheets()->Add(styleSheet);
    }

    for (ControlNode* control : rootControls)
    {
        bool canInsert = true;
        for (PackageNode* declinedPackage : declinedPackages)
        {
            if (control->IsDependsOnPackage(declinedPackage))
                canInsert = false;
        }

        if (canInsert)
        {
            package->GetPackageControlsNode()->Add(control);
        }
    }

    package->RefreshPackageStylesAndLayout();

    DVASSERT(declinedPackages.empty());

    return package;
}

const Vector<ControlNode*>& QuickEdPackageBuilder::GetRootControls() const
{
    return rootControls;
}

const Vector<PackageNode*>& QuickEdPackageBuilder::GetImportedPackages() const
{
    return importedPackages;
}

const Vector<StyleSheetNode*>& QuickEdPackageBuilder::GetStyles() const
{
    return styleSheets;
}

void QuickEdPackageBuilder::AddImportedPackage(PackageNode* node)
{
    importedPackages.push_back(SafeRetain(node));
}

ControlNode* QuickEdPackageBuilder::FindRootControl(const DAVA::String& name) const
{
    for (ControlNode* control : rootControls)
    {
        if (control->GetName() == name)
            return control;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// ControlDescr
////////////////////////////////////////////////////////////////////////////////
QuickEdPackageBuilder::ControlDescr::ControlDescr()
    : node(nullptr)
    , addToParent(false)
{
}

QuickEdPackageBuilder::ControlDescr::ControlDescr(ControlNode* node, bool addToParent)
    : node(node)
    , addToParent(addToParent)
{
}

QuickEdPackageBuilder::ControlDescr::ControlDescr(const ControlDescr& descr)
{
    node = DAVA::SafeRetain(descr.node);
    addToParent = descr.addToParent;
}

QuickEdPackageBuilder::ControlDescr::~ControlDescr()
{
    DAVA::SafeRelease(node);
}

QuickEdPackageBuilder::ControlDescr& QuickEdPackageBuilder::ControlDescr::operator=(const ControlDescr& descr)
{
    DAVA::SafeRetain(descr.node);
    DAVA::SafeRelease(node);

    node = descr.node;
    addToParent = descr.addToParent;
    return *this;
}
