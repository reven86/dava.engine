#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/ReflectedBase.h"
#include "Reflection/ReflectedType.h"

namespace DAVA
{
class ReflectedTypeDB
{
    template <typename C>
    friend class ReflectionRegistrator;

public:
    template <typename T>
    static const ReflectedType* Get();

    template <typename T>
    static const ReflectedType* GetByPointer(const T* ptr);

    static const ReflectedType* GetByRttiType(const RttiType* type);
    static const ReflectedType* GetByRttiName(const String& name);
    static const ReflectedType* GetByPermanentName(const String& name);

protected:
    template <typename T>
    static ReflectedType* Create();

    template <typename T>
    static ReflectedType* Edit();

    static UnorderedMap<const RttiType*, ReflectedType*> rttiTypeToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> permanentNameToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> rttiNameToReflectedTypeMap;
};
} // namespace DAVA

#define __DAVA_ReflectedTypeDB__
#include "Reflection/Private/ReflectedTypeDB_impl.h"
