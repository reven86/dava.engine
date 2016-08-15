#pragma once
#include "Reflection/Public/Wrappers.h"

namespace DAVA
{
class StructureWrapperDefault : public StructureWrapper
{
public:
    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    Reflection::Field GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        return Reflection::Field();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        return Vector<Reflection::Field>();
    }

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    Reflection::Method GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        return Reflection::Method();
    }

    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return Vector<Reflection::Method>();
    }
};

class StructureEditorWrapperDefault : public StructureEditorWrapper
{
    bool CanAdd() const override
    {
        return false;
    }

    bool CanInsert() const override
    {
        return false;
    }

    bool CanRemove() const override
    {
        return false;
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        return false;
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        return false;
    }
};

template <typename T>
struct StructureWrapperCreator
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperDefault();
    }
};

template <typename T>
struct StructureEditorWrapperCreator
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperDefault();
    }
};

} // namespace DAVA
