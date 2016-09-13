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
class PropertiesHolder
{
public:
    PropertiesHolder(const String& rootPath, const FilePath& dirPath);

    ~PropertiesHolder();

    PropertiesHolder CreateSubHolder(const String& name) const;

    void Save(const String& key, const Any& value);
    void SaveToFile();

    template <typename T>
    T Load(const String& key, const T& defaultValue = T()) const;

    PropertiesHolder(const PropertiesHolder& holder) = delete;
    PropertiesHolder(PropertiesHolder&& holder);
    PropertiesHolder& operator=(const PropertiesHolder& holder) = delete;
    PropertiesHolder& operator=(PropertiesHolder&& holder);

    void SetDirectory(const FilePath& dirPath);

private:
    Any Load(const String& key, const Any& defaultValue, const Type* type) const;
    PropertiesHolder(const PropertiesHolder& parent, const String& name);
    struct Impl;
    std::unique_ptr<Impl> impl;
};

template <typename T>
T PropertiesHolder::Load(const String& key, const T& defaultValue) const
{
    Any loadedValue = Load(key, defaultValue, Type::Instance<T>());
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
