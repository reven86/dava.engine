#pragma once

namespace DAVA
{
class ReflectedType;
class ReflectedObject final
{
public:
    ReflectedObject() = default;

    template <typename T>
    ReflectedObject(T* ptr);

    template <typename T>
    ReflectedObject(const T* ptr);

    /*
    ReflectedObject(void* ptr, const ReflectedType* reflectedType);
    ReflectedObject(const void* ptr, const ReflectedType* reflectedType);
    */

    bool IsValid() const;
    bool IsConst() const;

    const ReflectedType* GetReflectedType() const;

    template <typename T>
    T* GetPtr() const;

    void* GetVoidPtr() const;

    //ReflectedObject Deref() const;

protected:
    void* ptr = nullptr;
    const ReflectedType* reflectedType = nullptr;

    bool isConst = false;
};
} // namespace DAVA
