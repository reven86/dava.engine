#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Private/Wrappers/StructureWrapperDefault.h"

namespace DAVA
{
class StructureWrapperClass final : public StructureWrapperDefault
{
public:
    StructureWrapperClass(const Type* type);

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const override;

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
    AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;

private:
    struct CachedFieldEntry
    {
        const ReflectedStructure::Field* field;
        const ReflectedType* owner;
    };

    struct CachedMethodEntry
    {
        const ReflectedStructure::Method* method;
        const ReflectedType* owner;
    };

    Vector<CachedFieldEntry> fieldsCache;
    Vector<CachedMethodEntry> methodsCache;
    UnorderedMap<String, size_t> fieldsNameIndexes;
    UnorderedMap<String, size_t> methodsNameIndexes;

    void FillCache(const Type* type);
    void FillCacheEntries(const Type* type);
};

} // namespace DAVA
