#pragma once

#include <tuple>
#include "Base/Type.h"
#include "Base/BaseTypes.h"
#include "Base/Exception.h"
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

    struct Params
    {
        Params();
        bool operator==(const Params&) const;

        template <typename Ret, typename... Args>
        Params& Set();

        template <typename... Args>
        Params& SetArgs();

        template <typename Ret, typename... Args>
        static Params From();

        template <typename... Args>
        static Params FromArgs();

        const Type* retType;
        Vector<const Type*> argsType;
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

    const Params& GetInvokeParams() const;

    template <typename... Args>
    Any Invoke(const Args&... args) const;

    AnyFn BindThis(const void* this_) const;

private:
    AnyFnStorage anyFnStorage;
    AnyFnDetail::Invoker* invoker;
    Params invokeParams;
};

struct AnyFnException : public Exception
{
    enum ErrorCode
    {
        BadBind,
        BadArguments,
    };

    ErrorCode ecode;

    AnyFnException(ErrorCode ecode_, const String& message, const char* file_, size_t line_)
        : Exception(message, file_, line_)
        , ecode(ecode_)
    {
    }
};

} // namespace DAVA

#define __Dava_AnyFn__
#include "Base/Private/AnyFn_impl.h"