#include "../Reflection.h"

namespace DAVA
{
const CtorWrapper* Reflection::GetCtor() const
{
    const CtorWrapper* ret = nullptr;

    for (auto& it : db->ctorWrappers)
    {
        if (it->GetParamsList().size() == 0)
        {
            ret = it.get();
            break;
        }
    }

    return ret;
}

const CtorWrapper* Reflection::GetCtor(const Ref::ParamsList& params) const
{
    const CtorWrapper* ret = nullptr;

    for (auto& it : db->ctorWrappers)
    {
        if (it->GetParamsList() == params)
        {
            ret = it.get();
            break;
        }
    }

    return ret;
}

std::vector<const CtorWrapper*> Reflection::GetCtors() const
{
    std::vector<const CtorWrapper*> ret;

    ret.reserve(db->ctorWrappers.size());
    for (auto& it : db->ctorWrappers)
    {
        ret.push_back(it.get());
    }

    return ret;
}

} // namespace DAVA
