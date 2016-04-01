#pragma once

namespace DAVA
{
class ReflectionDB;

struct VirtualReflectionDB
{
    virtual const ReflectionDB* GetVirtualReflectionDB() const = 0;
};

struct VirtualReflectionDBGetter
{
    template <typename T>
    static const ReflectionDB* Get(T* object)
    {
        static const bool is_base_of_virt = std::is_base_of<VirtualReflectionDB, T>::value;

        auto condition = std::integral_constant<bool, is_base_of_virt>();
        return GetImpl<T>(object, condition);
    }

private:
    template <typename T>
    static const ReflectionDB* GetImpl(T* object, std::false_type)
    {
        return nullptr;
    }

    template <typename T>
    static const ReflectionDB* GetImpl(T* object, std::true_type)
    {
        return static_cast<VirtualReflectionDB*>(object)->GetVirtualReflectionDB();
    }
};
}
