#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Private/Wrappers/StructureWrapperDefault.h"
#include "Reflection/Private/Wrappers/ValueWrapperDefault.h"

namespace DAVA
{
namespace StructureWrapperDetail
{
enum class IsDynamic
{
    YES,
    NO
};

enum class HasRange
{
    YES,
    NO
};
} // namespace StructureWrapperDetail

template <typename C>
class StructureWrapperStdIdx : public StructureWrapperDefault
{
public:
    using V = typename C::value_type;

    StructureWrapperStdIdx(StructureWrapperDetail::IsDynamic isDynamic, StructureWrapperDetail::HasRange hasRange)
    {
        caps.canAddField = true;
        caps.canInsertField = true;
        caps.canRemoveField = true;
        caps.hasFlatStruct = true;
        caps.hasDynamicStruct = (isDynamic == StructureWrapperDetail::IsDynamic::YES);
        caps.hasRangeAccess = (hasRange == StructureWrapperDetail::HasRange::YES);
        caps.flatKeysType = Type::Instance<size_t>();
        caps.flatValuesType = Type::Instance<V>();
    }

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        if (!caps.hasRangeAccess)
            return 0;

        C* c = vw->GetValueObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    size_t GetFieldsCount(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        C* c = vw->GetValueObject(object).GetPtr<C>();
        return c->size();
    }

    Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<size_t>())
        {
            C* c = vw->GetValueObject(obj).GetPtr<C>();

            size_t i = key.Cast<size_t>();
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

        C* c = vw->GetValueObject(obj).GetPtr<C>();

        size_t key = 0;
        ret.reserve(c->size());
        for (auto& it : *c)
        {
            V* v = &it;
            ret.emplace_back(Any(key++), Reflection::Create(v), nullptr);
        }

        return ret;
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw, size_t first, size_t count) const override
    {
        Vector<Reflection::Field> ret;

        if (caps.hasRangeAccess)
        {
            C* c = vw->GetValueObject(obj).GetPtr<C>();
            size_t sz = c->size();

            DVASSERT(first < sz);
            DVASSERT(first + count <= sz);

            if (first < sz)
            {
                size_t n = std::min(first + count, sz);

                ret.reserve(n);
                for (size_t i = first; i < n; ++i)
                {
                    V* v = &(*c)[i];
                    ret.emplace_back(Any(i++), Reflection::Create(v), nullptr);
                }
            }
        }

        return ret;
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        if (vw->IsReadonly(object))
        {
            return false;
        }

        C* c = vw->GetValueObject(object).GetPtr<C>();
        c->push_back(value.Get<V>());

        return true;
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        if (vw->IsReadonly(object))
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
        if (vw->IsReadonly(object))
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
        return new StructureWrapperStdIdx<Array<T, N>>(StructureWrapperDetail::IsDynamic::NO, StructureWrapperDetail::HasRange::YES);
    }
};

template <typename T>
struct StructureWrapperCreator<Vector<T>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIdx<Vector<T>>(StructureWrapperDetail::IsDynamic::YES, StructureWrapperDetail::HasRange::YES);
    }
};

template <typename T>
struct StructureWrapperCreator<List<T>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIdx<List<T>>(StructureWrapperDetail::IsDynamic::NO, StructureWrapperDetail::HasRange::NO);
    }
};

} // namespace DAVA
