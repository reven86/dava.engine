#include "Base/Type.h"
#include "Base/TypeInheritance.h"

namespace DAVA
{
Type::Type()
    : inheritance(nullptr, [](const TypeInheritance* inh) { if (nullptr != inh) delete inh; })
{
}

namespace TypeDetail
{
void TypeDB::AddType(Type** typeptr)
{
    TypeDB::DB* localDB = GetLocalDB();
    localDB->push_back(typeptr);
}

TypeDB::DB* TypeDB::GetLocalDB()
{
    static TypeDB::DB localDB;
    return &localDB;
}

TypeDB::Stats TypeDB::GetStats()
{
    Stats stats;

    DB* localDB = GetLocalDB();

    stats.typesCount = localDB->size();
    stats.typesMemory = stats.typesCount * sizeof(DAVA::Type);

    for (Type** typePtr : *localDB)
    {
        const TypeInheritance* typeInh = (*typePtr)->GetInheritance();
        if (nullptr != typeInh)
        {
            stats.typeInheritanceCount++;
            stats.typeInheritanceInfoCount += typeInh->GetBaseTypes().size();
            stats.typeInheritanceInfoCount += typeInh->GetDerivedTypes().size();
        }
    }

    stats.typeInheritanceMemory = stats.typeInheritanceCount * sizeof(TypeInheritance);
    stats.typeInheritanceMemory += stats.typeInheritanceInfoCount * sizeof(TypeInheritance::Info);

    stats.typeDBMemory = sizeof(TypeDB::DB);
    stats.typeDBMemory += localDB->size() * sizeof(DB::value_type);

    stats.totalMemory = stats.typesMemory;
    stats.totalMemory += stats.typeInheritanceMemory;
    stats.totalMemory += stats.typeDBMemory;

    return stats;
}

} // namespace TypeDetail
} // namespace DAVA
