#include "ReflectionWrappersDefault.h"
#include "Private/StructureWrapperClass.h"
#include "Private/ValueWrapperStatic.h"
#include "Private/ValueWrapperStaticFn.h"
#include "Private/ValueWrapperClass.h"
#include "Private/ValueWrapperClassFn.h"
#include "Private/CtorWrapperDefault.h"
#include "Private/DtorWrapperDefault.h"

#include <cassert>
#include <tuple>

namespace DAVA
{
template <typename C>
struct ReflectionRegistrator
{
    static ReflectionRegistrator& Begin()
    {
        static ReflectionRegistrator ret;
        return ret;
    }

    template <typename B>
    ReflectionRegistrator& Base()
    {
        ReflectionDB::RegisterBaseClass<B, C>();
        childrenWrapper->AddBase<B, C>();
        return *this;
    }

    template <typename B, typename B1, typename... Args>
    ReflectionRegistrator& Base()
    {
        ReflectionDB::RegisterBaseClass<B, C>();
        childrenWrapper->AddBase<C, B>();
        return Base<B1, Args...>();
    }

    template <typename... Args>
    ReflectionRegistrator& Constructor()
    {
        ReflectionDB* db = ReflectionDB::EditGlobalDB<C>();
        auto ctorWrapper = std::make_unique<CtorWrapperDefault<C, Args...>>();
        db->ctorWrappers.emplace_back(std::move(ctorWrapper));
        return *this;
    }

    ReflectionRegistrator& Destructor()
    {
        ReflectionDB* db = ReflectionDB::EditGlobalDB<C>();
        auto dtorWrapper = std::make_unique<DtorWrapperDefault<C>>();
        db->dtorWrapper = std::move(dtorWrapper);
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T* field)
    {
        auto valueWrapper = std::make_unique<ValueWrapperStatic<T>>(field);
        childrenWrapper->AddField<T>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T C::*field)
    {
        auto valueWrapper = std::make_unique<ValueWrapperClass<T, C>>(field);
        childrenWrapper->AddField<T>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (*getter)(), std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        auto valueWrapper = std::make_unique<ValueWrapperStaticFn<GetT, SetT>>(getter, nullptr);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (*getter)(), void (*setter)(SetT))
    {
        auto valueWrapper = std::make_unique<ValueWrapperStaticFn<GetT, SetT>>(getter, setter);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)(), std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        auto valueWrapper = std::make_unique<ValueWrapperClassFn<GetT, SetT, C>>(getter, nullptr);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)() const, std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using GetterT = GetT (C::*)();
        auto valueWrapper = std::make_unique<ValueWrapperClassFn<GetT, SetT, C>>(reinterpret_cast<GetterT>(getter), nullptr);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)(), void (C::*setter)(SetT))
    {
        auto valueWrapper = std::make_unique<ValueWrapperClassFn<GetT, SetT, C>>(getter, setter);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)() const, void (C::*setter)(SetT))
    {
        using GetterT = GetT (C::*)();
        auto valueWrapper = std::make_unique<ValueWrapperClassFn<GetT, SetT, C>>(reinterpret_cast<GetterT>(getter), setter);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& operator[](T&& meta)
    {
    }

    void End()
    {
        ReflectionDB* db = ReflectionDB::EditGlobalDB<C>();

        // override children for class C in global DB
        db->structureWrapper = std::move(childrenWrapper);
    }

protected:
    ReflectionRegistrator() = default;

    const Type* thisType = Type::Instance<C>();
    std::unique_ptr<StructureWrapperClass> childrenWrapper = std::make_unique<StructureWrapperClass>();
};

} // namespace DAVA
