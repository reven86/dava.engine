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


#include "DefaultUIPackageBuilder.h"

#include "UI/UIPackage.h"
#include "UI/UIPackageLoader.h"
#include "UI/UIControlSystem.h"
#include "UI/Layouts/UILayoutSystem.h"

#include "Base/ObjectFactory.h"
#include "UI/UIControl.h"
#include "UI/UIControlHelpers.h"
#include "UI/Components/UIComponent.h"
#include "FileSystem/LocalizationSystem.h"
#include "UI/UIPackagesCache.h"
#include "UI/Styles/UIStyleSheet.h"

namespace DAVA
{

namespace
{
const String EXCEPTION_CLASS_UI_TEXT_FIELD = "UITextField";
const String EXCEPTION_CLASS_UI_LIST = "UIList";
const FastName PROPERTY_NAME_TEXT("text");
}

////////////////////////////////////////////////////////////////////////////////
// ControlDescr
////////////////////////////////////////////////////////////////////////////////

struct DefaultUIPackageBuilder::ControlDescr
{
    RefPtr<UIControl> control;
    bool addToParent;

    ControlDescr(UIControl *aControl, bool aAddToParent) : control(SafeRetain(aControl)), addToParent(aAddToParent)
    {
    }
};


DefaultUIPackageBuilder::DefaultUIPackageBuilder(UIPackagesCache *_cache)
    : currentObject(nullptr)
{
    if (_cache)
        cache = SafeRetain(_cache);
    else
        cache = new UIPackagesCache();
    
}

DefaultUIPackageBuilder::~DefaultUIPackageBuilder()
{
    SafeRelease(cache);
    
    if (!controlsStack.empty())
    {
        for (auto &descr : controlsStack)
            SafeDelete(descr);

        controlsStack.clear();
        
        DVASSERT(false);
    }
    
    for (UIPackage *importedPackage : importedPackages)
        SafeRelease(importedPackage);
    importedPackages.clear();
}

UIPackage *DefaultUIPackageBuilder::GetPackage() const
{
    return package.Get();
}

UIPackage *DefaultUIPackageBuilder::FindInCache(const String &packagePath) const
{
    return cache->GetPackage(packagePath);
}

void DefaultUIPackageBuilder::BeginPackage(const FilePath &packagePath)
{
    DVASSERT(!package.Valid());
    package = RefPtr<UIPackage>(new UIPackage());
}

void DefaultUIPackageBuilder::EndPackage()
{
    for (UIPackage* importedPackage : importedPackages)
    {
        const Vector<UIPriorityStyleSheet>& packageStyleSheets = importedPackage->GetControlPackageContext()->GetSortedStyleSheets();
        for (const UIPriorityStyleSheet& packageStyleSheet : packageStyleSheets)
        {
            styleSheets.emplace_back(UIPriorityStyleSheet(packageStyleSheet.GetStyleSheet(), packageStyleSheet.GetPriority() + 1));
        }
    }

    for (const UIPriorityStyleSheet &styleSheet : styleSheets)
    {
        package->GetControlPackageContext()->AddStyleSheet(styleSheet);
    }

    styleSheets.clear();
}

bool DefaultUIPackageBuilder::ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader)
{
    UIPackage *importedPackage = cache->GetPackage(packagePath);

    if (!importedPackage)
    {
        DefaultUIPackageBuilder builder(cache);
        if (loader->LoadPackage(packagePath, &builder) && builder.GetPackage())
        {
            importedPackage = builder.GetPackage();
            cache->PutPackage(packagePath, importedPackage);
        }
    }

    if (importedPackage)
    {
        PutImportredPackage(packagePath, importedPackage);
        return true;
    }
    else
    {
        DVASSERT(false);
        return false;
    }
}

void DefaultUIPackageBuilder::ProcessStyleSheet(const Vector<UIStyleSheetSelectorChain> &selectorChains, const Vector<UIStyleSheetProperty> &properties)
{
    for (const UIStyleSheetSelectorChain &chain : selectorChains)
    {
        ScopedPtr<UIStyleSheet> styleSheet(new UIStyleSheet());
        styleSheet->SetSelectorChain(chain);
        ScopedPtr<UIStyleSheetPropertyTable> propertiesTable(new UIStyleSheetPropertyTable());
        propertiesTable->SetProperties(properties);
        styleSheet->SetPropertyTable(propertiesTable);

        package->GetControlPackageContext()->AddStyleSheet(UIPriorityStyleSheet(styleSheet));
    }
}

UIControl *DefaultUIPackageBuilder::BeginControlWithClass(const String &className)
{
    RefPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(className));

    if (!control.Valid())
        Logger::Error("[DefaultUIControlFactory::CreateControl] Can't create control with class name \"%s\"", className.c_str());

    if (control.Valid() && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    controlsStack.push_back(new ControlDescr(control.Get(), true));
    return control.Get();
}

UIControl *DefaultUIPackageBuilder::BeginControlWithCustomClass(const String &customClassName, const String &className)
{
    RefPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(customClassName));

    if (!control.Valid())
        control.Set(ObjectFactory::Instance()->New<UIControl>(className)); // TODO: remove

    if (control.Valid() && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }
    
    DVASSERT(control.Valid());
    
    controlsStack.push_back(new ControlDescr(control.Get(), true));
    return control.Get();
}

UIControl *DefaultUIPackageBuilder::BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String *customClassName, AbstractUIPackageLoader *loader)
{
    UIControl *prototype = nullptr;
    
    if (packageName.empty())
    {
        prototype = package->GetControl(prototypeName);
        if (!prototype)
        {
            if (loader->LoadControlByName(prototypeName, this))
                prototype = package->GetControl(prototypeName);
        }
    }
    else
    {
        UIPackage *importedPackage = FindImportedPackageByName(packageName);
        if (importedPackage)
            prototype = importedPackage->GetControl(prototypeName);
    }
    
    DVASSERT(prototype != nullptr);
    
    RefPtr<UIControl> control;
    if (customClassName)
    {
        control.Set(ObjectFactory::Instance()->New<UIControl>(*customClassName));
        control->RemoveAllControls();
        
        control->CopyDataFrom(prototype);
    }
    else
    {
        control.Set(prototype->Clone());
    }
    
    controlsStack.push_back(new ControlDescr(control.Get(), true));
    return control.Get();
}

UIControl *DefaultUIPackageBuilder::BeginControlWithPath(const String &pathName)
{
    UIControl *control = nullptr;
    if (!controlsStack.empty())
    {
        control = controlsStack.back()->control->FindByPath(pathName);
    }

    DVASSERT(control);
    controlsStack.push_back(new ControlDescr(control, false));
    return control;
}

UIControl *DefaultUIPackageBuilder::BeginUnknownControl(const YamlNode *node)
{
    DVASSERT(false);
    controlsStack.push_back(new ControlDescr(nullptr, false));
    return nullptr;
}

void DefaultUIPackageBuilder::EndControl(bool isRoot)
{
    ControlDescr *lastDescr = controlsStack.back();
    controlsStack.pop_back();
    if (lastDescr->addToParent)
    {
        if (controlsStack.empty() || isRoot)
        {
            UIControl *control = lastDescr->control.Get();
            UIControlSystem::Instance()->GetLayoutSystem()->ApplyLayout(control);
            package->AddControl(control);
        }
        else
        {
            UIControl *control = controlsStack.back()->control.Get();
            control->AddControl(lastDescr->control.Get());
        }
    }
    SafeDelete(lastDescr);
}

void DefaultUIPackageBuilder::BeginControlPropertiesSection(const String &name)
{
    currentObject = controlsStack.back()->control.Get();
}

void DefaultUIPackageBuilder::EndControlPropertiesSection()
{
    currentObject = nullptr;
}
    
UIComponent *DefaultUIPackageBuilder::BeginComponentPropertiesSection(uint32 componentType, uint32 componentIndex)
{
    UIControl *control = controlsStack.back()->control.Get();
    UIComponent *component = control->GetComponent(componentType, componentIndex);
    if (component == nullptr)
    {
        component = UIComponent::CreateByType(componentType);
        control->AddComponent(component);
        component->Release();
    }
    currentObject = component;
    return component;
}
    
void DefaultUIPackageBuilder::EndComponentPropertiesSection()
{
    currentObject = nullptr;
}
    
UIControlBackground *DefaultUIPackageBuilder::BeginBgPropertiesSection(int32 index, bool sectionHasProperties)
{
    if (sectionHasProperties)
    {
        UIControl *control = controlsStack.back()->control.Get();
        if (!control->GetBackgroundComponent(index))
        {
            UIControlBackground *bg = control->CreateBackgroundComponent(index);
            control->SetBackgroundComponent(index, bg);
            SafeRelease(bg);
        }
        UIControlBackground *res = control->GetBackgroundComponent(index);
        currentObject = res;
        return res;
    }
    return nullptr;
}

void DefaultUIPackageBuilder::EndBgPropertiesSection()
{
    currentObject = nullptr;
}

UIControl *DefaultUIPackageBuilder::BeginInternalControlSection(int32 index, bool sectionHasProperties)
{
    if (sectionHasProperties)
    {
        UIControl *control = controlsStack.back()->control.Get();
        if (!control->GetInternalControl(index))
        {
            UIControl *internal = control->CreateInternalControl(index);
            control->SetInternalControl(index, internal);
            SafeRelease(internal);
        }
        UIControl *res = control->GetInternalControl(index);
        currentObject = res;
        return res;
    }
    return nullptr;
}

void DefaultUIPackageBuilder::EndInternalControlSection()
{
    currentObject = nullptr;
}

void DefaultUIPackageBuilder::ProcessProperty(const InspMember *member, const VariantType &value)
{
    DVASSERT(currentObject);
    
    if (currentObject && value.GetType() != VariantType::TYPE_NONE)
    {
        if (UIStyleSheetPropertyDataBase::Instance()->IsValidStyleSheetProperty(member->Name()))
        {
            UIControl *control = controlsStack.back()->control.Get();
            control->SetPropertyLocalFlag(UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetPropertyIndex(member->Name()), true);
        }

        if (member->Name() == PROPERTY_NAME_TEXT)
            member->SetValue(currentObject, VariantType(LocalizedString(value.AsWideString())));
        else
            member->SetValue(currentObject, value);
    }
}

void DefaultUIPackageBuilder::PutImportredPackage(const FilePath &path, UIPackage *package)
{
    int32 index = (int32) importedPackages.size();
    importedPackages.push_back(SafeRetain(package));
    packsByPaths[path] = index;
    packsByNames[path.GetBasename()] = index;
}

UIPackage *DefaultUIPackageBuilder::FindImportedPackageByName(const String &name) const
{
    auto it = packsByNames.find(name);
    if (it != packsByNames.end())
        return importedPackages[it->second];
    
    return nullptr;
}
    
}
