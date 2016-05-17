#include "Reflection/Reflection.h"
#include "Reflection/ReflectionDB.h"

namespace DAVA
{
const CtorWrapper* ReflectionDB::GetCtor() const
{
    const CtorWrapper* ret = nullptr;

    for (auto& it : ctorWrappers)
    {
        if (it->GetParamsList().size() == 0)
        {
            ret = it.get();
            break;
        }
    }

    return ret;
}

const CtorWrapper* ReflectionDB::GetCtor(const Ref::ParamsList& params) const
{
    const CtorWrapper* ret = nullptr;

    for (auto& it : ctorWrappers)
    {
        if (it->GetParamsList() == params)
        {
            ret = it.get();
            break;
        }
    }

    return ret;
}

DAVA::Vector<const CtorWrapper*> ReflectionDB::GetCtors() const
{
    DAVA::Vector<const CtorWrapper*> ret;

    ret.reserve(ctorWrappers.size());
    for (auto& it : ctorWrappers)
    {
        ret.push_back(it.get());
    }

    return ret;
}

const DtorWrapper* ReflectionDB::GetDtor() const
{
    return dtorWrapper.get();
}

} // namespace DAVA
