#pragma once
#include "Reflection/Private/StructureWrapperDefault.h"

#if !defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
template <typename T>
class StructureWrapperDefault<Vector<T>> : public StructureWrapper
{
public:
    bool IsDynamic() const override
    {
        return true;
    }

    bool CanAdd() const override
    {
        return true;
    }
    bool CanInsert() const override
    {
        return true;
    }
    bool CanRemove() const override
    {
        return true;
    }

    bool AddField(const ReflectedObject& object, const Any& key, const Any& value) const override
    {
        if (value.CanCast<T>())
        {
            Vector<T>* vector = object.GetPtr<Vector<T>>();
            vector->push_back(value.Cast<T>());
            return true;
        }
        return false;
    }

    bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        if (value.CanCast<T>() && beforeKey.CanCast<size_t>())
        {
            size_t beforeIndex = beforeKey.Cast<size_t>();
            Vector<T>* vector = object.GetPtr<Vector<T>>();
            if (beforeIndex < vector->size())
            {
                vector->insert(vector->begin() + beforeIndex, value.Cast<T>());
                return true;
            }
        }
        return false;
    }

    bool RemoveField(const ReflectedObject& object, const Any& key) const override
    {
        if (key.CanCast<size_t>())
        {
            size_t index = key.Cast<size_t>();
            Vector<T>* vector = object.GetPtr<Vector<T>>();
            vector->erase(vector->begin() + index);
            return true;
        }
        return false;
    }

    Ref::Field GetField(const ReflectedObject& object, const Any& key) const override
    {
        Ref::Field child;

        if (key.CanCast<size_t>())
        {
            size_t i = key.Cast<size_t>();
            Vector<T>* vector = object.GetPtr<Vector<T>>();

            if (i < vector->size())
            {
                T* valuePtr = &((*vector)[i]);
                child.key = key;
                child.valueRef = Reflection::Reflect(valuePtr);
            }
        }

        return child;
    }

    Ref::FieldsList GetFields(const ReflectedObject& object) const override
    {
        Vector<T>* vector = object.GetPtr<Vector<T>>();

        Ref::FieldsList ret;
        ret.reserve(vector->size());
        for (size_t i = 0; i < vector->size(); ++i)
        {
            T* valuePtr = &((*vector)[i]);

            Ref::Field child;
            child.key = i;
            child.valueRef = Reflection::Reflect(valuePtr);
            ret.emplace_back(std::move(child));
        }

        return ret;
    }
};

template <typename T>
class StructureWrapperDefault<List<T>> : public StructureWrapper
{
public:
    bool IsDynamic() const override
    {
        return true;
    }

    bool CanAdd() const override
    {
        return true;
    }
    bool CanInsert() const override
    {
        return true;
    }
    bool CanRemove() const override
    {
        return true;
    }
    bool AddField(const ReflectedObject& object, const Any& key, const Any& value) const override
    {
        if (value.CanCast<T>())
        {
            List<T>* list = object.GetPtr<List<T>>();
            list->push_back(value.Cast<T>());
            return true;
        }
        return false;
    }
    bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        if (value.CanCast<T>() && beforeKey.CanCast<size_t>())
        {
            size_t beforeIndex = beforeKey.Cast<size_t>();
            List<T>* list = object.GetPtr<List<T>>();
            if (beforeIndex < list->size())
            {
                list->insert(std::next(list->begin(), beforeIndex), value.Cast<T>());
                return true;
            }
        }
        return false;
    }
    bool RemoveField(const ReflectedObject& object, const Any& key) const override
    {
        if (key.CanCast<size_t>())
        {
            size_t index = key.Cast<size_t>();
            List<T>* list = object.GetPtr<List<T>>();
            list->erase(std::next(list->begin(), index));
            return true;
        }
        return false;
    }

    Ref::Field GetField(const ReflectedObject& object, const Any& key) const override
    {
        Ref::Field child;

        if (key.CanCast<size_t>())
        {
            size_t i = key.Cast<size_t>();
            List<T>* list = object.GetPtr<List<T>>();

            if (i < list->size())
            {
                T* valuePtr = &(*std::next(list->begin(), i));
                child.key = key;
                child.valueRef = Reflection::Reflect(valuePtr);
            }
        }

        return child;
    }

    Ref::FieldsList GetFields(const ReflectedObject& object) const override
    {
        Ref::FieldsList ret;
        List<T>* list = object.GetPtr<List<T>>();

        for (auto iter = list->begin(); iter != list->end(); ++iter)
        {
            T* valuePtr = &(*iter);

            Ref::Field child;
            child.key = std::distance(list->begin(), iter);
            child.valueRef = Reflection::Reflect(valuePtr);
            ret.emplace_back(std::move(child));
        }

        return ret;
    }
};

template <typename TMap>
class StructureWrapperMap : public StructureWrapper
{
    using K = typename TMap::key_type;
    using T = typename TMap::mapped_type;

public:
    bool IsDynamic() const override
    {
        return true;
    }

    bool CanAdd() const override
    {
        return true;
    }
    bool CanInsert() const override
    {
        return true;
    }
    bool CanRemove() const override
    {
        return true;
    }
    bool AddField(const ReflectedObject& object, const Any& key, const Any& value) const override
    {
        if (key.CanCast<K>() && value.CanCast<T>())
        {
            TMap* map = object.GetPtr<TMap>();
            return map->emplace(key.Cast<K>(), value.Cast<T>()).second;
        }
        return false;
    }
    bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        return AddField(object, key, value);
    }
    bool RemoveField(const ReflectedObject& object, const Any& key) const override
    {
        if (key.CanCast<K>())
        {
            TMap* map = object.GetPtr<TMap>();
            return map->erase(key.Cast<K>()) > 0;
        }
        return false;
    }

    Ref::Field GetField(const ReflectedObject& object, const Any& key) const override
    {
        Ref::Field child;

        if (key.CanCast<K>())
        {
            TMap* map = object.GetPtr<TMap>();

            TMap::iterator iter = map->find(key.Cast<K>());
            if (iter != map->end())
            {
                T* valuePtr = &(iter->second);

                child.key = key;
                child.valueRef = Reflection::Reflect(valuePtr);
            }
        }

        return child;
    }

    Ref::FieldsList GetFields(const ReflectedObject& object) const override
    {
        Ref::FieldsList ret;
        TMap* map = object.GetPtr<TMap>();

        for (auto iter = map->begin(); iter != map->end(); ++iter)
        {
            T* valuePtr = &(iter->second);

            Ref::Field child;
            child.key = iter->first;
            child.valueRef = Reflection::Reflect(valuePtr);
            ret.emplace_back(std::move(child));
        }

        return ret;
    }
};

// specification for Set containers
template <typename TSet>
class StructureWrapperSet : public StructureWrapper
{
    using T = typename TSet::value_type;

public:
    bool IsDynamic() const override
    {
        return true;
    }
    bool CanAdd() const override
    {
        return true;
    }
    bool CanInsert() const override
    {
        return true;
    }
    bool CanRemove() const override
    {
        return true;
    }
    bool AddField(const ReflectedObject& object, const Any& key, const Any& value) const override
    {
        if (value.CanCast<T>())
        {
            TSet* set = object.GetPtr<TSet>();
            return set->insert(value.Cast<T>()).second;
        }
        return false;
    }
    bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        return AddField(object, key, value);
    }
    bool RemoveField(const ReflectedObject& object, const Any& key) const override
    {
        if (key.CanCast<T>())
        {
            TSet* set = object.GetPtr<TSet>();
            return set->erase(key.Cast<T>()) > 0;
        }
        return false;
    }

    Ref::Field GetField(const ReflectedObject& object, const Any& key) const override
    {
        Ref::Field child;

        if (key.CanCast<T>())
        {
            TSet* set = object.GetPtr<TSet>();

            typename TSet::iterator iter = set->find(key.Cast<T>());
            if (iter != set->end())
            {
                //T& valueRef = *iter;
                const T* valuePtr = &(*iter);

                child.key = key;
                child.valueRef = Reflection::Reflect(valuePtr);
            }
        }

        return child;
    }

    Ref::FieldsList GetFields(const ReflectedObject& object) const override
    {
        Ref::FieldsList ret;
        TSet* set = object.GetPtr<TSet>();

        for (TSet::iterator iter = set->begin(); iter != set->end(); ++iter)
        {
            const T* valuePtr = &(*iter);

            Ref::Field child;
            child.key = Any(*iter);
            child.valueRef = Reflection::Reflect(valuePtr);
            ret.emplace_back(std::move(child));
        }

        return ret;
    }
};

template <typename K, typename T, typename P>
struct StructureWrapperCreator<Map<K, T, P>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperMap<Map<K, T, P>>();
    }
};

template <typename K, typename T, typename H, typename P>
struct StructureWrapperCreator<UnorderedMap<K, T, H, P>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperMap<UnorderedMap<K, T, H, P>>();
    }
};

template <typename T, typename C>
struct StructureWrapperCreator<Set<T, C>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperSet<Set<T, C>>();
    }
};

template <typename T, typename H, typename C>
struct StructureWrapperCreator<UnorderedSet<T, H, C>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperSet<UnorderedSet<T, H, C>>();
    }
};

} // namespace DAVA

#endif