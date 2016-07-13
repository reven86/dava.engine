#include "Reflection/Private/StructureWrapperClass.h"

namespace DAVA
{
Ref::Field StructureWrapperClass::GetField(const ReflectedObject& object, const Any& key) const
{
    Ref::Field result;
    if (key.CanGet<String>())
    {
        const String& stringKey = key.Get<String>();
        Vector<ClassField>::const_iterator fieldIter = std::find_if(fields.begin(), fields.end(), [&stringKey](const ClassField& classField)
                                                                    {
                                                                        return classField.name == stringKey;
                                                                    });

        if (fieldIter != fields.end())
        {
            result.key = fieldIter->name;
            result.valueRef = Reflection(object, fieldIter->vw.get(), fieldIter->db);
        }
        else
        {
            for (const BaseClass& base : bases)
            {
                const StructureWrapper* baseChildren = base.db->structureWrapper.get();
                if (nullptr != baseChildren)
                {
                    result = baseChildren->GetField(base.castOP(object), key);
                    /// how to check: is result valid and not empty???
                    if (!result.key.IsEmpty())
                    {
                        break;
                    }
                }
            }
        }
    }

    return result;
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

    ret.reserve(ret.size() + fields.size());
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
