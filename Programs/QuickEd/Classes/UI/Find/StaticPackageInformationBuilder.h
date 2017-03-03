#pragma once

#include "UI/AbstractUIPackageBuilder.h"
#include "FileSystem/FilePath.h"
#include "UI/Styles/UIStyleSheetStructs.h"
#include "Model/ControlProperties/SectionProperty.h"

#include "StaticPackageInformation.h"
#include "StaticControlInformation.h"

class PackageNode;
class ControlNode;
class StyleSheetNode;
class ControlsContainerNode;
class IntrospectionProperty;

class StaticPackageInformationBuilder : public DAVA::AbstractUIPackageBuilder
{
public:
    StaticPackageInformationBuilder(PackageInformationCache* cache);
    ~StaticPackageInformationBuilder() override;

    void BeginPackage(const DAVA::FilePath& packagePath) override;
    void EndPackage() override;

    bool ProcessImportedPackage(const DAVA::String& packagePath, DAVA::AbstractUIPackageLoader* loader) override;
    void ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain>& selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty>& properties) override;

    UIControlWithTypeInfo BeginControlWithClass(const DAVA::FastName& controlName, const DAVA::String& className) override;
    UIControlWithTypeInfo BeginControlWithCustomClass(const DAVA::FastName& controlName, const DAVA::String& customClassName, const DAVA::String& className) override;
    UIControlWithTypeInfo BeginControlWithPrototype(const DAVA::FastName& controlName, const DAVA::String& packageName, const DAVA::FastName& prototypeName, const DAVA::String* customClassName, DAVA::AbstractUIPackageLoader* loader) override;
    UIControlWithTypeInfo BeginControlWithPath(const DAVA::String& pathName) override;
    UIControlWithTypeInfo BeginUnknownControl(const DAVA::FastName& controlName, const DAVA::YamlNode* node) override;
    void EndControl(eControlPlace controlPlace) override;

    void BeginControlPropertiesSection(const DAVA::String& name) override;
    void EndControlPropertiesSection() override;

    const DAVA::InspInfo* BeginComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex) override;
    void EndComponentPropertiesSection() override;

    void ProcessProperty(const DAVA::InspMember* member, const DAVA::VariantType& value) override;

    std::shared_ptr<StaticPackageInformation> GetPackage() const;

private:
    struct Description;

    std::shared_ptr<StaticPackageInformation> packageInformation;
    DAVA::Vector<Description> stack;
    DAVA::Function<void(const DAVA::InspMember* member, const DAVA::VariantType& value)> propertyProcessor;

    PackageInformationCache* cache = nullptr;
};
