#include "DefaultUIPackageBuilder.h"

#include "UIPackage.h"
#include "UIPackageLoader.h"
#include "Base/ObjectFactory.h"
#include "UI/UIControl.h"
#include "UI/UIControlHelpers.h"
#include "FileSystem/LocalizationSystem.h"

namespace DAVA
{
    DefaultUIPackageBuilder::DefaultUIPackageBuilder()
    : package(NULL), currentObject(NULL)
    {
        
    }
    
    DefaultUIPackageBuilder::~DefaultUIPackageBuilder()
    {
        for (auto it = importedPackages.begin(); it != importedPackages.end(); ++it)
            it->second->Release();
        importedPackages.clear();
    }
 
    UIPackage *DefaultUIPackageBuilder::BeginPackage(const FilePath &packagePath)
    {
        DVASSERT(package == NULL)
        package = new UIPackage(packagePath);
        return package;
    }
    
    void DefaultUIPackageBuilder::EndPackage()
    {
        package = NULL;
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
            Logger::Warning("[DefaultUIControlFactory::CreateControl] Can't create control with class name \"%s\"", className.c_str());

        if (control && className != "UITextField")//TODO: fix internal staticText for Win\Mac
        {
            control->RemoveAllControls();
        }

        controlsStack.push_back(ControlDescr(control, true));
        return control;
    }
    
    UIControl *DefaultUIPackageBuilder::BeginControlWithCustomClass(const String &customClassName, const String &className)
    {
        UIControl *control = ObjectFactory::Instance()->New<UIControl>(customClassName);
        if (className != "UITextField")//TODO: fix internal staticText for Win\Mac
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
            DVASSERT(control != NULL);
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
    
    void DefaultUIPackageBuilder::EndControl()
    {
        ControlDescr lastControl = controlsStack.back();
        controlsStack.pop_back();
        if (lastControl.addToParent)
        {
            if (controlsStack.empty())
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
    
    void DefaultUIPackageBuilder::BeginControlPropretiesSection(const String &name)
    {
        currentObject = controlsStack.back().control;
    }
    
    void DefaultUIPackageBuilder::EndControlPropertiesSection()
    {
        currentObject = NULL;
    }
    
    UIControlBackground *DefaultUIPackageBuilder::BeginBgPropertiesSection(int index, bool sectionHasProperties)
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
    
    UIControl *DefaultUIPackageBuilder::BeginInternalControlSection(int index, bool sectionHasProperties)
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
        SafeRetain(descr.control);
        SafeRelease(control);
        
        control = descr.control;
        addToParent = descr.addToParent;
        return *this;
    }

}
