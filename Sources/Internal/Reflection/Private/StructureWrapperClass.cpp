#include "StructureWrapperClass.h"

namespace DAVA
{
Ref::Field StructureWrapperClass::GetField(const ReflectedObject& object, const Any& key) const
{
    // TODO:
    // ...

    return Ref::Field();
}

Ref::FieldsList StructureWrapperClass::GetFields(const ReflectedObject& object) const
{
    Ref::FieldsList ret;

    for (auto& base : bases)
    {
        const StructureWrapper* baseChildren = base.db->structureWrapper.get();
        if (nullptr != baseChildren)
        {
            ReflectedObject b_obj = base.castOP(object);
            Ref::FieldsList b_ret = baseChildren->GetFields(b_obj);

            ret.reserve(ret.size() + b_ret.size());
            std::move(b_ret.begin(), b_ret.end(), std::inserter(ret, ret.end()));
        }
    }

    for (auto& field : fields)
    {
        Ref::Field child;
        child.key = field.name;
        child.valueRef = Reflection(object, field.vw.get(), field.db);
        ret.emplace_back(std::move(child));
    }

    return ret;
}

} // namespace DAVA
