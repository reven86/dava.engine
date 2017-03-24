#include "Reflection/ReflectedTypeDB.h"
#include "Reflection/Private/Wrappers/StructureWrapperClass.h"

namespace DAVA
{
List<std::unique_ptr<ReflectedType>> ReflectedTypeDB::customReflectedTypes;
UnorderedMap<const Type*, ReflectedType*> ReflectedTypeDB::typeToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::typeNameToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::permanentNameToReflectedTypeMap;

void ReflectedTypeDB::RegisterDBType(ReflectedType* r)
{
    typeToReflectedTypeMap[r->type] = r;
    typeNameToReflectedTypeMap[String(r->type->GetName())] = r;
}

const ReflectedType* ReflectedTypeDB::GetByPointer(const void* ptr, const Type* derefType)
{
    DVASSERT(nullptr != ptr);
    DVASSERT(nullptr != derefType);
    DVASSERT(!derefType->IsPointer());

    Type::SeedCastOP seedOP = derefType->GetSeedCastOP();

    if (nullptr != seedOP)
    {
        const ReflectionBase* rb = static_cast<const ReflectionBase*>((*seedOP)(ptr));
        return ReflectedTypeDBDetail::GetVirtualReflectedType(rb);
    }

    return nullptr;
}

const ReflectedType* ReflectedTypeDB::GetByType(const Type* type)
{
    const ReflectedType* ret = nullptr;

    auto it = typeToReflectedTypeMap.find(type);
    if (it != typeToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedTypeDB::GetByTypeName(const String& rttiName)
{
    const ReflectedType* ret = nullptr;

    auto it = typeNameToReflectedTypeMap.find(rttiName);
    if (it != typeNameToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedTypeDB::GetByPermanentName(const String& permanentName)
{
    const ReflectedType* ret = nullptr;

    auto it = permanentNameToReflectedTypeMap.find(permanentName);
    if (it != permanentNameToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

ReflectedType* ReflectedTypeDB::CreateCustomType(const Type* type, const String& permanentName)
{
    customReflectedTypes.emplace_back(new ReflectedType(type));
    ReflectedType* ret = customReflectedTypes.back().get();

    String rttiName(type->GetName());

    DVASSERT(typeToReflectedTypeMap.count(type) == 0 && "ReflectedType with specified RttiType already exists");
    DVASSERT(typeNameToReflectedTypeMap.count(rttiName) == 0 && "ReflectedType with specified RttiType::name already exists");

    typeToReflectedTypeMap[type] = ret;
    typeNameToReflectedTypeMap[rttiName] = ret;

    RegisterPermanentName(ret, permanentName);

    return ret;
}

void ReflectedTypeDB::RegisterPermanentName(const ReflectedType* reflectedType, const String& permanentName)
{
    ReflectedType* rt = const_cast<ReflectedType*>(reflectedType);

    DVASSERT(rt != nullptr);
    DVASSERT(rt->permanentName.empty() && "Name is already set");
    DVASSERT(permanentNameToReflectedTypeMap.count(permanentName) == 0 && "Permanent name alredy in use");

    rt->permanentName = permanentName;
    permanentNameToReflectedTypeMap[permanentName] = rt;
}

ReflectedTypeDB::Stats ReflectedTypeDB::GetStats()
{
    ReflectedTypeDB::Stats stats;

    stats.reflectedTypeCount = typeToReflectedTypeMap.size();
    stats.reflectedTypeMemory = stats.reflectedTypeCount * sizeof(ReflectedType);

    for (auto& p : typeToReflectedTypeMap)
    {
        const ReflectedType* tr = p.second;
        if (tr->structure != nullptr)
        {
            stats.reflectedStructCount++;
            stats.reflectedStructFieldsCount += tr->structure->fields.size();
            stats.reflectedStructMethodsCount += tr->structure->methods.size();
            stats.reflectedStructEnumsCount += tr->structure->enums.size();
            stats.reflectedStructCtorsCount += tr->structure->ctors.size();

            if (tr->structure->dtor != nullptr)
                stats.reflectedStructDtorsCount++;

            if (tr->structure->meta != nullptr)
            {
                stats.reflectedStructMetasCount++;
                stats.reflectedStructMetaMCount += tr->structure->meta->metas.size();
            }
        }

        if (tr->structureWrapper != nullptr)
        {
            stats.reflectedStructWrapperCount++;

            StructureWrapperClass* swc = dynamic_cast<StructureWrapperClass*>(tr->structureWrapper.get());
            if (nullptr != swc)
            {
                stats.reflectedStructWrapperClassCount++;
                stats.reflectedStructWrapperClassMemory +=
                sizeof(StructureWrapperClass) +
                swc->fieldsCache.size() * sizeof(StructureWrapperClass::CachedFieldEntry) +
                swc->methodsCache.size() * sizeof(StructureWrapperClass::CachedMethodEntry) +
                swc->fieldsNameIndexes.size() * sizeof(decltype(swc->fieldsNameIndexes)::key_type) +
                swc->fieldsNameIndexes.size() * sizeof(decltype(swc->fieldsNameIndexes)::mapped_type) +
                swc->methodsNameIndexes.size() * sizeof(decltype(swc->methodsNameIndexes)::key_type) +
                swc->methodsNameIndexes.size() * sizeof(decltype(swc->methodsNameIndexes)::mapped_type);
            }
        }
    }

    stats.reflectedTypeDBMemory =
    sizeof(ReflectedTypeDB) +
    sizeof(ReflectedTypeDB::typeToReflectedTypeMap) +
    sizeof(ReflectedTypeDB::typeNameToReflectedTypeMap) +
    sizeof(ReflectedTypeDB::permanentNameToReflectedTypeMap) +
    ReflectedTypeDB::typeToReflectedTypeMap.size() * sizeof(decltype(ReflectedTypeDB::typeToReflectedTypeMap)::key_type) +
    ReflectedTypeDB::typeToReflectedTypeMap.size() * sizeof(decltype(ReflectedTypeDB::typeToReflectedTypeMap)::mapped_type) +
    ReflectedTypeDB::typeNameToReflectedTypeMap.size() * sizeof(decltype(ReflectedTypeDB::typeNameToReflectedTypeMap)::key_type) +
    ReflectedTypeDB::typeNameToReflectedTypeMap.size() * sizeof(decltype(ReflectedTypeDB::typeNameToReflectedTypeMap)::mapped_type) +
    ReflectedTypeDB::permanentNameToReflectedTypeMap.size() * sizeof(decltype(ReflectedTypeDB::permanentNameToReflectedTypeMap)::key_type) +
    ReflectedTypeDB::permanentNameToReflectedTypeMap.size() * sizeof(decltype(ReflectedTypeDB::permanentNameToReflectedTypeMap)::mapped_type);

    stats.reflectedStructMemory =
    stats.reflectedStructCount * sizeof(ReflectedStructure) +
    stats.reflectedStructCtorsCount * sizeof(AnyFn) +
    stats.reflectedStructDtorsCount * sizeof(AnyFn) +
    stats.reflectedStructEnumsCount * sizeof(ReflectedStructure::Enum) +
    stats.reflectedStructFieldsCount * sizeof(ReflectedStructure::Field) +
    stats.reflectedStructMethodsCount * sizeof(ReflectedStructure::Method) +
    stats.reflectedStructMetasCount * sizeof(ReflectedMeta) +
    stats.reflectedStructMetaMCount * sizeof(Any);

    stats.totalMemory =
    stats.reflectedTypeMemory +
    stats.reflectedTypeDBMemory +
    stats.reflectedStructMemory +
    stats.reflectedStructWrapperClassMemory;

    return stats;
}

} // namespace DAVA
