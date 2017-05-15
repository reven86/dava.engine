#include "Modules/UIPreviewModule/Private/UIPreviewPackageBuilder.h"

#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedTypeDB.h>
#include <UI/UIControl.h>
#include <Utils/StringFormat.h>

UIPreviewPackageBuilder::UIPreviewPackageBuilder(FilesCollection* cache_)
    : cache(cache_)
{
}

void UIPreviewPackageBuilder::BeginPackage(const DAVA::FilePath& packagePath)
{
    cache->yamlFiles.insert(packagePath);
}

void UIPreviewPackageBuilder::EndPackage()
{
}

bool UIPreviewPackageBuilder::ProcessImportedPackage(const DAVA::String& packagePathStr, DAVA::AbstractUIPackageLoader* loader)
{
    if (cache->yamlFiles.count(packagePathStr) == 0)
    {
        UIPreviewPackageBuilder builder(cache);
        return (loader->LoadPackage(packagePathStr, &builder));
    }

    return true;
}

void UIPreviewPackageBuilder::ProcessStyleSheet(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain>& selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty>& properties)
{
    using namespace DAVA;

    for (const UIStyleSheetProperty& prop : properties)
    {
        const Any& value = prop.value;
        if (value.IsEmpty() == false && value.CanGet<FilePath>())
        {
            FilePath path = value.Get<FilePath>();
            if (path.IsEmpty() == false)
            {
                CollectFilePath(path);
            }
        }
    }
}

const DAVA::ReflectedType* UIPreviewPackageBuilder::BeginControlWithClass(const DAVA::FastName& controlName, const DAVA::String& className)
{
    using namespace DAVA;
    const ReflectedType* ret = ret = ReflectedTypeDB::GetByPermanentName(className);
    if (ret == nullptr)
    {
        ret = ReflectedTypeDB::Get<UIControl>();
    }

    return ret;
}

const DAVA::ReflectedType* UIPreviewPackageBuilder::BeginControlWithCustomClass(const DAVA::FastName& controlName, const DAVA::String& customClassName, const DAVA::String& className)
{
    using namespace DAVA;
    const ReflectedType* ret = ret = ReflectedTypeDB::GetByPermanentName(className);
    if (ret == nullptr)
    {
        ret = ReflectedTypeDB::Get<UIControl>();
    }

    return ret;
}

const DAVA::ReflectedType* UIPreviewPackageBuilder::BeginControlWithPrototype(const DAVA::FastName& controlName, const DAVA::String& packageName, const DAVA::FastName& prototypeName, const DAVA::String* customClassName, DAVA::AbstractUIPackageLoader* loader)
{
    using namespace DAVA;

    if (packageName.empty())
    {
        loader->LoadControlByName(prototypeName, this);
    }

    const ReflectedType* ret = nullptr;
    if (customClassName != nullptr)
    {
        ret = ReflectedTypeDB::GetByPermanentName(*customClassName);
    }

    if (ret == nullptr)
    {
        ret = ReflectedTypeDB::Get<UIControl>();
    }
    return ret;
}

const DAVA::ReflectedType* UIPreviewPackageBuilder::BeginControlWithPath(const DAVA::String& pathName)
{
    return DAVA::ReflectedTypeDB::Get<DAVA::UIControl>();
}

const DAVA::ReflectedType* UIPreviewPackageBuilder::BeginUnknownControl(const DAVA::FastName& controlName, const DAVA::YamlNode* node)
{
    return DAVA::ReflectedTypeDB::Get<DAVA::UIControl>();
}

void UIPreviewPackageBuilder::EndControl(eControlPlace controlPlace)
{
}

void UIPreviewPackageBuilder::BeginControlPropertiesSection(const DAVA::String& name)
{
}

void UIPreviewPackageBuilder::EndControlPropertiesSection()
{
}

const DAVA::ReflectedType* UIPreviewPackageBuilder::BeginComponentPropertiesSection(const DAVA::Type* componentType, DAVA::uint32 componentIndex)
{
    using namespace DAVA;
    if (Type::Instance<UIControlBackground>() == componentType)
    {
        return ReflectedTypeDB::Get<UIControlBackground>();
    }

    return nullptr;
}

void UIPreviewPackageBuilder::EndComponentPropertiesSection()
{
}

void UIPreviewPackageBuilder::ProcessProperty(const DAVA::ReflectedStructure::Field& field, const DAVA::Any& value)
{
    using namespace DAVA;

    if (value.IsEmpty() == false && value.CanGet<FilePath>())
    {
        FilePath path = value.Get<FilePath>();
        if (path.IsEmpty() == false)
        {
            CollectFilePath(path);
        }
    }
}

void UIPreviewPackageBuilder::CollectFilePath(const DAVA::FilePath& path)
{
    using namespace DAVA;

    if (path.IsEqualToExtension(".sc2"))
    {
        cache->effectsFiles.insert(path);
    }
    else if (path.GetExtension().empty() || path.IsEqualToExtension(".txt"))
    { // trick to resolve real path
        FilePath tmp = path;
        if (tmp.GetExtension().empty())
        {
            tmp.ReplaceExtension(".txt");
        }

        cache->spritesFolders.insert(FilePath(tmp.GetAbsolutePathname()).GetDirectory());
    }
    else
    {
        DVASSERT(false, Format("undefined path: %s", path.GetStringValue().c_str()).c_str());
    }
}
