#pragma once

#include <tuple>
#include "Base/Type.h"
#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/Private/AutoStorage.h"

namespace DAVA
{
namespace AnyFnDetail
{
struct Invoker;
}

class AnyFn final
{
public:
    using AnyFnStorage = AutoStorage<>;

    struct InvokeParams
    {
        const Type* retType;
        Vector<const Type*> argsType;

        template <typename Ret, typename... Args>
        void Init();
    };

    AnyFn();

    template <typename Fn>
    AnyFn(const Fn& fn);

    template <typename Ret, typename... Args>
    AnyFn(Ret (*fn)(Args...));

    template <typename Ret, typename Cls, typename... Args>
    AnyFn(Ret (Cls::*fn)(Args...) const);

    template <typename Ret, typename Cls, typename... Args>
    AnyFn(Ret (Cls::*fn)(Args...));

    bool IsValid() const;
    bool IsStatic() const;

    const InvokeParams& GetInvokeParams() const;

    template <typename... Args>
    Any Invoke(const Args&... args) const;

    AnyFn BindThis(const void* this_) const;

private:
    AnyFnStorage anyFnStorage;
    AnyFnDetail::Invoker* invoker;
    InvokeParams invokeParams;
};

} // namespace DAVA

#define __Dava_AnyFn__
#include "Base/Private/AnyFn_impl.h"