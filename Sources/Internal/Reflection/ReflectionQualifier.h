#pragma once

#include <type_traits>
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedStructure.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/ValueWrapperDirect.h"
#include "Reflection/Private/ValueWrapperClass.h"
#include "Reflection/Private/ValueWrapperClassFn.h"
#include "Reflection/Private/ValueWrapperClassFnPtr.h"
#include "Reflection/Private/ValueWrapperStatic.h"
#include "Reflection/Private/ValueWrapperStaticFn.h"
#include "Reflection/Private/ValueWrapperStaticFnPtr.h"
#include "Reflection/Private/CtorWrapperDefault.h"
#include "Reflection/Private/DtorWrapperDefault.h"
#include "Reflection/Private/StructureWrapperClass.h"
#include "Reflection/Private/StructureWrapperPtr.h"
#include "Reflection/Private/StructureWrapperStd.h"
#include "Reflection/Private/StructureEditorWrapperPtr.h"
#include "Reflection/Private/StructureEditorWrapperStd.h"

namespace DAVA
{
/// \brief A reflection registration, that is used to register complex types structure.
template <typename C>
class ReflectionQualifier final
{
public:
    static ReflectionQualifier& Begin();

    template <typename... Args>
    ReflectionQualifier& Constructor();

    ReflectionQualifier& Destructor();

    template <typename T>
    ReflectionQualifier& Field(const char* name, T* field);

    template <typename T>
    ReflectionQualifier& Field(const char* name, T C::*field);

    template <typename GetT>
    ReflectionQualifier& Field(const char* name, GetT (*getter)(), std::nullptr_t);

    template <typename GetT, typename SetT>
    ReflectionQualifier& Field(const char* name, GetT (*getter)(), void (*setter)(SetT));

    template <typename GetT>
    ReflectionQualifier& Field(const char* name, GetT (C::*getter)(), std::nullptr_t);

    template <typename GetT>
    ReflectionQualifier& Field(const char* name, GetT (C::*getter)() const, std::nullptr_t);

    template <typename GetT, typename SetT>
    ReflectionQualifier& Field(const char* name, GetT (C::*getter)() const, void (C::*setter)(SetT));

    template <typename GetT, typename SetT>
    ReflectionQualifier& Field(const char* name, GetT (C::*getter)(), void (C::*setter)(SetT));

    template <typename GetT>
    ReflectionQualifier& Field(const char* name, const Function<GetT()>& getter, std::nullptr_t);

    template <typename GetT, typename SetT>
    ReflectionQualifier& Field(const char* name, const Function<GetT()>& getter, const Function<void(SetT)>& setter);

    template <typename GetT>
    ReflectionQualifier& Field(const char* name, const Function<GetT(C*)>& getter, std::nullptr_t);

    template <typename GetT, typename SetT>
    ReflectionQualifier& Field(const char* name, const Function<GetT(C*)>& getter, const Function<void(C*, SetT)>& setter);

    template <typename Mt>
    ReflectionQualifier& Method(const char* name, const Mt& method);

    void End();

    ReflectionQualifier& operator[](ReflectedMeta&& meta);

private:
    ReflectionQualifier() = default;
    ReflectedStructure* structure = nullptr;

    std::unique_ptr<ReflectedMeta>* lastMeta;

    template <typename T>
    ReflectionQualifier& AddField(const char* name, ValueWrapper* vw);
};

} // namespace DAVA

#define __DAVA_Reflection_Qualifier__
#include "Reflection/Private/ReflectionQualifier_impl.h"
