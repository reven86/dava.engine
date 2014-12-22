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
#include "FileSystem/LocalizationSystem.h"

namespace DAVA
{
    
const String DefaultUIPackageBuilder::EXCEPTION_CLASS_UI_TEXT_FIELD = "UITextField";

DefaultUIPackageBuilder::DefaultUIPackageBuilder()
: package(NULL), currentObject(NULL)
{
    
}

DefaultUIPackageBuilder::~DefaultUIPackageBuilder()
{
    for (auto it = importedPackages.begin(); it != importedPackages.end(); ++it)
        it->second->Release();
    importedPackages.clear();
    
    DVASSERT(package == NULL);
}

UIPackage *DefaultUIPackageBuilder::BeginPackage(const FilePath &packagePath)
{
    DVASSERT(package == NULL)
    package = new UIPackage(packagePath);
    return package;
}

void DefaultUIPackageBuilder::EndPackage()
{
    SafeRelease(package);
}

UIPackage *DefaultUIPackageBuilder::ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader)
{
    UIPackage *result;
    UIPackage *prevPackage = package;
    package = NULL;
    auto it = importedPackages.find(packagePath);
    if (it != importedPackages.end())
    {
        result = it->second;
        prevPackage->AddPackage(it->second);
    }
    else
    {
        UIPackage *loadedPackage = loader->LoadPackage(packagePath);
        result = loadedPackage;
        importedPackages[packagePath] = loadedPackage;
        prevPackage->AddPackage(loadedPackage);
    }
    DVASSERT(package == NULL);
    package = prevPackage;
    return result;
}

UIControl *DefaultUIPackageBuilder::BeginControlWithClass(const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    if (!control)
        Logger::Error("[DefaultUIControlFactory::CreateControl] Can't create control with class name \"%s\"", className.c_str());

    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    controlsStack.push_back(ControlDescr(control, true));
    return control;
}

UIControl *DefaultUIPackageBuilder::BeginControlWithCustomClass(const String &customClassName, const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(customClassName);
    if (className != EXCEPTION_CLASS_UI_TEXT_FIELD)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    if (!control)
        control = ObjectFactory::Instance()->New<UIControl>(className); // TODO: remove
    
    if (control)
    {
        control->SetCustomControlClassName(customClassName);
    }
    else
    {
        DVASSERT(false);
    }
    
    controlsStack.push_back(ControlDescr(control, true));
    return control;
}

UIControl *DefaultUIPackageBuilder::BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String &customClassName, AbstractUIPackageLoader *loader)
{
    UIControl *prototype;
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
        UIPackage *importedPackage = package->GetPackage(packageName);
        if (importedPackage)
            prototype = importedPackage->GetControl(prototypeName);
    }
    
    DVASSERT(prototype != NULL);
    
    UIControl *control;
    if (!customClassName.empty())
    {
        control = ObjectFactory::Instance()->New<UIControl>(customClassName);
        control->RemoveAllControls();
        
        control->CopyDataFrom(prototype);
    }
    else
        control = prototype->Clone();
    
    controlsStack.push_back(ControlDescr(control, true));
    return control;
}

UIControl *DefaultUIPackageBuilder::BeginControlWithPath(const String &pathName)
{
    UIControl *control = NULL;
    if (!controlsStack.empty())
    {
        control = controlsStack.back().control->FindByPath(pathName);
    }

    DVASSERT(control);
    controlsStack.push_back(ControlDescr(SafeRetain(control), false));
    return control;
}

UIControl *DefaultUIPackageBuilder::BeginUnknownControl(const YamlNode *node)
{
    DVASSERT(false);
    controlsStack.push_back(ControlDescr(NULL, false));
    return NULL;
}

void DefaultUIPackageBuilder::EndControl(bool isRoot)
{
    ControlDescr lastControl = controlsStack.back();
    controlsStack.pop_back();
    if (lastControl.addToParent)
    {
        if (controlsStack.empty() || isRoot)
        {
            package->AddControl(lastControl.control);
        }
        else
        {
            UIControl *control = controlsStack.back().control;
            control->AddControl(lastControl.control);
            lastControl.control->UpdateLayout();
        }
    }
}

void DefaultUIPackageBuilder::BeginControlPropertiesSection(const String &name)
{
    currentObject = controlsStack.back().control;
}

void DefaultUIPackageBuilder::EndControlPropertiesSection()
{
    currentObject = NULL;
}

UIControlBackground *DefaultUIPackageBuilder::BeginBgPropertiesSection(int32 index, bool sectionHasProperties)
{
    if (sectionHasProperties)
    {
        UIControl *control = controlsStack.back().control;
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
    return NULL;
}

void DefaultUIPackageBuilder::EndBgPropertiesSection()
{
    currentObject = NULL;
}

UIControl *DefaultUIPackageBuilder::BeginInternalControlSection(int32 index, bool sectionHasProperties)
{
    if (sectionHasProperties)
    {
        UIControl *control = controlsStack.back().control;
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
    return NULL;
}

void DefaultUIPackageBuilder::EndInternalControlSection()
{
    currentObject = NULL;
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

////////////////////////////////////////////////////////////////////////////////
// ControlDescr
////////////////////////////////////////////////////////////////////////////////

DefaultUIPackageBuilder::ControlDescr::ControlDescr() : control(NULL), addToParent(false)
{
}

DefaultUIPackageBuilder::ControlDescr::ControlDescr(UIControl *control, bool addToParent) : control(control), addToParent(addToParent)
{
}

DefaultUIPackageBuilder::ControlDescr::ControlDescr(const ControlDescr &descr)
{
    control = DAVA::SafeRetain(descr.control);
    addToParent = descr.addToParent;
}

DefaultUIPackageBuilder::ControlDescr::~ControlDescr()
{
    SafeRelease(control);
}

DefaultUIPackageBuilder::ControlDescr &DefaultUIPackageBuilder::ControlDescr::operator=(const ControlDescr &descr)
{
    if(&descr == this)
        return *this;
    
    SafeRetain(descr.control);
    SafeRelease(control);
    
    control = descr.control;
    addToParent = descr.addToParent;
    return *this;
}

}
