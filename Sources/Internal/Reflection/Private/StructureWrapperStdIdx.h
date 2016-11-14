#pragma once
#include "Reflection/Wrappers.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/StructureWrapperDefault.h"

namespace DAVA
{
template <typename C>
class StructureWrapperStdIdx : public StructureWrapperDefault
{
public:
    using V = typename C::value_type;

    StructureWrapperStdIdx()
    {
        caps.add = true;
        caps.insert = true;
        caps.remove = true;
        caps.flat = true;
        caps.dynamic = true;
        caps.flatKeyType = RttiType::Instance<size_t>();
        caps.flatValueType = RttiType::Instance<V>();
    }

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        C* c = vw->GetValueObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<size_t>())
        {
            size_t i = key.Cast<size_t>();
            C* c = vw->GetValueObject(obj).GetPtr<C>();

            auto it = std::next(c->begin(), i);
            if (it != c->end())
            {
                V* v = &(*it);
                return Reflection::Create(v);
            }
        }

        return Reflection();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        Vector<Reflection::Field> ret;

        size_t i = 0;
        C* c = vw->GetValueObject(obj).GetPtr<C>();

        ret.reserve(c->size());
        for (auto& it : *c)
        {
            V* v = &it;
            ret.push_back({ Any(i++), Reflection::Create(v) });
        }

        return ret;
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        if (vw->IsReadonly())
        {
            return false;
        }

        C* c = vw->GetValueObject(object).GetPtr<C>();
        c->push_back(value.Get<V>());

        return true;
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        if (vw->IsReadonly())
        {
            return false;
        }

        if (!beforeKey.CanCast<size_t>())
        {
            return false;
        }

        size_t i = beforeKey.Cast<size_t>();
        C* c = vw->GetValueObject(object).GetPtr<C>();

        auto it = std::next(c->begin(), i);
        c->insert(it, value.Get<V>());

        return true;
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        if (vw->IsReadonly())
        {
            return false;
        }

        if (!key.CanCast<size_t>())
        {
            return false;
        }

        size_t i = key.Cast<size_t>();
        C* c = vw->GetValueObject(object).GetPtr<C>();

        if (i >= c->size())
        {
            return false;
        }

        auto it = std::next(c->begin(), i);
        it = c->erase(it);

        return true;
    }
};

template <typename T, size_t N>
struct StructureWrapperCreator<Array<T, N>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIdx<Array<T, N>>();
    }
};

template <typename T>
struct StructureWrapperCreator<Vector<T>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIdx<Vector<T>>();
    }
};

template <typename T>
struct StructureWrapperCreator<List<T>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIdx<List<T>>();
    }
};

} // namespace DAVA
