#pragma once
#include <type_traits>
#include <cassert>
#include <map>

#include "Base/Type.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
class ReflectedObject final
{
public:
    ReflectedObject() = default;

    template <typename T>
    ReflectedObject(T* ptr_, const ReflectedMeta* meta_ = nullptr)
        : ptr(ptr_)
        , type(Type::Instance<T*>())
        , meta(meta_)
    {
    }

    template <typename T>
    ReflectedObject(const T* ptr_, const ReflectedMeta* meta_ = nullptr)
        : ptr(const_cast<T*>(ptr_))
        , type(Type::Instance<T*>())
        , meta(meta_)
    {
    }

    ReflectedObject(void* ptr_, const Type* type_, const ReflectedMeta* meta_ = nullptr)
        : ptr(ptr_)
        , type(type_)
        , meta(meta_)
    {
        assert(nullptr != type_);
        assert(type_->IsPointer());
    }

    inline bool IsValid() const
    {
        return ((nullptr != ptr) && (nullptr != type));
    }

    inline const Type* GetType() const
    {
        return type;
    }

    inline ReflectedObject GetDerefObject() const
    {
        ReflectedObject ret;

        if (!IsValid())
            return ret;

        const Type* derefType = type->Deref();

        if (nullptr == derefType)
            return ret;

        if (derefType->IsPointer())
        {
            void* inPtr = *(static_cast<void**>(ptr));
            ret = ReflectedObject(inPtr, derefType);
        }

        return ret;
    }

    template <typename T>
    T* GetPtr() const
    {
        const Type* reqType = Type::Instance<T*>();

        assert(reqType == type || reqType->IsDerivedFrom(type));
        return static_cast<T*>(ptr);
    }

    void* GetVoidPtr() const
    {
        return ptr;
    }

    const ReflectedMeta* GetMeta() const
    {
        return meta;
    }

protected:
    void* ptr = nullptr;
    const Type* type = nullptr;
    const ReflectedMeta* meta = nullptr;
};

} // namespace DAVA
