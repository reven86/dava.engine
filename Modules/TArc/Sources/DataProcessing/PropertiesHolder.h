#pragma once

#include <memory>

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
namespace TArc
{
class PropertiesHolder
{
public:
    PropertiesHolder(const String &rootPath, const FilePath &directory);

    ~PropertiesHolder();

    PropertiesHolder SubHolder(const String &name) const;

    void Save(const Any &value, const String &key);

    Any Load(const String &key, const Any& defaultValue) const;

    template <typename T>
    T Load(const String &key) const;

    PropertiesHolder(const PropertiesHolder &holder) = delete;
    PropertiesHolder(PropertiesHolder &&holder);
    PropertiesHolder& operator =(const PropertiesHolder &holder) = delete;
    PropertiesHolder& operator = (PropertiesHolder &&holder);

    void SetDirectory(const FilePath& filePath);

private:
    PropertiesHolder(const PropertiesHolder &parent, const String &name);
    struct Impl;
    std::unique_ptr<Impl> impl;
};

template <typename T>
T PropertiesHolder::Load(const String &key) const
{
    return Load(key, T()).Get<T>(T());
}

} // namespace TArc
} // namespace DAVA
