#include "../Reflection.h"
#include "../ReflectionDB.h"

namespace DAVA
{
std::set<std::unique_ptr<ReflectionDB>> ReflectionDB::allDBs;

/*
const ReflectionDB * ReflectionDB::GetGlobalDB(const Type * type)
{
    return EditGlobalDB(type);
}

ReflectionDB * ReflectionDB::EditGlobalDB(const Type * type)
{
    return type->reflectionDb;
}
*/

} // namespace DAVA
