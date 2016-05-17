#include "Base/Type.h"
#include <cassert>

namespace DAVA
{
std::unordered_map<std::string, const Type*> Type::nameToTypeMap;

const char* Type::GetPermanentName() const
{
    return permanentName.c_str();
}

const Type* Type::Instance(const std::string& permanentName)
{
    const Type* ret = nullptr;

    auto it = nameToTypeMap.find(permanentName);
    if (it != nameToTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

void Type::RegisterPermanentName(const std::string& permanentName_) const
{
    assert(permanentName.empty()); // already registered?
    assert(0 == nameToTypeMap.count(permanentName_));

    Type* t = const_cast<Type*>(this);

    t->permanentName = permanentName_;
    t->nameToTypeMap[permanentName] = this;
}

} // namespace DAVA
