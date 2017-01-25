#pragma once

#include "UI/AbstractUIPackageBuilder.h"
#include "FileSystem/FilePath.h"
#include "UI/Styles/UIStyleSheetStructs.h"
#include "Model/ControlProperties/SectionProperty.h"

class PackageNode;
class ControlNode;
class StyleSheetNode;
class ControlsContainerNode;
class IntrospectionProperty;

class QuickEdPackageBuilder : public DAVA::AbstractUIPackageBuilder
{
public:
    QuickEdPackageBuilder();
    virtual ~QuickEdPackageBuilder();

    virtual void BeginPackage(const DAVA::FilePath& packagePath) override;
    virtual void EndPackage() override;

    virtual bool ProcessImportedPackage(const DAVA::String& packagePath, DAVA::AbstractUIPackageLoader* loader) override;
    virtual void ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain>& selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty>& properties) override;

    virtual DAVA::UIControl* BeginControlWithClass(const DAVA::FastName& controlName, const DAVA::String& className) override;
    virtual DAVA::UIControl* BeginControlWithCustomClass(const DAVA::FastName& controlName, const DAVA::String& customClassName, const DAVA::String& className) override;
    virtual DAVA::UIControl* BeginControlWithPrototype(const DAVA::FastName& controlName, const DAVA::String& packageName, const DAVA::FastName& prototypeName, const DAVA::String* customClassName, DAVA::AbstractUIPackageLoader* loader) override;
    virtual DAVA::UIControl* BeginControlWithPath(const DAVA::String& pathName) override;
    virtual DAVA::UIControl* BeginUnknownControl(const DAVA::FastName& controlName, const DAVA::YamlNode* node) override;
    virtual void EndControl(eControlPlace controlPlace) override;

    virtual void BeginControlPropertiesSection(const DAVA::String& name) override;
    virtual void EndControlPropertiesSection() override;

    virtual DAVA::UIComponent* BeginComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex) override;
    virtual void EndComponentPropertiesSection() override;

    virtual void ProcessProperty(const DAVA::InspMember* member, const DAVA::VariantType& value) override;

    DAVA::RefPtr<PackageNode> BuildPackage() const;
    const DAVA::Vector<ControlNode*>& GetRootControls() const;
    const DAVA::Vector<PackageNode*>& GetImportedPackages() const;
    const DAVA::Vector<StyleSheetNode*>& GetStyles() const;
    void AddImportedPackage(PackageNode* node);

private:
    ControlNode* FindPrototype(const DAVA::String& name) const;

private:
    struct ControlDescr
    {
        ControlNode* node;
        bool addToParent;

        ControlDescr();
        ControlDescr(ControlNode* node, bool addToParent);
        ControlDescr(const ControlDescr& descr);
        ~ControlDescr();
        ControlDescr& operator=(const ControlDescr& descr);
    };

private:
    DAVA::FilePath packagePath;
    DAVA::List<ControlDescr> controlsStack;

    DAVA::Vector<PackageNode*> importedPackages;
    DAVA::Vector<ControlNode*> rootControls;
    DAVA::Vector<ControlNode*> prototypes;
    DAVA::Vector<StyleSheetNode*> styleSheets;
    DAVA::Vector<DAVA::FilePath> declinedPackages;

    DAVA::BaseObject* currentObject;
    SectionProperty<IntrospectionProperty>* currentSection;
};
