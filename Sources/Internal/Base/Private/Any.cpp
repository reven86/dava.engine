#include "Base/Any.h"

namespace DAVA
{
std::unique_ptr<Any::AnyOPsMap> Any::anyOPsMap = std::make_unique<Any::AnyOPsMap>();
std::unique_ptr<Any::CastOPsMap> Any::castOPsMap = std::make_unique<Any::CastOPsMap>();

void Any::LoadValue(const Type* type_, void* data)
{
    type = type_;

    if (type_->IsPointer())
    {
        void** src = reinterpret_cast<void**>(data);
        anyStorage.SetAuto(*src);
    }
    else if (type_->IsFundamental())
    {
        anyStorage.SetData(data, type_->GetSize());
    }
    else
    {
        auto it = anyOPsMap->find(type_);
        if (it != anyOPsMap->end())
        {
            Any::LoadOP op = it->second.load;
            if (nullptr != op)
            {
                return (*op)(anyStorage, data);
            }
        }

        throw Exception(Exception::BadOperation, "Load operation wasn't registered");
    }
}

void Any::StoreValue(void* data, size_t size) const
{
    if (nullptr != type)
    {
        if (type->GetSize() > size)
        {
            throw Exception(Exception::BadSize, "Type size mismatch while saving value from Any");
        }

        if (type->IsPointer())
        {
            void** dst = reinterpret_cast<void**>(data);
            *dst = anyStorage.GetAuto<void*>();
        }
        else if (type->IsFundamental())
        {
            std::memcpy(data, anyStorage.GetData(), size);
        }
        else
        {
            auto it = anyOPsMap->find(type);
            if (it != anyOPsMap->end())
            {
                Any::StoreOP op = it->second.store;
                if (nullptr != op)
                {
                    return (*op)(anyStorage, data);
                }
            }

            throw Exception(Exception::BadOperation, "Save operation wasn't registered");
        }
    }
}

bool Any::operator==(const Any& any) const
{
    if (any.type == nullptr)
    {
        return (type == nullptr);
    }

    if (any.type->IsPointer())
    {
        if (type->IsPointer())
        {
            return GetImpl<void*>() == any.GetImpl<void*>();
        }
        else
        {
            return false;
        }
    }

    if (type != any.type)
    {
        return false;
    }

    if (type->IsFundamental())
    {
        return (0 == std::memcmp(anyStorage.GetData(), any.anyStorage.GetData(), type->GetSize()));
    }
    else
    {
        auto it = anyOPsMap->find(type);
        if (it != anyOPsMap->end())
        {
            Any::CompareOP op = it->second.compare;
            if (op != nullptr)
            {
                return (*op)(anyStorage.GetData(), any.anyStorage.GetData());
            }
        }
    }

    throw Exception(Exception::BadOperation, "Compare operation wasn't registered");
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
