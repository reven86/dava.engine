#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Private/ValueWrapperDefault.h"

namespace DAVA
{
inline Reflection::Reflection(const ReflectedObject& object_, const ValueWrapper* vw_, const ReflectedType* rtype_, const ReflectedMeta* meta_)
    : object(object_)
    , vw(vw_)
    , meta(meta_)
    , objectType(rtype_)
{
    if (nullptr != rtype_)
    {
        sw = rtype_->structureWrapper.get();
        sew = rtype_->structureEditorWrapper.get();
    }

    if (nullptr != meta)
    {
        if (meta->HasMeta<StructureWrapper>())
        {
            sw = meta->GetMeta<StructureWrapper>();
        }

        if (meta->HasMeta<StructureEditorWrapper>())
        {
            sew = meta->GetMeta<StructureEditorWrapper>();
        }
    }
}

inline bool Reflection::IsValid() const
{
    return (nullptr != vw && object.IsValid());
}

inline bool Reflection::IsReadonly() const
{
    return vw->IsReadonly();
}

inline const Type* Reflection::GetValueType() const
{
    return vw->GetType();
}

inline ReflectedObject Reflection::GetValueObject() const
{
    return vw->GetValueObject(object);
}

inline const DAVA::ReflectedType* Reflection::GetReflectedType() const
{
    return objectType;
}

inline Any Reflection::GetValue() const
{
    return vw->GetValue(object);
}

inline bool Reflection::SetValue(const Any& value) const
{
    return vw->SetValue(object, value);
}

inline bool Reflection::HasFields() const
{
    return sw->HasFields(object, vw);
}

inline Reflection::Field Reflection::GetField(const Any& key) const
{
    return sw->GetField(object, vw, key);
}

inline Vector<Reflection::Field> Reflection::GetFields() const
{
    return sw->GetFields(object, vw);
}

inline bool Reflection::CanAddFields() const
{
    return sew->CanAdd(object, vw);
}

inline bool Reflection::CanInsertFields() const
{
    return sew->CanInsert(object, vw);
}

inline bool Reflection::CanRemoveFields() const
{
    return sew->CanRemove(object, vw);
}

inline bool Reflection::CanCreateFieldValue() const
{
    return sew->CanCreateValue(object, vw);
}

inline Any Reflection::CreateFieldValue() const
{
    return sew->CreateValue(object, vw);
}

inline bool Reflection::AddField(const Any& key, const Any& value) const
{
    return sew->AddField(object, vw, key, value);
}

inline bool Reflection::InsertField(const Any& beforeKey, const Any& key, const Any& value) const
{
    return sew->InsertField(object, vw, beforeKey, key, value);
}

inline bool Reflection::RemoveField(const Any& key) const
{
    return sew->RemoveField(object, vw, key);
}

inline bool Reflection::HasMethods() const
{
    return sw->HasMethods(object, vw);
}

inline Reflection::Method Reflection::GetMethod(const String& key) const
{
    return sw->GetMethod(object, vw, key);
}

inline Vector<Reflection::Method> Reflection::GetMethods() const
{
    return sw->GetMethods(object, vw);
}

template <typename Meta>
inline bool Reflection::HasMeta() const
{
    return (nullptr != meta) ? meta->HasMeta<Meta>() : false;
}

template <typename Meta>
inline const Meta* Reflection::GetMeta() const
{
    return (nullptr != meta) ? meta->GetMeta<Meta>() : nullptr;
}

template <typename T>
Reflection Reflection::Create(T* ptr, const ReflectedMeta* meta)
{
    static ValueWrapperDefault<T> vw;

    if (nullptr != ptr)
    {
        const ReflectedType* rtype = ReflectedType::GetByPointer(ptr);
        return Reflection(ReflectedObject(ptr), &vw, rtype, meta);
    }

    return Reflection();
}

template <typename T>
Reflection::Field Reflection::Field::Create(const Any& key, T* ptr, const ReflectedMeta* meta)
{
    return Reflection::Field{ key, Reflection::Create(ptr, meta) };
}

} // namespace DAVA
