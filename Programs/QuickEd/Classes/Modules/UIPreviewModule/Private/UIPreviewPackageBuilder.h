#pragma once

#include <UI/AbstractUIPackageBuilder.h>

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>

class FilesCollection
{
public:
    DAVA::Set<DAVA::FilePath> yamlFiles;
    DAVA::Set<DAVA::FilePath> spritesFolders;
    DAVA::Set<DAVA::FilePath> effectsFiles;
};

class UIPreviewPackageBuilder : public DAVA::AbstractUIPackageBuilder
{
public:
    UIPreviewPackageBuilder(FilesCollection* cache);

    void BeginPackage(const DAVA::FilePath& packagePath) override;
    void EndPackage() override;

    bool ProcessImportedPackage(const DAVA::String& packagePath, DAVA::AbstractUIPackageLoader* loader) override;
    void ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain>& selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty>& properties) override;

    const DAVA::ReflectedType* BeginControlWithClass(const DAVA::FastName& controlName, const DAVA::String& className) override;
    const DAVA::ReflectedType* BeginControlWithCustomClass(const DAVA::FastName& controlName, const DAVA::String& customClassName, const DAVA::String& className) override;
    const DAVA::ReflectedType* BeginControlWithPrototype(const DAVA::FastName& controlName, const DAVA::String& packageName, const DAVA::FastName& prototypeName, const DAVA::String* customClassName, DAVA::AbstractUIPackageLoader* loader) override;
    const DAVA::ReflectedType* BeginControlWithPath(const DAVA::String& pathName) override;
    const DAVA::ReflectedType* BeginUnknownControl(const DAVA::FastName& controlName, const DAVA::YamlNode* node) override;
    void EndControl(eControlPlace controlPlace) override;

    void BeginControlPropertiesSection(const DAVA::String& name) override;
    void EndControlPropertiesSection() override;

    const DAVA::ReflectedType* BeginComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex) override;
    void EndComponentPropertiesSection() override;

    void ProcessProperty(const DAVA::ReflectedStructure::Field& field, const DAVA::Any& value) override;

private:
    void CollectFilePath(const DAVA::FilePath& path);

    FilesCollection* cache = nullptr;
};
