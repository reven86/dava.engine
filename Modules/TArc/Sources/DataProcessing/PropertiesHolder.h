#pragma once

#include <memory>

#include "Base/BaseTypes.h"
#include "Base/Any.h"

namespace DAVA
{
class FilePath;
class Type;

namespace TArc
{
class PropertiesItem;

class RootPropertiesHolder
{
public:
    RootPropertiesHolder(const String& rootPath, const FilePath& dirPath);
    ~RootPropertiesHolder();

    PropertiesItem CreateSubHolder(const String& name) const;

    void SaveToFile();
    void SetDirectory(const FilePath& dirPath);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

class PropertiesItem
{
public:
    virtual ~PropertiesItem();

    PropertiesItem CreateSubHolder(const String& name) const;

    void Set(const String& key, const Any& value);

    template <typename T>
    T Get(const String& key, const T& defaultValue = T()) const;

    PropertiesItem(const PropertiesItem& holder) = delete;
    PropertiesItem(PropertiesItem&& holder);
    PropertiesItem& operator=(const PropertiesItem& holder) = delete;
    PropertiesItem& operator=(PropertiesItem&& holder);

private:
    friend class RootPropertiesHolder;
    //RootPropertiesHolder use this empty c-tor
    PropertiesItem();
    Any Get(const String& key, const Any& defaultValue, const Type* type) const;
    PropertiesItem(const PropertiesItem& parent, const String& name);

    struct Impl;
    std::unique_ptr<Impl> impl;
};

template <typename T>
T PropertiesItem::Get(const String& key, const T& defaultValue) const
{
    Any loadedValue = Get(key, defaultValue, Type::Instance<T>());
    if (loadedValue.CanGet<T>())
    {
        return loadedValue.Get<T>();
    }
    else
    {
        return defaultValue;
    }
}

} // namespace TArc
} // namespace DAVA
