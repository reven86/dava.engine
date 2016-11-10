#pragma once

#ifndef __DAVA_Reflection__
#error Don't include this file directly, instead use #include "Reflection/Reflection.h"
#else

#include "Base/BaseTypes.h"
#include "Base/RttiType.h"
#include "Base/RttiInheritance.h"
#include "Reflection/ReflectedBase.h"
#include "Reflection/ReflectedType.h"

namespace DAVA
{
class ReflectedTypeDB
{
    template <typename C>
    friend class ReflectionQualifier;

public:
    static ReflectedType* Create(const RttiType* rttiType, const String& permanentName);

    template <typename T>
    static const ReflectedType* Get();

    template <typename T>
    static const ReflectedType* GetByPointer(const T* ptr);

    static const ReflectedType* GetByRttiType(const RttiType* rttiType);
    static const ReflectedType* GetByRttiName(const String& rttiName);
    static const ReflectedType* GetByPermanentName(const String& permanentName);

    template <typename T, typename... Bases>
    static void RegisterBases();

    static void RegisterPermanentName(const ReflectedType* reflectedType, const String& permanentName);

protected:
    template <typename T>
    static ReflectedType* CreateStatic();

    template <typename T>
    static ReflectedType* Edit();

    static List<std::unique_ptr<ReflectedType>> customReflectedTypes;
    static UnorderedMap<const RttiType*, ReflectedType*> rttiTypeToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> rttiNameToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> permanentNameToReflectedTypeMap;
};
} // namespace DAVA

#define __DAVA_ReflectedTypeDB__
#include "Reflection/Private/ReflectedTypeDB_impl.h"
#endif
