#include "StaticPackageInformationBuilder.h"
#include "Utils/Utils.h"
using namespace DAVA;

struct StaticPackageInformationBuilder::Description
{
    std::shared_ptr<StaticControlInformation> controlInformation;
    bool addToParent = false;

    Description(const std::shared_ptr<StaticControlInformation>& controlInformation_, bool addToParent_)
        : controlInformation(controlInformation_)
        , addToParent(addToParent_)
    {
    }
};

StaticPackageInformationBuilder::StaticPackageInformationBuilder(PackageInformationCache* cache_)
    : cache(cache_)
{
}

StaticPackageInformationBuilder::~StaticPackageInformationBuilder()
{
}

void StaticPackageInformationBuilder::BeginPackage(const DAVA::FilePath& packagePath)
{
    DVASSERT(packageInformation.get() == nullptr);
    packageInformation = std::make_shared<StaticPackageInformation>(packagePath.GetFrameworkPath());
}

void StaticPackageInformationBuilder::EndPackage()
{
}

bool StaticPackageInformationBuilder::ProcessImportedPackage(const DAVA::String& packagePathStr, DAVA::AbstractUIPackageLoader* loader)
{
    std::shared_ptr<StaticPackageInformation> pack = cache->Find(packagePathStr);
    if (pack)
    {
        packageInformation->AddImportedPackage(pack);
        return true;
    }
    else
    {
        FilePath packagePath(packagePathStr);
        StaticPackageInformationBuilder builder(cache);

        if (loader->LoadPackage(packagePath, &builder))
        {
            pack = builder.GetPackage();

            cache->Put(pack);
            packageInformation->AddImportedPackage(pack);
            return true;
        }
    }

    DVASSERT(false);
    return false;
}

void StaticPackageInformationBuilder::ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain>& selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty>& properties)
{
    // do nothing
}

DAVA::UIControl* StaticPackageInformationBuilder::BeginControlWithClass(const FastName& controlName, const DAVA::String& className)
{
    stack.emplace_back(Description(std::make_shared<StaticControlInformation>(controlName), true));
    return nullptr;
}

DAVA::UIControl* StaticPackageInformationBuilder::BeginControlWithCustomClass(const FastName& controlName, const DAVA::String& customClassName, const DAVA::String& className)
{
    stack.emplace_back(Description(std::make_shared<StaticControlInformation>(controlName), true));
    return nullptr;
}

DAVA::UIControl* StaticPackageInformationBuilder::BeginControlWithPrototype(const FastName& controlName, const DAVA::String& packageName, const DAVA::FastName& prototypeName, const DAVA::String* customClassName, DAVA::AbstractUIPackageLoader* loader)
{
    std::shared_ptr<StaticPackageInformation> prototypePackage;
    std::shared_ptr<StaticControlInformation> prototype;

    if (packageName.empty())
    {
        prototypePackage = packageInformation;
        prototype = packageInformation->FindPrototypeByName(prototypeName);

        if (prototype.get() == nullptr)
        {
            if (loader->LoadControlByName(prototypeName, this))
            {
                prototype = packageInformation->FindPrototypeByName(prototypeName);
            }
        }
    }
    else
    {
        const DAVA::Vector<std::shared_ptr<StaticPackageInformation>>& packages = packageInformation->GetImportedPackages();
        auto it = std::find_if(packages.begin(), packages.end(), [packageName](const std::shared_ptr<StaticPackageInformation>& pack) {
            return FilePath(pack->GetPath()).GetBasename() == packageName;
        });

        prototypePackage = *it;
        prototype = prototypePackage->FindPrototypeByName(prototypeName);
    }

    DVASSERT(prototypePackage.get() != nullptr);
    DVASSERT(prototype.get() != nullptr);

    stack.emplace_back(Description(std::make_shared<StaticControlInformation>(*prototype, controlName, prototypePackage, FastName(prototypeName)), true));
    return nullptr;
}

DAVA::UIControl* StaticPackageInformationBuilder::BeginControlWithPath(const DAVA::String& pathName)
{
    if (!stack.empty())
    {
        std::shared_ptr<StaticControlInformation> ptr = stack.back().controlInformation;

        Vector<String> controlNames;
        Split(pathName, "/", controlNames, false, true);
        for (String& name : controlNames)
        {
            ptr = ptr->FindChildByName(FastName(name));
            if (!ptr)
            {
                DVASSERT(false);
                break;
            }
        }

        DVASSERT(ptr.get() != nullptr);
        stack.emplace_back(Description(ptr, false));
    }
    else
    {
        DVASSERT(false);
    }

    return nullptr; // do nothing
}

DAVA::UIControl* StaticPackageInformationBuilder::BeginUnknownControl(const FastName& controlName, const DAVA::YamlNode* node)
{
    stack.emplace_back(Description(std::make_shared<StaticControlInformation>(controlName), true));
    return nullptr; // do nothing
}

void StaticPackageInformationBuilder::EndControl(eControlPlace controlPlace)
{
    Description descr = stack.back();
    stack.pop_back();

    if (descr.addToParent)
    {
        switch (controlPlace)
        {
        case TO_CONTROLS:
            packageInformation->AddControl(descr.controlInformation);
            break;

        case TO_PROTOTYPES:
            packageInformation->AddPrototype(descr.controlInformation);
            break;

        case TO_PREVIOUS_CONTROL:
            DVASSERT(!stack.empty());
            descr.controlInformation->SetParent(stack.back().controlInformation.get());
            stack.back().controlInformation->AddChild(descr.controlInformation);
            break;

        default:
            DVASSERT(false);
            break;
        }
    }
}

void StaticPackageInformationBuilder::BeginControlPropertiesSection(const DAVA::String& name)
{
    // do nothing
}

void StaticPackageInformationBuilder::EndControlPropertiesSection()
{
    // do nothing
}

DAVA::UIComponent* StaticPackageInformationBuilder::BeginComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex)
{
    return nullptr; // do nothing
}

void StaticPackageInformationBuilder::EndComponentPropertiesSection()
{
    // do nothing
}

void StaticPackageInformationBuilder::ProcessProperty(const DAVA::InspMember* member, const DAVA::VariantType& value)
{
}

std::shared_ptr<StaticPackageInformation> StaticPackageInformationBuilder::GetPackage() const
{
    return packageInformation;
}
