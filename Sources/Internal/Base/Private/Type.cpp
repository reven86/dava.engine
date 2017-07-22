#include "Base/Type.h"
#include "Base/TypeInheritance.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
Type::Type()
    : inheritance(nullptr, [](TypeInheritance* inh) { if (nullptr != inh) delete inh; })
{
}

TypeInheritance* Type::EditInheritance() const
{
    Type* thisType = const_cast<Type*>(this);
    if (!inheritance)
    {
        thisType->inheritance.reset(new TypeInheritance());
    }

    return inheritance.get();
}

void TypeDB::AddType(Type** typeptr)
{
    TypeDB* localDB = GetLocalDB();
    localDB->types.push_back(typeptr);
}

TypeDB* TypeDB::GetLocalDB()
{
    return *GetLocalDBPtr();
}

TypeDB** TypeDB::GetLocalDBPtr()
{
    static TypeDB db;
    static TypeDB* dbPtr = &db;
    return &dbPtr;
}

void TypeDB::SetMasterDB(TypeDB* db)
{
    static bool hasMaster = false;

    DVASSERT(!hasMaster);
    hasMaster = true;

    TypeDB* localDb = GetLocalDB();

    // Sync types from master to local
    for (Type** masterTypePtr : db->types)
    {
        Type* masterType = *masterTypePtr;
        for (Type** localTypePtr : localDb->types)
        {
            Type* localType = *localTypePtr;
            if (0 == std::strcmp(masterType->GetName(), localType->GetName()))
            {
                if (localType->GetTypeFlags() == masterType->GetTypeFlags())
                {
                    *localTypePtr = masterType;
                    break;
                }
            }
        }
    }

    // Now override db from local to master
    TypeDB** localDBPtr = GetLocalDBPtr();
    *localDBPtr = db;
}

TypeDB::Stats TypeDB::GetLocalDBStats()
{
    TypeDB::Stats stats;

    TypeDB* localDB = TypeDB::GetLocalDB();

    stats.typesCount = localDB->types.size();
    stats.typesMemory = stats.typesCount * sizeof(DAVA::Type);

    for (Type** typePtr : localDB->types)
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

    stats.typeDBMemory = sizeof(TypeDB);
    stats.typeDBMemory += stats.typesCount * sizeof(decltype(localDB->types)::value_type);

    stats.totalMemory = stats.typesMemory;
    stats.totalMemory += stats.typeInheritanceMemory;
    stats.totalMemory += stats.typeDBMemory;

    return stats;
}

} // namespace DAVA
