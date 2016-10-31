#pragma once
#include "Reflection/Wrappers.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/StructureEditorWrapperDefault.h"

#ifdef __REFLECTION_FEATURE__

namespace DAVA
{
template <typename T>
class StructureEditorWrapperPtr final : public StructureEditorWrapperDefault
{
public:
    StructureEditorWrapperPtr() = default;

    bool CanCreateValue(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(object).Deref();
        const StructureEditorWrapper* sew = GetInternalWrapper(derefObj);
        if (nullptr != sew)
        {
            return sew->CanCreateValue(derefObj, &ptrVW);
        }

        return StructureEditorWrapperDefault::CanCreateValue(object, vw);
    }

    bool CanAdd(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(object).Deref();
        const StructureEditorWrapper* sew = GetInternalWrapper(derefObj);
        if (nullptr != sew)
        {
            return sew->CanAdd(derefObj, &ptrVW);
        }

        return StructureEditorWrapperDefault::CanCreateValue(object, vw);
    }

    bool CanInsert(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(object).Deref();
        const StructureEditorWrapper* sew = GetInternalWrapper(derefObj);
        if (nullptr != sew)
        {
            return sew->CanInsert(derefObj, &ptrVW);
        }

        return StructureEditorWrapperDefault::CanCreateValue(object, vw);
    }

    bool CanRemove(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(object).Deref();
        const StructureEditorWrapper* sew = GetInternalWrapper(derefObj);
        if (nullptr != sew)
        {
            return sew->CanRemove(derefObj, &ptrVW);
        }

        return StructureEditorWrapperDefault::CanCreateValue(object, vw);
    }

    Any CreateValue(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(object).Deref();
        const StructureEditorWrapper* sew = GetInternalWrapper(derefObj);
        if (nullptr != sew)
        {
            return sew->CreateValue(derefObj, &ptrVW);
        }

        return StructureEditorWrapperDefault::CreateValue(object, vw);
    }

    bool AddField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& key, const Any& value) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(object).Deref();
        const StructureEditorWrapper* sew = GetInternalWrapper(derefObj);
        if (nullptr != sew)
        {
            return sew->AddField(derefObj, &ptrVW, key, value);
        }

        return StructureEditorWrapperDefault::AddField(object, vw, key, value);
    }

    bool InsertField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(object).Deref();
        const StructureEditorWrapper* sew = GetInternalWrapper(derefObj);
        if (nullptr != sew)
        {
            return sew->InsertField(derefObj, &ptrVW, beforeKey, key, value);
        }

        return StructureEditorWrapperDefault::InsertField(object, vw, beforeKey, key, value);
    }

    bool RemoveField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& key) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(object).Deref();
        const StructureEditorWrapper* sew = GetInternalWrapper(derefObj);
        if (nullptr != sew)
        {
            return sew->RemoveField(derefObj, &ptrVW, key);
        }

        return StructureEditorWrapperDefault::RemoveField(object, vw, key);
    }

protected:
    ValueWrapperDefault<T*> ptrVW;

    const StructureEditorWrapper* GetInternalWrapper(const ReflectedObject& derefObj) const
    {
        const StructureEditorWrapper* sew = nullptr;

        if (derefObj.IsValid())
        {
            T* ptr = derefObj.GetPtr<T>();
            sew = ReflectedType::GetByPointer(ptr)->structureEditorWrapper.get();
        }

        return sew;
    }
};

template <typename T>
struct StructureEditorWrapperCreator<T*>
{
    static StructureEditorWrapper* Create()
    {
        return new StructureEditorWrapperPtr<T>();
    }
};

} // namespace DAVA

#endif
