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

    for (StyleSheetNode *styleSheet : styleSheets)
        styleSheet->Release();
    styleSheets.clear();
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
        if (package->GetPath().GetFrameworkPath() == packagePath.GetFrameworkPath())
            return true;
    }
    
    if (std::find(declinedPackages.begin(), declinedPackages.end(), packagePath) != declinedPackages.end())
    {
        DVASSERT(false); 
        return false;
    }
    
    EditorUIPackageBuilder builder;
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

void EditorUIPackageBuilder::ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain> &selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty> &properties)
{
    StyleSheetNode *node = new StyleSheetNode(selectorChains, properties);
    styleSheets.push_back(node);
}

UIControl *EditorUIPackageBuilder::BeginControlWithClass(const String &className)
{
    RefPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(className));
    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    controlsStack.push_back(ControlDescr(ControlNode::CreateFromControl(control.Get()), true));

    return control.Get();
}

UIControl *EditorUIPackageBuilder::BeginControlWithCustomClass(const String &customClassName, const String &className)
{
    RefPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(className));

    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    ControlNode *node = ControlNode::CreateFromControl(control.Get());
    node->GetRootProperty()->GetCustomClassProperty()->SetValue(VariantType(customClassName));
    controlsStack.push_back(ControlDescr(node, true));

    return control.Get();
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
        {
            lastControl->GetControl()->UpdateLayout();
            rootControls.push_back(SafeRetain(lastControl));
        }
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
        {
            if (property->GetStylePropertyIndex() != -1)
                controlsStack.back().node->GetControl()->SetPropertyLocalFlag(property->GetStylePropertyIndex(), true);

            property->SetValue(value);
        }
    }
}

RefPtr<PackageNode> EditorUIPackageBuilder::BuildPackage() const
{
    DVASSERT(!packagePath.IsEmpty());
    RefPtr<PackageNode> package(new PackageNode(packagePath));
    
    Vector<PackageNode*> declinedPackages;
    for (PackageNode *importedPackage : importedPackages)
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
    
    for (StyleSheetNode *styleSheet : styleSheets)
    {
        package->GetStyleSheets()->Add(styleSheet);
    }
    
    for (ControlNode *control : rootControls)
    {
        bool canInsert = true;
        for (PackageNode *declinedPackage : declinedPackages)
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

const Vector<ControlNode*> &EditorUIPackageBuilder::GetRootControls() const
{
    return rootControls;
}

const Vector<PackageNode*> &EditorUIPackageBuilder::GetImportedPackages() const
{
    return importedPackages;
}

const Vector<StyleSheetNode*> &EditorUIPackageBuilder::GetStyles() const
{
    return styleSheets;
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
