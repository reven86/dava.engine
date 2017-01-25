#ifndef __Dava_ReflectedTypeDB__
#define __Dava_ReflectedTypeDB__

#include "Base/BaseTypes.h"

namespace DAVA
{
class Type;
class ReflectedType;

/**
    \ingroup reflection
    ReflectedType database.

    Holds info about type static structure `ReflectedStructure` and runtime structure wrapper `StructureWrapper`.
*/
class ReflectedTypeDB
{
    template <typename C>
    friend class ReflectionRegistrator;

public:
    template <typename T>
    static const ReflectedType* Get();

    template <typename T>
    static const ReflectedType* GetByPointer(const T* ptr);

    static const ReflectedType* GetByType(const Type* type);
    static const ReflectedType* GetByTypeName(const String& rttiName);
    static const ReflectedType* GetByPermanentName(const String& permanentName);

    static ReflectedType* CreateCustomType(const Type* type, const String& permanentName);

    template <typename T, typename... Bases>
    static void RegisterBases();

    static void RegisterPermanentName(const ReflectedType* reflectedType, const String& permanentName);

protected:
    template <typename T>
    static ReflectedType* CreateStatic();

    template <typename T>
    static ReflectedType* Edit();

    static List<std::unique_ptr<ReflectedType>> customReflectedTypes;
    static UnorderedMap<const Type*, ReflectedType*> typeToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> typeNameToReflectedTypeMap;
    static UnorderedMap<String, ReflectedType*> permanentNameToReflectedTypeMap;
};
} // namespace DAVA

#endif // __Dava_ReflectedTypeDB__

#ifndef __Dava_ReflectedTypeDB_Fwd__
#include "Reflection/Private/ReflectedTypeDB_impl.h"
#endif // __Dava_ReflectedTypeDB_Fwd__
