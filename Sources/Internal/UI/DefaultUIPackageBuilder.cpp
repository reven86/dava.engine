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

#include "UIPackage.h"
#include "UIPackageLoader.h"
#include "Base/ObjectFactory.h"
#include "UI/UIControl.h"
#include "UI/UIControlHelpers.h"
#include "UI/Components/UIComponent.h"
#include "FileSystem/LocalizationSystem.h"
#include "UIPackagesCache.h"

namespace DAVA
{
    
const String EXCEPTION_CLASS_UI_TEXT_FIELD = "UITextField";
const String EXCEPTION_CLASS_UI_LIST = "UIList";

////////////////////////////////////////////////////////////////////////////////
// PackageDescr
////////////////////////////////////////////////////////////////////////////////
    
class DefaultUIPackageBuilder::PackageDescr
{
public:
    PackageDescr(UIPackage *_package) : package(SafeRetain(_package))
    {
        
    }
    
    ~PackageDescr()
    {
        SafeRelease(package);
        
        for (UIPackage *pack : importedPackages)
            pack->Release();
        importedPackages.clear();
    }
    
    UIPackage *GetPackage() const
    {
        return package;
    }

    void PutImportredPackage(const FilePath &path, UIPackage *package)
    {
        int32 index = (int32) importedPackages.size();
        importedPackages.push_back(SafeRetain(package));
        packsByPaths[path] = index;
        packsByNames[path.GetBasename()] = index;
    }
    
    UIPackage *FindImportedPackageByName(const String &name) const
    {
        auto it = packsByNames.find(name);
        if (it != packsByNames.end())
            return importedPackages[it->second];
        
        return nullptr;
    }
    
private:
    UIPackage *package;
    Vector<UIPackage*> importedPackages;
    Map<FilePath, int32> packsByPaths;
    Map<String, int32> packsByNames;
};

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
    
    if (!packagesStack.empty())
    {
        for (auto &descr : packagesStack)
            SafeDelete(descr);
        
        packagesStack.clear();
        
        DVASSERT(false);
    }

    if (!controlsStack.empty())
    {
        for (auto &descr : controlsStack)
            SafeDelete(descr);

        controlsStack.clear();
        
        DVASSERT(false);
    }
}

UIPackage *DefaultUIPackageBuilder::FindInCache(const String &packagePath) const
{
    return cache->GetPackage(packagePath);
}

RefPtr<UIPackage> DefaultUIPackageBuilder::BeginPackage(const FilePath &packagePath)
{
    RefPtr<UIPackage> package(new UIPackage());
    
    packagesStack.push_back(new PackageDescr(package.Get()));
    
    return package;
}

void DefaultUIPackageBuilder::EndPackage()
{
    if (!packagesStack.empty())
    {
        SafeDelete(packagesStack.back());
        packagesStack.pop_back();
        
        if (!controlsStack.empty())
        {
            for (auto &descr : controlsStack)
                SafeDelete(descr);
            
            controlsStack.clear();
            
            DVASSERT(false);
        }
        
    }
    else
    {
        DVASSERT(false);
    }
}

RefPtr<UIPackage> DefaultUIPackageBuilder::ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader)
{
    UIPackage *cachedPackage = cache->GetPackage(packagePath);

    PackageDescr *descr = packagesStack.back();

    if (cachedPackage)
    {
        descr->PutImportredPackage(packagePath, cachedPackage);
        return RefPtr<UIPackage>(SafeRetain(cachedPackage));
    }
    else
    {
        RefPtr<UIPackage> res(loader->LoadPackage(packagePath));

        cache->PutPackage(packagePath, res.Get());
        descr->PutImportredPackage(packagePath, res.Get());
        
        return res;
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
    
    if (control.Valid())
    {
        control->SetCustomControlClassName(customClassName);
    }
    else
    {
        DVASSERT(false);
    }
    
    controlsStack.push_back(new ControlDescr(control.Get(), true));
    return control.Get();
}

UIControl *DefaultUIPackageBuilder::BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String &customClassName, AbstractUIPackageLoader *loader)
{
    UIControl *prototype = nullptr;
    UIPackage *package = packagesStack.back()->GetPackage();
    
    if (packageName.empty())
    {
        prototype = package->GetControl(prototypeName);
        if (!prototype)
        {
            if (loader->LoadControlByName(prototypeName))
                prototype = package->GetControl(prototypeName);
        }
    }
    else
    {
        UIPackage *importedPackage = packagesStack.back()->FindImportedPackageByName(packageName);
        if (importedPackage)
            prototype = importedPackage->GetControl(prototypeName);
    }
    
    DVASSERT(prototype != nullptr);
    
    RefPtr<UIControl> control;
    if (!customClassName.empty())
    {
        control.Set(ObjectFactory::Instance()->New<UIControl>(customClassName));
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
            packagesStack.back()->GetPackage()->AddControl(lastDescr->control.Get());
        }
        else
        {
            UIControl *control = controlsStack.back()->control.Get();
            control->AddControl(lastDescr->control.Get());
            lastDescr->control->UpdateLayout();
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
    
UIComponent *DefaultUIPackageBuilder::BeginComponentPropertiesSecion(uint32 componentType)
{
    UIComponent *component = UIComponent::CreateByType(componentType);
    UIControl *control = controlsStack.back()->control.Get();
    control->PutComponent(component);
    component->Release();
    currentObject = component;
    return component;
}
    
void DefaultUIPackageBuilder::EndComponentPropertiesSecion()
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
        if (String(member->Name()) == "text")
            member->SetValue(currentObject, VariantType(LocalizedString(value.AsWideString())));
        else
            member->SetValue(currentObject, value);
    }
}

}
