#pragma once

#include <memory>

namespace DAVA
{
class String;
class Any;

namespace TArc
{
class PropertiesHolder final
{
    //constructs a root element, which may contain any children and will be stored to the separate file.
    PropertiesHolder(const DAVA::String &projectName);

    //constructs child of root element, which will be stored to the same file as root parent.
    PropertiesHolder(const PropertiesHolder &parent, &path);

    void Save(const Any &value, const DAVA::String &key);

    Any Load(const DAVA::String &key) const;

    struct Impl;
    std::unique_ptr<Impl> impl;
};
}
}
