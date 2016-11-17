#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Type;
class ReflectedType;
class ReflectedTypeDB
{
    template <typename C>
    friend class ReflectionRegistrator;

public:
    static ReflectedType* Create(const Type* type, const String& permanentName);

    template <typename T>
    static const ReflectedType* Get();

    template <typename T>
    static const ReflectedType* GetByPointer(const T* ptr);

    static const ReflectedType* GetByRtType(const Type* type);
    static const ReflectedType* GetByRtTypeName(const String& rttiName);
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
    static UnorderedMap<const Type*, ReflectedType*> rtTypeToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> rtTypeNameToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> permanentNameToReflectedTypeMap;
};
} // namespace DAVA
