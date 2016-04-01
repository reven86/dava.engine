#pragma once
#include "../ReflectionDB.h"
#include "StructureWrapperDefault.h"

namespace DAVA
{
template <typename T>
class StructureWrapperDefault<T*> : public StructureWrapper
{
public:
    StructureWrapperDefault()
    {
        ptrDB = ReflectionDB::GetGlobalDB<T>();
    }

    bool IsDynamic() const override
    {
        return ptrDB->structureWrapper->IsDynamic();
    }

    bool CanAdd() const override
    {
        return ptrDB->structureWrapper->CanAdd();
    }

    bool CanInsert() const override
    {
        return ptrDB->structureWrapper->CanInsert();
    }

    bool CanRemove() const override
    {
        return ptrDB->structureWrapper->CanRemove();
    }

    Ref::Field GetField(const ReflectedObject& object, const Any& key) const override
    {
        ReflectedObject derefObj = object.GetDerefObject();

        if (!derefObj.IsValid())
            return Ref::Field();

        return ptrDB->structureWrapper->GetField(object.GetDerefObject(), key);
    }

    Ref::FieldsList GetFields(const ReflectedObject& object) const override
    {
        ReflectedObject derefObj = object.GetDerefObject();

        if (!derefObj.IsValid())
            return Ref::FieldsList();

        return ptrDB->structureWrapper->GetFields(object.GetDerefObject());
    }

    bool AddField(const ReflectedObject& object, const Any& key, const Any& value) const override
    {
        return ptrDB->structureWrapper->AddField(object.GetDerefObject(), key, value);
    }

    bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        return ptrDB->structureWrapper->InsertField(object.GetDerefObject(), key, beforeKey, value);
    }

    bool RemoveField(const ReflectedObject& object, const Any& key) const override
    {
        return ptrDB->structureWrapper->RemoveField(object.GetDerefObject(), key);
    }

protected:
    const ReflectionDB* ptrDB;
};

} // namespace DAVA
