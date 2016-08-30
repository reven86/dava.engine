#pragma once

#include <memory>

#include "Base/BaseTypes.h"

namespace DAVA
{
class Any;

namespace TArc
{
class PropertiesHolder
{
public:
    PropertiesHolder(const String &rootPath);

    ~PropertiesHolder();

    PropertiesHolder SubHolder(const String &name) const;

    void Save(const Any &value, const String &key);

    Any Load(const String &key, const Any& defaultValue) const;
    Any Load(const String &key) const;

    PropertiesHolder(const PropertiesHolder &holder) = delete;
    PropertiesHolder(PropertiesHolder &&holder);
    PropertiesHolder& operator =(const PropertiesHolder &holder) = delete;
    PropertiesHolder& operator = (PropertiesHolder &&holder);

private:
    PropertiesHolder(const PropertiesHolder &parent, const String &name);

    struct Impl;
    std::unique_ptr<Impl> impl;
};
} // namespace TArc
} // namespace DAVA
