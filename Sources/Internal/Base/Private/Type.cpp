#include "Base/Type.h"
#include <cassert>

namespace DAVA
{
UnorderedMap<String, const Type*> Type::nameToTypeMap;

const char* Type::GetPermanentName() const
{
    return permanentName.c_str();
}

const Type* Type::Instance(const String& permanentName)
{
    const Type* ret = nullptr;

    auto it = nameToTypeMap.find(permanentName);
    if (it != nameToTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

void Type::RegisterPermanentName(const String& permanentName_) const
{
    assert(permanentName.empty()); // already registered?
    assert(0 == nameToTypeMap.count(permanentName_));

    permanentName = permanentName_;
    nameToTypeMap[permanentName] = this;
}

} // namespace DAVA
