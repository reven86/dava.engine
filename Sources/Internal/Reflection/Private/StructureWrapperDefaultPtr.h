#pragma once
#include "Reflection/ReflectionDB.h"
#include "Reflection/Private/StructureWrapperDefault.h"

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

        T* ptr = derefObj.GetPtr<T>();

        const ReflectionDB* db = ReflectionDB::GetGlobalDB(ptr);
        return db->structureWrapper->GetField(derefObj, key);
    }

    Ref::FieldsList GetFields(const ReflectedObject& object) const override
    {
        ReflectedObject derefObj = object.GetDerefObject();

        if (!derefObj.IsValid())
            return Ref::FieldsList();

        T* ptr = derefObj.GetPtr<T>();

        const ReflectionDB* db = ReflectionDB::GetGlobalDB(ptr);
        return db->structureWrapper->GetFields(derefObj);
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
