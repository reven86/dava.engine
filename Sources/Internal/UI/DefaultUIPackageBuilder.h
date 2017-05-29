#pragma once

#include "AbstractUIPackageBuilder.h"
#include "UIPackage.h"
#include "UI/UIControlPackageContext.h"

#include <memory>

namespace DAVA
{
class UIPackagesCache;

class DefaultUIPackageBuilder : public AbstractUIPackageBuilder
{
public:
    DefaultUIPackageBuilder(UIPackagesCache* _packagesCache = nullptr);
    ~DefaultUIPackageBuilder() override;

    UIPackage* GetPackage() const;
    UIPackage* FindInCache(const String& packagePath) const;

    void BeginPackage(const FilePath& packagePath, int32 version) override;
    void EndPackage() override;

    bool ProcessImportedPackage(const String& packagePath, AbstractUIPackageLoader* loader) override;
    void ProcessStyleSheet(const Vector<UIStyleSheetSelectorChain>& selectorChains, const Vector<UIStyleSheetProperty>& properties) override;

    const ReflectedType* BeginControlWithClass(const FastName& controlName, const String& className) override;
    const ReflectedType* BeginControlWithCustomClass(const FastName& controlName, const String& customClassName, const String& className) override;
    const ReflectedType* BeginControlWithPrototype(const FastName& controlName, const String& packageName, const FastName& prototypeName, const String* customClassName, AbstractUIPackageLoader* loader) override;
    const ReflectedType* BeginControlWithPath(const String& pathName) override;
    const ReflectedType* BeginUnknownControl(const FastName& controlName, const YamlNode* node) override;
    void EndControl(eControlPlace controlPlace) override;

    void BeginControlPropertiesSection(const String& name) override;
    void EndControlPropertiesSection() override;

    const ReflectedType* BeginComponentPropertiesSection(uint32 componentType, uint32 componentIndex) override;
    void EndComponentPropertiesSection() override;

    void ProcessProperty(const ReflectedStructure::Field& field, const Any& value) override;

protected:
    virtual RefPtr<UIControl> CreateControlByName(const String& customClassName, const String& className);
    virtual std::unique_ptr<DefaultUIPackageBuilder> CreateBuilder(UIPackagesCache* packagesCache);

private:
    void PutImportredPackage(const FilePath& path, UIPackage* package);
    UIPackage* FindImportedPackageByName(const String& name) const;

private:
    //class PackageDescr;
    struct ControlDescr;

    //Vector<PackageDescr*> packagesStack;
    Vector<ControlDescr*> controlsStack;

    UIPackagesCache* cache;
    ReflectedObject currentObject;
    int32 currentComponentType = -1;

    RefPtr<UIPackage> package;
    FilePath currentPackagePath;

    Vector<UIPackage*> importedPackages;
    Vector<UIPriorityStyleSheet> styleSheets;
    Map<FilePath, int32> packsByPaths;
    Map<String, int32> packsByNames;
};
}
