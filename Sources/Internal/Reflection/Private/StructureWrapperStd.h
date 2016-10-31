#pragma once
#include "Reflection/Wrappers.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/StructureWrapperDefault.h"

namespace DAVA
{
template <typename C>
class StructureWrapperStdIndexed : public StructureWrapperDefault
{
public:
    using V = typename C::value_type;

    bool HasFields(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        C* c = vw->GetPropertieObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    Reflection::Field GetField(const ReflectedObject& obj, const PropertieWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<size_t>())
        {
            size_t i = key.Cast<size_t>();
            C* c = vw->GetPropertieObject(obj).GetPtr<C>();

            auto it = std::next(c->begin(), i);
            if (it != c->end())
            {
                V* v = &(*it);
                return Reflection::Field::Create(key, v);
            }
        }

        return Reflection::Field();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const PropertieWrapper* vw) const override
    {
        Vector<Reflection::Field> ret;

        size_t i = 0;
        C* c = vw->GetPropertieObject(obj).GetPtr<C>();

        ret.reserve(c->size());
        for (auto& it : *c)
        {
            V* v = &it;
            ret.emplace_back(Reflection::Field::Create(Any(i++), v));
        }

        return ret;
    }
};

template <typename C>
class StructureWrapperStdSet : public StructureWrapperDefault
{
public:
    using K = typename C::key_type;

    bool HasFields(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        C* c = vw->GetPropertieObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    Reflection::Field GetField(const ReflectedObject& obj, const PropertieWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<K>())
        {
            const K& k = key.Cast<K>();
            C* c = vw->GetPropertieObject(obj).GetPtr<C>();

            auto it = c->find(k);
            if (it != c->end())
            {
                // std::set values shouldn't be modified
                // so get const pointer on value
                const K* v = &(*it);
                return Reflection::Field::Create(key, v);
            }
        }

        return Reflection::Field();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const PropertieWrapper* vw) const override
    {
        Vector<Reflection::Field> ret;

        C* c = vw->GetPropertieObject(obj).GetPtr<C>();

        ret.reserve(c->size());

        auto end = c->end();
        for (auto it = c->begin(); it != end; it++)
        {
            // std::set values shouldn't be modified
            // so get const pointer on value
            const K* v = &(*it);
            ret.emplace_back(Reflection::Field::Create(Any(*v), v));
        }

        return ret;
    }
};

template <typename C>
class StructureWrapperStdMap : public StructureWrapperDefault
{
public:
    using K = typename C::key_type;
    using V = typename C::mapped_type;

    bool HasFields(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        C* c = vw->GetPropertieObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    Reflection::Field GetField(const ReflectedObject& obj, const PropertieWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<K>())
        {
            const K& k = key.Cast<K>();
            C* c = vw->GetPropertieObject(obj).GetPtr<C>();

            if (c->count(k) > 0)
            {
                V* v = &c->at(k);
                return Reflection::Field::Create(key, v);
            }
        }

        return Reflection::Field();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const PropertieWrapper* vw) const override
    {
        Vector<Reflection::Field> ret;

        C* c = vw->GetPropertieObject(obj).GetPtr<C>();

        ret.reserve(c->size());
        for (auto& pair : *c)
        {
            V* v = &(pair.second);
            ret.emplace_back(Reflection::Field::Create(Any(pair.first), v));
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
        return new StructureWrapperStdSet<Set<T, Eq>>();
    }
};

template <typename T, typename Hash, typename Eq>
struct StructureWrapperCreator<UnorderedSet<T, Hash, Eq>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdSet<UnorderedSet<T, Hash, Eq>>();
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
