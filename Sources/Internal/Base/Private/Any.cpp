#include "Base/Any.h"

namespace DAVA
{
std::unique_ptr<Any::AnyOPsMap> Any::anyOPsMap;

#ifdef ANY_EXPERIMENTAL_CAST_IMPL
std::unique_ptr<Any::CastOPsMap> Any::castOPsMap;
#endif

/*
Any::Any()
{
    if (nullptr == anyOPsMap && nullptr == castOPsMap)
    {
        anyOPsMap.reset(new AnyOPsMap());
        castOPsMap.reset(new CastOPsMap());

        RegisterDefaultOPs<void*>();
        RegisterDefaultOPs<bool>();
        RegisterDefaultOPs<DAVA::int8>();
        RegisterDefaultOPs<DAVA::uint8>();
        RegisterDefaultOPs<DAVA::float32>();
        RegisterDefaultOPs<DAVA::float64>();
        RegisterDefaultOPs<DAVA::int16>();
        RegisterDefaultOPs<DAVA::uint16>();
        RegisterDefaultOPs<DAVA::int32>();
        RegisterDefaultOPs<DAVA::uint32>();
        RegisterDefaultOPs<DAVA::int64>();
        RegisterDefaultOPs<DAVA::uint64>();
        RegisterDefaultOPs<DAVA::String>();
    }
}
*/

void Any::LoadValue(const Type* type_, void* data)
{
    if (anyOPsMap->count(type_) == 0 || anyOPsMap->at(type_).load == nullptr)
    {
        throw Exception(Exception::BadOperation, "Load operation wasn't registered");
    }

    const AnyOPs& ops = anyOPsMap->at(type_);
    (*ops.load)(anyStorage, data);
    type = type_;
}

void Any::StoreValue(void* data, size_t size) const
{
    if (anyOPsMap->count(type) == 0 || anyOPsMap->at(type).store == nullptr)
    {
        throw Exception(Exception::BadOperation, "Save operation wasn't registered");
    }

    if (type->GetSize() < size)
    {
        throw Exception(Exception::BadSize, "Type size mismatch while saving value into Any");
    }

    const AnyOPs& ops = anyOPsMap->at(type);
    (*ops.store)(anyStorage, data);
}

bool Any::operator==(const Any& any) const
{
    if (type != any.type)
    {
        return false;
    }
    else if (nullptr == type)
    {
        return true;
    }

    if (anyOPsMap->count(type) == 0 || anyOPsMap->at(type).compare == nullptr)
    {
        throw Exception(Exception::BadOperation, "Compare operation wasn't registered");
    }

    const AnyOPs& ops = anyOPsMap->operator[](type);
    return (*ops.compare)(anyStorage.GetData(), any.anyStorage.GetData());
}

Any::Exception::Exception(ErrorCode code, const std::string& message)
    : runtime_error(message)
    , errorCode(code)
{
}

Any::Exception::Exception(ErrorCode code, const char* message)
    : runtime_error(message)
    , errorCode(code)
{
}

} // namespace DAVA
