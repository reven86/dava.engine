#pragma once
#include "Reflection/Wrappers.h"

#ifdef __REFLECTION_FEATURE__

namespace DAVA
{
class StructureEditorWrapperDefault : public StructureEditorWrapper
{
public:
    bool CanCreateValue(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        return false;
    }

    bool CanAdd(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        return false;
    }

    bool CanInsert(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        return false;
    }

    bool CanRemove(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        return false;
    }

    Any CreateValue(const ReflectedObject& object, const PropertieWrapper* vw) const override
    {
        return Any();
    }

    bool AddField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool InsertField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool RemoveField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& key) const override
    {
        return false;
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

#endif
