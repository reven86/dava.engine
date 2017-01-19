#include "PackageInformationBuilder.h"
#include "Utils/Utils.h"
using namespace DAVA;

struct PackageInformationBuilder::Description
{
    std::shared_ptr<ControlInformation> controlInformation;
    bool addToParent = false;

    Description(const std::shared_ptr<ControlInformation>& controlInformation_, bool addToParent_)
        : controlInformation(controlInformation_)
        , addToParent(addToParent_)
    {
    }
};

PackageInformationBuilder::PackageInformationBuilder(PackageInformationCache* cache_)
    : cache(cache_)
{
}

PackageInformationBuilder::~PackageInformationBuilder()
{
}

void PackageInformationBuilder::BeginPackage(const DAVA::FilePath& packagePath)
{
    DVASSERT(packageInformation.get() == nullptr);
    packageInformation = std::make_shared<PackageInformation>(packagePath.GetFrameworkPath());
}

void PackageInformationBuilder::EndPackage()
{
}

bool PackageInformationBuilder::ProcessImportedPackage(const DAVA::String& packagePathStr, DAVA::AbstractUIPackageLoader* loader)
{
    std::shared_ptr<PackageInformation> pack = cache->Find(packagePathStr);
    if (pack)
    {
        packageInformation->AddImportedPackage(pack);
        return true;
    }
    else
    {
        FilePath packagePath(packagePathStr);
        PackageInformationBuilder builder(cache);

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

void PackageInformationBuilder::ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain>& selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty>& properties)
{
    // do nothing
}

DAVA::UIControl* PackageInformationBuilder::BeginControlWithClass(const FastName& controlName, const DAVA::String& className)
{
    stack.emplace_back(Description(std::make_shared<ControlInformation>(controlName), true));
    return nullptr;
}

DAVA::UIControl* PackageInformationBuilder::BeginControlWithCustomClass(const FastName& controlName, const DAVA::String& customClassName, const DAVA::String& className)
{
    stack.emplace_back(Description(std::make_shared<ControlInformation>(controlName), true));
    return nullptr;
}

DAVA::UIControl* PackageInformationBuilder::BeginControlWithPrototype(const FastName& controlName, const DAVA::String& packageName, const DAVA::FastName& prototypeName, const DAVA::String* customClassName, DAVA::AbstractUIPackageLoader* loader)
{
    std::shared_ptr<PackageInformation> prototypePackage;
    std::shared_ptr<ControlInformation> prototype;

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
        const DAVA::Vector<std::shared_ptr<PackageInformation>>& packages = packageInformation->GetImportedPackages();
        auto it = std::find_if(packages.begin(), packages.end(), [packageName](const std::shared_ptr<PackageInformation>& pack) {
            return FilePath(pack->GetPath()).GetBasename() == packageName;
        });

        prototypePackage = *it;
        prototype = prototypePackage->FindPrototypeByName(prototypeName);
    }

    DVASSERT(prototypePackage.get() != nullptr);
    DVASSERT(prototype.get() != nullptr);

    stack.emplace_back(Description(std::make_shared<ControlInformation>(*prototype, controlName, prototypePackage, FastName(prototypeName)), true));
    return nullptr;
}

DAVA::UIControl* PackageInformationBuilder::BeginControlWithPath(const DAVA::String& pathName)
{
    if (!stack.empty())
    {
        std::shared_ptr<ControlInformation> ptr = stack.back().controlInformation;

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

DAVA::UIControl* PackageInformationBuilder::BeginUnknownControl(const FastName& controlName, const DAVA::YamlNode* node)
{
    stack.emplace_back(Description(std::make_shared<ControlInformation>(controlName), true));
    return nullptr; // do nothing
}

void PackageInformationBuilder::EndControl(eControlPlace controlPlace)
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

void PackageInformationBuilder::BeginControlPropertiesSection(const DAVA::String& name)
{
    // do nothing
}

void PackageInformationBuilder::EndControlPropertiesSection()
{
    // do nothing
}

DAVA::UIComponent* PackageInformationBuilder::BeginComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex)
{
    return nullptr; // do nothing
}

void PackageInformationBuilder::EndComponentPropertiesSection()
{
    // do nothing
}

void PackageInformationBuilder::ProcessProperty(const DAVA::InspMember* member, const DAVA::VariantType& value)
{
}

std::shared_ptr<PackageInformation> PackageInformationBuilder::GetPackage() const
{
    return packageInformation;
}
