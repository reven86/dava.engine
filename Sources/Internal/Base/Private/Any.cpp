#include "Base/Any.h"
#include <string>

namespace DAVA
{
std::unordered_map<const Type*, Any::AnyOP> Any::operations = {
    AnyDetails::DefaultOP<void*>(),
    AnyDetails::DefaultOP<bool>(),
    AnyDetails::DefaultOP<DAVA::int8>(),
    AnyDetails::DefaultOP<DAVA::uint8>(),
    AnyDetails::DefaultOP<DAVA::float32>(),
    AnyDetails::DefaultOP<DAVA::float64>(),
    AnyDetails::DefaultOP<DAVA::int16>(),
    AnyDetails::DefaultOP<DAVA::uint16>(),
    AnyDetails::DefaultOP<DAVA::int32>(),
    AnyDetails::DefaultOP<DAVA::uint32>(),
    AnyDetails::DefaultOP<DAVA::int64>(),
    AnyDetails::DefaultOP<DAVA::uint64>(),
    AnyDetails::DefaultOP<DAVA::String>()
};

Any::Any(Any&& any)
{
    storage = std::move(any.storage);
    type = any.type;
    any.type = nullptr;
}

bool Any::IsEmpty() const
{
    return (nullptr == type);
}

void Any::Clear()
{
    storage.Clear();
    type = nullptr;
}

const Type* Any::GetType() const
{
    return type;
}

void Any::LoadValue(const Type* type_, void* data)
{
    auto op = operations.find(type_);

    if (op == operations.end() || nullptr == op->second.load)
        throw Exception(Exception::BadOperation);

    type = type_;
    op->second.load(storage, data);
}

void Any::SaveValue(void* data, size_t size) const
{
    if (type->GetSize() < size)
        throw Exception(Exception::BadSize);

    auto op = operations.find(type);

    if (op == operations.end() || nullptr == op->second.save)
        throw Exception(Exception::BadOperation);

    op->second.save(storage, data);
}

Any& Any::operator=(Any&& any)
{
    if (this != &any)
    {
        type = any.type;
        storage = std::move(any.storage);
        any.type = nullptr;
    }

    return *this;
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

    auto op = operations.find(type);

    if (op == operations.end() || nullptr == op->second.compare)
        throw Exception(Exception::BadOperation);

    return op->second.compare(storage.GetData(), any.storage.GetData());
}

bool Any::operator!=(const Any& any) const
{
    return !operator==(any);
}

Any::Exception::Exception(ErrorCode code)
    : runtime_error("")
    , errorCode(code)
{
}

} // namespace DAVA
