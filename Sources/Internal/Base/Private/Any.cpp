#include "Base/Any.h"

namespace DAVA
{
UnorderedMap<const Type*, Any::AnyOPs>* Any::operations = nullptr;

void Any::LoadValue(const Type* type_, void* data)
{
    if (operations == nullptr
        || operations->count(type_) == 0
        || operations->at(type_).load == nullptr)
    {
        throw Exception(Exception::BadOperation, "Load operation wasn't registered");
    }

    const AnyOPs& ops = operations->at(type_);
    (*ops.load)(anyStorage, data);
    type = type_;
}

void Any::SaveValue(void* data, size_t size) const
{
    if (operations == nullptr
        || operations->count(type) == 0
        || operations->at(type).save == nullptr)
    {
        throw Exception(Exception::BadOperation, "Save operation wasn't registered");
    }

    if (type->GetSize() < size)
    {
        throw Exception(Exception::BadSize, "Type size mismatch while saving value into Any");
    }

    const AnyOPs& ops = operations->at(type);
    (*ops.save)(anyStorage, data);
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

    if (operations == nullptr
        || operations->count(type) == 0
        || operations->at(type).compare == nullptr)
    {
        throw Exception(Exception::BadOperation, "Compare operation wasn't registered");
    }

    const AnyOPs& ops = operations->operator[](type);
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
