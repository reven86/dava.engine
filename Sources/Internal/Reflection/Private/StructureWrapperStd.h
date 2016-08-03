#pragma once
#include "Reflection/Public/Wrappers.h"
#include "Reflection/Private/ValueWrapperDefault.h"

namespace DAVA
{
template <typename C>
class StructureWrapperStdIndexed : public StructureWrapperDefault
{
public:
    using V = typename C::value_type;

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        C* c = vw->GetValueObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    Reflection::Field GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        Reflection::Field ret;
        if (key.CanCast<size_t>())
        {
            size_t i = key.Cast<size_t>();
            C* c = vw->GetValueObject(obj).GetPtr<C>();

            auto it = std::next(c->begin(), i);
            if (it != c->end())
            {
                const V* v = &(*it);
                ret = Reflection::Create(v, key);
            }
        }

        return ret;
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        Vector<Reflection::Field> ret;

        size_t i = 0;
        C* c = vw->GetValueObject(obj).GetPtr<C>();

        ret.reserve(c->size());
        for (auto& it : *c)
        {
            const V* v = &it;
            ret.emplace_back(Reflection::Create(v, Any(i++)));
        }

        return ret;
    }
};

template <typename C>
class StructureWrapperStdKeyed : public StructureWrapperDefault
{
public:
    using K = typename C::key_type;
    using V = typename C::mapped_type;

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        C* c = vw->GetValueObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    Reflection::Field GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        Reflection::Field ret;
        if (key.CanCast<K>())
        {
            K k = key.Cast<K>();
            C* c = vw->GetValueObject(obj).GetPtr<C>();

            if (c->count(k) > 0)
            {
                const V* v = &c->at(k);
                ret = Reflection::Create(v, key);
            }
        }

        return ret;
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        Vector<Reflection::Field> ret;

        C* c = vw->GetValueObject(obj).GetPtr<C>();

        ret.reserve(c->size());
        for (auto& it : *c)
        {
            const V* v = &(it.second);
            ret.emplace_back(Reflection::Create(v, Any(it.first)));
        }

        return ret;
    }
};

template <typename T, size_t N>
struct StructureWrapperCreator<Array<T, N>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIndexed<Array<T, N>>();
    }
};

template <typename T>
struct StructureWrapperCreator<Vector<T>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIndexed<Vector<T>>();
    }
};

template <typename T>
struct StructureWrapperCreator<List<T>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIndexed<List<T>>();
    }
};

template <typename T, typename Eq>
struct StructureWrapperCreator<Set<T, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIndexed<Set<T, Eq>>();
    }
};

template <typename T, typename Hash, typename Eq>
struct StructureWrapperCreator<UnorderedSet<T, Hash, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIndexed<UnorderedSet<T, Hash, Eq>>();
    }
};

template <typename K, typename V, typename Eq>
struct StructureWrapperCreator<Map<K, V, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdKeyed<Map<K, V, Eq>>();
    }
};

template <typename K, typename V, typename Eq>
struct StructureWrapperCreator<MultiMap<K, V, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdKeyed<MultiMap<K, V, Eq>>();
    }
};

template <typename K, typename V, typename Hash, typename Eq>
struct StructureWrapperCreator<UnorderedMap<K, V, Hash, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdKeyed<UnorderedMap<K, V, Hash, Eq>>();
    }
};

template <typename K, typename V, typename Hash, typename Eq>
struct StructureWrapperCreator<UnorderedMultiMap<K, V, Hash, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdKeyed<UnorderedMultiMap<K, V, Hash, Eq>>();
    }
};

} // namespace DAVA
