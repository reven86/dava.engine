#pragma once

#include <tuple>
#include "Base/RttiType.h"
#include "Base/BaseTypes.h"
#include "Base/Exception.h"
#include "Base/Any.h"
#include "Base/Private/AutoStorage.h"

namespace DAVA
{
class AnyFnInvoker;
class AnyFn final
{
public:
    struct Params;
    using AnyFnStorage = AutoStorage<>;

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

    const Params& GetInvokeParams() const;

    template <typename... Args>
    Any Invoke(const Args&... args) const;

    AnyFn BindThis(const void* this_) const;

    struct Params
    {
        Params();
        bool operator==(const Params&) const;

        template <typename Ret, typename... Args>
        Params& Set();

        template <typename... Args>
        Params& SetArgs();

        template <typename Ret, typename... Args>
        bool ArePassing() const;

        template <typename Ret, typename... Args>
        static Params From();

        template <typename... Args>
        static Params FromArgs();

        const RttiType* retType;
        Vector<const RttiType*> argsType;
    };

private:
    AnyFnStorage anyFnStorage;
    AnyFnInvoker* invoker;
    Params invokeParams;
};

} // namespace DAVA

#define __Dava_AnyFn__
#include "Base/Private/AnyFn_impl.h"