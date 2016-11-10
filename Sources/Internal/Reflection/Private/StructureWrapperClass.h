#pragma once

#include "Base/RttiType.h"
#include "Base/RttiInheritance.h"
#include "Reflection/ReflectedStructure.h"
#include "Reflection/Private/StructureWrapperDefault.h"

namespace DAVA
{
class StructureWrapperClass final : public StructureWrapperDefault
{
public:
    StructureWrapperClass(const ReflectedType* reflectedType);

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const override;

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
    AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;

private:
    struct FieldCacheEntry
    {
        const ReflectedStructure::Field* field;
        RttiInheritance::CastOP castToBaseOP;
    };

    struct MethodCacheEntry
    {
        const ReflectedStructure::Method* method;
    };

    Vector<FieldCacheEntry> fieldsCache;
    Vector<MethodCacheEntry> methodsCache;
    Map<String, size_t> fieldsNameIndexes;
    Map<String, size_t> methodsNameIndexes;

    void FillCache(const ReflectedType* reflectedType, RttiInheritance::CastOP castOP);
    Reflection CreateFieldReflection(const ReflectedObject& object, const ValueWrapper* vw, const FieldCacheEntry& entry) const;
};

} // namespace DAVA
