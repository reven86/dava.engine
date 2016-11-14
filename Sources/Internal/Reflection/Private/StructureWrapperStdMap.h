#pragma once
#include "Reflection/Wrappers.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/StructureWrapperDefault.h"

namespace DAVA
{
template <typename C>
class StructureWrapperStdMap : public StructureWrapperDefault
{
public:
    using K = typename C::key_type;
    using V = typename C::mapped_type;
    using Pair = typename C::value_type;

    StructureWrapperStdMap()
    {
        caps.add = true;
        caps.remove = true;
        caps.flat = true;
        caps.dynamic = true;
        caps.flatKeyType = RttiType::Instance<K>();
        caps.flatValueType = RttiType::Instance<V>();
    }

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        C* c = vw->GetValueObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<K>())
        {
            const K& k = key.Cast<K>();
            C* c = vw->GetValueObject(obj).GetPtr<C>();

            if (c->count(k) > 0)
            {
                V* v = &c->at(k);
                return Reflection::Create(v);
            }
        }

        return Reflection();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        Vector<Reflection::Field> ret;

        C* c = vw->GetValueObject(obj).GetPtr<C>();

        ret.reserve(c->size());
        for (auto& pair : *c)
        {
            V* v = &(pair.second);
            ret.push_back({ Any(pair.first), Reflection::Create(v) });
        }

        return ret;
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        if (vw->IsReadonly())
        {
            return false;
        }

        if (!key.CanCast<K>())
        {
            return false;
        }

        const K& k = key.Cast<K>();
        C* c = vw->GetValueObject(object).GetPtr<C>();

        return c->insert(Pair(k, value.Get<V>())).second;
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        if (vw->IsReadonly())
        {
            return false;
        }

        if (!key.CanCast<K>())
        {
            return false;
        }

        K k = key.Cast<K>();
        C* c = vw->GetValueObject(object).GetPtr<C>();

        return (c->erase(k) > 0);
    }
};

template <typename K, typename V, typename Eq>
struct StructureWrapperCreator<Map<K, V, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdMap<Map<K, V, Eq>>();
    }
};

template <typename K, typename V, typename Eq>
struct StructureWrapperCreator<MultiMap<K, V, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdMap<MultiMap<K, V, Eq>>();
    }
};

template <typename K, typename V, typename Hash, typename Eq>
struct StructureWrapperCreator<UnorderedMap<K, V, Hash, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdMap<UnorderedMap<K, V, Hash, Eq>>();
    }
};

template <typename K, typename V, typename Hash, typename Eq>
struct StructureWrapperCreator<UnorderedMultiMap<K, V, Hash, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdMap<UnorderedMultiMap<K, V, Hash, Eq>>();
    }
};
} // namespace DAVA
