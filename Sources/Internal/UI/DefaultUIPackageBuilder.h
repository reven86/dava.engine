#ifndef __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__

#include "AbstractUIPackageBuilder.h"
#include "UIPackage.h"
#include "UI/UIControlPackageContext.h"

namespace DAVA
{
class UIPackagesCache;

class DefaultUIPackageBuilder : public AbstractUIPackageBuilder
{
public:
    DefaultUIPackageBuilder(UIPackagesCache* _packagesCache = nullptr);
    virtual ~DefaultUIPackageBuilder();

    UIPackage* GetPackage() const;
    UIPackage* FindInCache(const String& packagePath) const;

    virtual void BeginPackage(const FilePath& packagePath) override;
    virtual void EndPackage() override;

    virtual bool ProcessImportedPackage(const String& packagePath, AbstractUIPackageLoader* loader) override;
    virtual void ProcessStyleSheet(const Vector<UIStyleSheetSelectorChain>& selectorChains, const Vector<UIStyleSheetProperty>& properties) override;

    virtual UIControl* BeginControlWithClass(const String& className) override;
    virtual UIControl* BeginControlWithCustomClass(const String& customClassName, const String& className) override;
    virtual UIControl* BeginControlWithPrototype(const String& packageName, const String& prototypeName, const String* customClassName, AbstractUIPackageLoader* loader) override;
    virtual UIControl* BeginControlWithPath(const String& pathName) override;
    virtual UIControl* BeginUnknownControl(const YamlNode* node) override;
    virtual void EndControl(bool isRoot) override;

    virtual void BeginControlPropertiesSection(const String& name) override;
    virtual void EndControlPropertiesSection() override;

    virtual UIComponent* BeginComponentPropertiesSection(uint32 componentType, uint32 componentIndex) override;
    virtual void EndComponentPropertiesSection() override;

    virtual UIControlBackground* BeginBgPropertiesSection(int32 index, bool sectionHasProperties) override;
    virtual void EndBgPropertiesSection() override;

    virtual UIControl* BeginInternalControlSection(int32 index, bool sectionHasProperties) override;
    virtual void EndInternalControlSection() override;

    virtual void ProcessProperty(const InspMember* member, const VariantType& value) override;

private:
    void PutImportredPackage(const FilePath& path, UIPackage* package);
    UIPackage* FindImportedPackageByName(const String& name) const;

private:
    //class PackageDescr;
    struct ControlDescr;

    //Vector<PackageDescr*> packagesStack;
    Vector<ControlDescr*> controlsStack;

    UIPackagesCache* cache;
    BaseObject* currentObject;

    RefPtr<UIPackage> package;

    Vector<UIPackage*> importedPackages;
    Vector<UIPriorityStyleSheet> styleSheets;
    Map<FilePath, int32> packsByPaths;
    Map<String, int32> packsByNames;
};
}

#endif // __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__
