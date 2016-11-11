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
    StructureWrapperClass(const RttiType* rttiType);

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const override;

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
    AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;

private:
    Vector<const ReflectedStructure::Field*> fieldsCache;
    Vector<const ReflectedStructure::Method*> methodsCache;
    Map<String, size_t> fieldsNameIndexes;
    Map<String, size_t> methodsNameIndexes;

    void FillCache(const RttiType* rttiType);
    void FillCacheEntries(const RttiType* rttiType);
};

} // namespace DAVA
