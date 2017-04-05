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
#include "Styles/UIStyleSheetSystem.h"

#include "Logger/Logger.h"

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

    ControlDescr(UIControl* aControl, bool aAddToParent)
        : control(SafeRetain(aControl))
        , addToParent(aAddToParent)
    {
    }
};

DefaultUIPackageBuilder::DefaultUIPackageBuilder(UIPackagesCache* _cache)
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
        for (auto& descr : controlsStack)
            SafeDelete(descr);

        controlsStack.clear();

        DVASSERT(false);
    }

    for (UIPackage* importedPackage : importedPackages)
        SafeRelease(importedPackage);
    importedPackages.clear();
}

UIPackage* DefaultUIPackageBuilder::GetPackage() const
{
    return package.Get();
}

UIPackage* DefaultUIPackageBuilder::FindInCache(const String& packagePath) const
{
    return cache->GetPackage(packagePath);
}

void DefaultUIPackageBuilder::BeginPackage(const FilePath& packagePath)
{
    DVASSERT(!package.Valid());
    package = RefPtr<UIPackage>(new UIPackage());
    currentPackagePath = packagePath;
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

    // kill duplicates
    {
        std::sort(styleSheets.begin(), styleSheets.end(), [](const UIPriorityStyleSheet& a, const UIPriorityStyleSheet& b)
                  {
                      const UIStyleSheet* s1 = a.GetStyleSheet();
                      const UIStyleSheet* s2 = b.GetStyleSheet();
                      return s1 == s2 ? a.GetPriority() < b.GetPriority() : s1 < s2;
                  });
        auto lastNeeded = std::unique(styleSheets.begin(), styleSheets.end(), [](const UIPriorityStyleSheet& a, const UIPriorityStyleSheet& b)
                                      {
                                          return a.GetStyleSheet() == b.GetStyleSheet();
                                      });
        styleSheets.erase(lastNeeded, styleSheets.end());
    }

    for (const UIPriorityStyleSheet& styleSheet : styleSheets)
    {
        package->GetControlPackageContext()->AddStyleSheet(styleSheet);
    }

    styleSheets.clear();
}

bool DefaultUIPackageBuilder::ProcessImportedPackage(const String& packagePath, AbstractUIPackageLoader* loader)
{
    UIPackage* importedPackage = cache->GetPackage(packagePath);

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

void DefaultUIPackageBuilder::ProcessStyleSheet(const Vector<UIStyleSheetSelectorChain>& selectorChains, const Vector<UIStyleSheetProperty>& properties)
{
    for (const UIStyleSheetSelectorChain& chain : selectorChains)
    {
        ScopedPtr<UIStyleSheet> styleSheet(new UIStyleSheet());
        styleSheet->SetSelectorChain(chain);
        ScopedPtr<UIStyleSheetPropertyTable> propertiesTable(new UIStyleSheetPropertyTable());
        propertiesTable->SetProperties(properties);
        styleSheet->SetPropertyTable(propertiesTable);
        styleSheet->SetSourceInfo(UIStyleSheetSourceInfo(currentPackagePath));

        package->GetControlPackageContext()->AddStyleSheet(UIPriorityStyleSheet(styleSheet));
    }
}

UIControl* DefaultUIPackageBuilder::BeginControlWithClass(const FastName& controlName, const String& className)
{
    RefPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(className));

    if (control.Valid())
    {
        if (className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST) //TODO: fix internal staticText for Win\Mac
        {
            control->RemoveAllControls();
        }

        if (controlName.IsValid())
        {
            control->SetName(controlName);
        }
    }
    else
    {
        DVASSERT(false);
    }

    controlsStack.push_back(new ControlDescr(control.Get(), true));
    return control.Get();
}

UIControl* DefaultUIPackageBuilder::BeginControlWithCustomClass(const FastName& controlName, const String& customClassName, const String& className)
{
    RefPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(customClassName));

    if (control.Valid())
    {
        if (className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST) //TODO: fix internal staticText for Win\Mac
        {
            control->RemoveAllControls();
        }

        if (controlName.IsValid())
        {
            control->SetName(controlName);
        }
    }
    else
    {
        DVASSERT(false);
        control.Set(ObjectFactory::Instance()->New<UIControl>(className)); // TODO: remove
    }

    DVASSERT(control.Valid());

    controlsStack.push_back(new ControlDescr(control.Get(), true));
    return control.Get();
}

UIControl* DefaultUIPackageBuilder::BeginControlWithPrototype(const FastName& controlName, const String& packageName, const FastName& prototypeName, const String* customClassName, AbstractUIPackageLoader* loader)
{
    UIControl* prototype = nullptr;

    if (packageName.empty())
    {
        prototype = package->GetPrototype(prototypeName);
        if (!prototype)
        {
            if (loader->LoadControlByName(prototypeName, this))
                prototype = package->GetPrototype(prototypeName);
        }
    }
    else
    {
        UIPackage* importedPackage = FindImportedPackageByName(packageName);
        if (importedPackage)
            prototype = importedPackage->GetPrototype(prototypeName);
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

    if (controlName.IsValid())
    {
        control->SetName(controlName);
    }

    control->SetPackageContext(nullptr);

    controlsStack.push_back(new ControlDescr(control.Get(), true));
    return control.Get();
}

UIControl* DefaultUIPackageBuilder::BeginControlWithPath(const String& pathName)
{
    UIControl* control = nullptr;
    if (!controlsStack.empty())
    {
        control = controlsStack.back()->control->FindByPath(pathName);
    }

    DVASSERT(control);
    controlsStack.push_back(new ControlDescr(control, false));
    return control;
}

UIControl* DefaultUIPackageBuilder::BeginUnknownControl(const FastName& controlName, const YamlNode* node)
{
    DVASSERT(false);
    controlsStack.push_back(new ControlDescr(nullptr, false));
    return nullptr;
}

void DefaultUIPackageBuilder::EndControl(eControlPlace controlPlace)
{
    ControlDescr* lastDescr = controlsStack.back();
    controlsStack.pop_back();
    if (lastDescr->addToParent)
    {
        switch (controlPlace)
        {
        case TO_PROTOTYPES:
        {
            UIControl* control = lastDescr->control.Get();
            UIControlSystem::Instance()->GetLayoutSystem()->ManualApplyLayout(control);
            package->AddPrototype(control);
            break;
        }

        case TO_CONTROLS:
        {
            UIControl* control = lastDescr->control.Get();
            UIControlSystem::Instance()->GetLayoutSystem()->ManualApplyLayout(control);
            package->AddControl(control);
            break;
        }

        case TO_PREVIOUS_CONTROL:
        {
            DVASSERT(!controlsStack.empty());
            UIControl* control = controlsStack.back()->control.Get();
            control->AddControl(lastDescr->control.Get());
            break;
        }

        default:
            DVASSERT(false);
            break;
        }
    }
    SafeDelete(lastDescr);
}

void DefaultUIPackageBuilder::BeginControlPropertiesSection(const String& name)
{
    DVASSERT(currentComponentType == -1);
    currentObject = controlsStack.back()->control.Get();
}

void DefaultUIPackageBuilder::EndControlPropertiesSection()
{
    currentObject = nullptr;
}

UIComponent* DefaultUIPackageBuilder::BeginComponentPropertiesSection(uint32 componentType, uint32 componentIndex)
{
    UIControl* control = controlsStack.back()->control.Get();
    UIComponent* component = control->GetComponent(componentType, componentIndex);
    if (component == nullptr)
    {
        component = UIComponent::CreateByType(componentType);
        control->AddComponent(component);
        component->Release();
    }
    currentObject = component;
    currentComponentType = int32(componentType);
    return component;
}

void DefaultUIPackageBuilder::EndComponentPropertiesSection()
{
    currentComponentType = -1;
    currentObject = nullptr;
}

void DefaultUIPackageBuilder::ProcessProperty(const Reflection::Field& field, const Any& value)
{
    DVASSERT(currentObject);

    if (currentObject && !value.IsEmpty())
    {
        FastName name = field.key.Cast<FastName>();
        int32 propertyIndex = UIStyleSheetPropertyDataBase::Instance()->FindStyleSheetProperty(currentComponentType, name);
        if (propertyIndex >= 0)
        {
            UIControl* control = controlsStack.back()->control.Get();
            control->SetPropertyLocalFlag(propertyIndex, true);
        }
        if (name == PROPERTY_NAME_TEXT)
        {
            field.ref.SetValueWithCast(Any(LocalizedUtf8String(value.Cast<String>())));
        }
        else
        {
            field.ref.SetValueWithCast(value);
        }
    }
}

void DefaultUIPackageBuilder::PutImportredPackage(const FilePath& path, UIPackage* package)
{
    int32 index = static_cast<int32>(importedPackages.size());
    importedPackages.push_back(SafeRetain(package));
    packsByPaths[path] = index;
    packsByNames[path.GetBasename()] = index;
}

UIPackage* DefaultUIPackageBuilder::FindImportedPackageByName(const String& name) const
{
    auto it = packsByNames.find(name);
    if (it != packsByNames.end())
        return importedPackages[it->second];

    return nullptr;
}
}
