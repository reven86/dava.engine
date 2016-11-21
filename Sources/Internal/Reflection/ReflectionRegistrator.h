#pragma once

#include "Reflection/Reflection.h"
#include "Reflection/Private/Wrappers/ValueWrapperDefault.h"
#include "Reflection/Private/Wrappers/ValueWrapperDirect.h"
#include "Reflection/Private/Wrappers/ValueWrapperClass.h"
#include "Reflection/Private/Wrappers/ValueWrapperClassFn.h"
#include "Reflection/Private/Wrappers/ValueWrapperClassFnPtr.h"
#include "Reflection/Private/Wrappers/ValueWrapperStatic.h"
#include "Reflection/Private/Wrappers/ValueWrapperStaticFn.h"
#include "Reflection/Private/Wrappers/ValueWrapperStaticFnPtr.h"
#include "Reflection/Private/Wrappers/StructureWrapperClass.h"
#include "Reflection/Private/Wrappers/StructureWrapperPtr.h"
#include "Reflection/Private/Wrappers/StructureWrapperStdIdx.h"
#include "Reflection/Private/Wrappers/StructureWrapperStdSet.h"
#include "Reflection/Private/Wrappers/StructureWrapperStdMap.h"

namespace DAVA
{
/// \brief A reflection registration, that is used to register complex types structure.
template <typename C>
class ReflectionRegistrator final
{
public:
    static ReflectionRegistrator& Begin();

    template <typename... Args>
    ReflectionRegistrator& ConstructorByValue();

    template <typename... Args>
    ReflectionRegistrator& ConstructorByPointer();

    template <typename... Args>
    ReflectionRegistrator& ConstructorByPointer(C* (*fn)(Args...));

    ReflectionRegistrator& DestructorByPointer();

    ReflectionRegistrator& DestructorByPointer(void (*fn)(C*));

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T* field);

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T C::*field);

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (*getter)(), std::nullptr_t);

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (*getter)(), void (*setter)(SetT));

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)(), std::nullptr_t);

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)() const, std::nullptr_t);

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)() const, void (C::*setter)(SetT));

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)(), void (C::*setter)(SetT));

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT()>& getter, std::nullptr_t);

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT()>& getter, const Function<void(SetT)>& setter);

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT(C*)>& getter, std::nullptr_t);

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT(C*)>& getter, const Function<void(C*, SetT)>& setter);

    template <typename Mt>
    ReflectionRegistrator& Method(const char* name, const Mt& method);

    ReflectionRegistrator& BindMeta(ReflectedMeta&& meta);

    void End();

    ReflectionRegistrator& operator[](ReflectedMeta&& meta);

private:
    ReflectionRegistrator() = default;
    ReflectedStructure* structure = nullptr;

    std::unique_ptr<ReflectedMeta>* lastMeta;

    template <typename T>
    ReflectionRegistrator& AddField(const char* name, ValueWrapper* vw);
};

} // namespace DAVA

#define __DAVA_Reflection_Qualifier__
#include "Reflection/Private/ReflectionRegistrator_impl.h"
