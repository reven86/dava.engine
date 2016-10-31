#pragma once
#include "Reflection/Wrappers.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/StructureEditorWrapperDefault.h"

#ifdef __REFLECTION_FEATURE__

namespace DAVA
{
template <typename C>
class StructureEditorWrapperStdIndexed : public StructureEditorWrapperDefault
{
public:
    using V = typename C::value_type;

    bool CanAdd(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return !vw->IsReadonly();
    }

    bool CanInsert(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return !vw->IsReadonly();
    }

    bool CanRemove(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return !vw->IsReadonly();
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

template <typename C>
class StructureEditorWrapperStdSet : public StructureEditorWrapperDefault
{
public:
    using K = typename C::key_type;

    bool CanAdd(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return !vw->IsReadonly();
    }

    bool CanRemove(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return !vw->IsReadonly();
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        if (vw->IsReadonly())
        {
            return false;
        }

        C* c = vw->GetValueObject(object).GetPtr<C>();
        return c->insert(value.Get<K>()).second;
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

template <typename C>
class StructureEditorWrapperStdMap : public StructureEditorWrapperDefault
{
public:
    using K = typename C::key_type;
    using V = typename C::mapped_type;
    using Pair = typename C::value_type;

    bool CanAdd(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return !vw->IsReadonly();
    }

    bool CanRemove(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return !vw->IsReadonly();
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

template <typename T, size_t N>
struct StructureEditorWrapperCreator<Array<T, N>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdIndexed<Array<T, N>>();
    }
};

template <typename T>
struct StructureEditorWrapperCreator<Vector<T>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdIndexed<Vector<T>>();
    }
};

template <typename T>
struct StructureEditorWrapperCreator<List<T>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdIndexed<List<T>>();
    }
};

template <typename T, typename Eq>
struct StructureEditorWrapperCreator<Set<T, Eq>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdSet<Set<T, Eq>>();
    }
};

template <typename T, typename Hash, typename Eq>
struct StructureEditorWrapperCreator<UnorderedSet<T, Hash, Eq>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdSet<UnorderedSet<T, Hash, Eq>>();
    }
};

template <typename K, typename V, typename Eq>
struct StructureEditorWrapperCreator<Map<K, V, Eq>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdMap<Map<K, V, Eq>>();
    }
};

template <typename K, typename V, typename Eq>
struct StructureEditorWrapperCreator<MultiMap<K, V, Eq>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdMap<MultiMap<K, V, Eq>>();
    }
};

template <typename K, typename V, typename Hash, typename Eq>
struct StructureEditorWrapperCreator<UnorderedMap<K, V, Hash, Eq>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdMap<UnorderedMap<K, V, Hash, Eq>>();
    }
};

template <typename K, typename V, typename Hash, typename Eq>
struct StructureEditorWrapperCreator<UnorderedMultiMap<K, V, Hash, Eq>>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperStdMap<UnorderedMultiMap<K, V, Hash, Eq>>();
    }
};

} // namespace DAVA

#endif
