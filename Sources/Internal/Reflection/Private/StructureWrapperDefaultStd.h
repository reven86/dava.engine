#pragma once
#include "StructureWrapperDefault.h"

namespace DAVA
{
template <typename T>
class StructureWrapperDefault<std::vector<T>> : public StructureWrapper
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
        return false;
    }
    bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        return false;
    }
    bool RemoveField(const ReflectedObject& object, const Any& key) const override
    {
        return false;
    }

    Ref::Field GetField(const ReflectedObject& object, const Any& key) const override
    {
        Ref::Field child;

        if (key.CanCast<size_t>())
        {
            size_t i = key.Cast<size_t>();
            std::vector<T>* vector = object.GetPtr<std::vector<T>>();

            if (i < vector->size())
            {
                T* valuePtr = &vector->at(i);
                child.key = key;
                child.valueRef = Reflection::Reflect(valuePtr);
            }
        }

        return child;
    }

    Ref::FieldsList GetFields(const ReflectedObject& object) const override
    {
        Ref::FieldsList ret;
        std::vector<T>* vector = object.GetPtr<std::vector<T>>();

        for (size_t i = 0; i < vector->size(); ++i)
        {
            T* valuePtr = &vector->at(i);

            Ref::Field child;
            child.key = i;
            child.valueRef = Reflection::Reflect(valuePtr);
            ret.emplace_back(std::move(child));
        }

        return ret;
    }
};

} // namespace DAVA
