#include "Base/Platform.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/Wrappers.h"

namespace DAVA
{
UnorderedMap<const Type*, ReflectedType*> ReflectedType::typeToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedType::rttiNameToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedType::permanentNameToReflectedTypeMap;

void ReflectedType::SetPermanentName(const String& name) const
{
    ReflectedType* rt = const_cast<ReflectedType*>(this);

    assert(permanentName.empty() && "Name is already set");
    assert(permanentNameToReflectedTypeMap.count(name) == 0 && "Permanent name alredy in use");

    rt->permanentName = name;
    rt->permanentNameToReflectedTypeMap[permanentName] = rt;
}

const CtorWrapper* ReflectedType::GetCtor(const AnyFn::Params& params) const
{
    const CtorWrapper* ret = nullptr;

    for (auto& it : ctorWrappers)
    {
        if (it->GetInvokeParams() == params)
        {
            ret = it.get();
            break;
        }
    }

    return ret;
}

Vector<const CtorWrapper*> ReflectedType::GetCtors() const
{
    Vector<const CtorWrapper*> ret;

    ret.reserve(ctorWrappers.size());
    for (auto& it : ctorWrappers)
    {
        ret.push_back(it.get());
    }

    return ret;
}

const DtorWrapper* ReflectedType::GetDtor() const
{
    return dtorWrapper.get();
}

const ReflectedType* ReflectedType::GetByType(const Type* type)
{
    const ReflectedType* ret = nullptr;

    auto it = typeToReflectedTypeMap.find(type);
    if (it != typeToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedType::GetByRttiName(const String& name)
{
    const ReflectedType* ret = nullptr;

    auto it = rttiNameToReflectedTypeMap.find(name);
    if (it != rttiNameToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedType::GetByPermanentName(const String& name)
{
    const ReflectedType* ret = nullptr;

    auto it = permanentNameToReflectedTypeMap.find(name);
    if (it != permanentNameToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

} // namespace DAVA
