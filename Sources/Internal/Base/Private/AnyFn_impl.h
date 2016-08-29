#pragma once

#ifndef __Dava_AnyFn__
#include "Base/AnyFn.h"
#endif

namespace DAVA
{
namespace AnyFnDetail
{
struct Invoker
{
    virtual bool IsStatic() const = 0;
    virtual Any Invoke(const AnyFn::AnyFnStorage&) const = 0;
    virtual Any Invoke(const AnyFn::AnyFnStorage&, const Any&) const = 0;
    virtual Any Invoke(const AnyFn::AnyFnStorage&, const Any&, const Any&) const = 0;
    virtual Any Invoke(const AnyFn::AnyFnStorage&, const Any&, const Any&, const Any&) const = 0;
    virtual Any Invoke(const AnyFn::AnyFnStorage&, const Any&, const Any&, const Any&, const Any&) const = 0;
    virtual Any Invoke(const AnyFn::AnyFnStorage&, const Any&, const Any&, const Any&, const Any&, const Any&) const = 0;
    virtual Any Invoke(const AnyFn::AnyFnStorage&, const Any&, const Any&, const Any&, const Any&, const Any&, const Any&) const = 0;

    virtual AnyFn BindThis(const AnyFn::AnyFnStorage& storage, const void*) const = 0;

    virtual ~Invoker()
    {
    }
};

template <typename Fn, typename Ret, typename... Args>
struct StaticAnyFnInvoker : Invoker
{
    template <bool, typename R, typename... A>
    struct FinalInvoker;

    bool IsStatic() const override
    {
        return true;
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage) const override
    {
        return FinalInvoker<0 == sizeof...(Args), Ret>::Invoke(storage);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& a1) const override
    {
        return FinalInvoker<1 == sizeof...(Args), Ret, Any>::Invoke(storage, a1);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& a1, const Any& a2) const override
    {
        return FinalInvoker<2 == sizeof...(Args), Ret, Any, Any>::Invoke(storage, a1, a2);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& a1, const Any& a2, const Any& a3) const override
    {
        return FinalInvoker<3 == sizeof...(Args), Ret, Any, Any, Any>::Invoke(storage, a1, a2, a3);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& a1, const Any& a2, const Any& a3, const Any& a4) const override
    {
        return FinalInvoker<4 == sizeof...(Args), Ret, Any, Any, Any, Any>::Invoke(storage, a1, a2, a3, a4);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) const override
    {
        return FinalInvoker<5 == sizeof...(Args), Ret, Any, Any, Any, Any, Any>::Invoke(storage, a1, a2, a3, a4, a5);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6) const override
    {
        return FinalInvoker<6 == sizeof...(Args), Ret, Any, Any, Any, Any, Any, Any>::Invoke(storage, a1, a2, a3, a4, a5, a6);
    }

    AnyFn BindThis(const AnyFn::AnyFnStorage& storage, const void* this_) const override
    {
        throw Any::Exception(Any::Exception::BadOperation, "This can't be binded to static function");
    }
};

template <typename Fn, typename Ret, typename... Args>
template <bool, typename R, typename... A>
struct StaticAnyFnInvoker<Fn, Ret, Args...>::FinalInvoker
{
    inline static Any Invoke(const AnyFn::AnyFnStorage& storage, const A&... args)
    {
        throw Any::Exception(Any::Exception::BadOperation, "Bad arguments");
    }
};

template <typename Fn, typename Ret, typename... Args>
template <typename R, typename... A>
struct StaticAnyFnInvoker<Fn, Ret, Args...>::FinalInvoker<true, R, A...>
{
    inline static Any Invoke(const AnyFn::AnyFnStorage& storage, const A&... args)
    {
        Fn fn = storage.template GetAuto<Fn>();
        return Any(fn(args.template Get<Args>()...));
    };
};

template <typename Fn, typename Ret, typename... Args>
template <typename... A>
struct StaticAnyFnInvoker<Fn, Ret, Args...>::FinalInvoker<true, void, A...>
{
    inline static Any Invoke(const AnyFn::AnyFnStorage& storage, const A&... args)
    {
        Fn fn = storage.GetAuto<Fn>();
        fn(args.template Get<Args>()...);
        return Any();
    };
};

template <typename Ret, typename C, typename... Args>
struct ClassAnyFnInvoker : Invoker
{
    using Fn = Ret (C::*)(Args...);

    template <bool, typename R, typename... A>
    struct FinalInvoker;

    bool IsStatic() const override
    {
        return false;
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage) const override
    {
        return FinalInvoker<false, Ret>::Invoke(storage, Any());
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& that) const override
    {
        return FinalInvoker<0 == sizeof...(Args), Ret>::Invoke(storage, that);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& that, const Any& a1) const override
    {
        return FinalInvoker<1 == sizeof...(Args), Ret, Any>::Invoke(storage, that, a1);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& that, const Any& a1, const Any& a2) const override
    {
        return FinalInvoker<2 == sizeof...(Args), Ret, Any, Any>::Invoke(storage, that, a1, a2);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& that, const Any& a1, const Any& a2, const Any& a3) const override
    {
        return FinalInvoker<3 == sizeof...(Args), Ret, Any, Any, Any>::Invoke(storage, that, a1, a2, a3);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& that, const Any& a1, const Any& a2, const Any& a3, const Any& a4) const override
    {
        return FinalInvoker<4 == sizeof...(Args), Ret, Any, Any, Any, Any>::Invoke(storage, that, a1, a2, a3, a4);
    }

    Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& that, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) const override
    {
        return FinalInvoker<5 == sizeof...(Args), Ret, Any, Any, Any, Any, Any>::Invoke(storage, that, a1, a2, a3, a4, a5);
    }

    AnyFn BindThis(const AnyFn::AnyFnStorage& storage, const void* this_) const override
    {
        Fn fn = storage.GetAuto<Fn>();
        C* p = const_cast<C*>(static_cast<const C*>(this_));
        return AnyFn([p, fn](Args... args) -> Ret
                     {
                         return (p->*fn)(std::forward<Args>(args)...);
                     });
    }
};

template <typename Ret, typename C, typename... Args>
template <bool, typename R, typename... A>
struct ClassAnyFnInvoker<Ret, C, Args...>::FinalInvoker
{
    inline static Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& cls, const A&... args)
    {
        throw Any::Exception(Any::Exception::BadOperation, "Bad arguments");
    }
};

template <typename Ret, typename C, typename... Args>
template <typename R, typename... A>
struct ClassAnyFnInvoker<Ret, C, Args...>::FinalInvoker<true, R, A...>
{
    inline static Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& cls, const A&... args)
    {
        Fn fn = storage.GetAuto<Fn>();
        C* p = const_cast<C*>(cls.Get<const C*>());
        return Any((p->*fn)(args.template Get<Args>()...));
    }
};

template <typename Ret, typename C, typename... Args>
template <typename... A>
struct ClassAnyFnInvoker<Ret, C, Args...>::FinalInvoker<true, void, A...>
{
    inline static Any Invoke(const AnyFn::AnyFnStorage& storage, const Any& cls, const A&... args)
    {
        Fn fn = storage.GetAuto<Fn>();
        C* p = const_cast<C*>(cls.Get<const C*>());
        (p->*fn)(args.template Get<Args>()...);
        return Any();
    }
};

template <typename Fn, typename T>
struct FunctorTraits;

template <typename Fn, typename Ret, typename Cls, typename... Args>
struct FunctorTraits<Fn, Ret (Cls::*)(Args...) const>
{
    using InvokerType = StaticAnyFnInvoker<Fn, Ret, Args...>;

    static void InitInvokeParams(AnyFn::InvokeParams& params)
    {
        params.Init<Ret, Args...>();
    }
};

} // namespace AnyFnDetail

inline AnyFn::AnyFn()
    : invoker(nullptr)
{
}

template <typename Fn>
inline AnyFn::AnyFn(const Fn& fn)
{
    using FunctorTraits = AnyFnDetail::FunctorTraits<Fn, decltype(&Fn::operator())>;
    static typename FunctorTraits::InvokerType staticFnInvoker;

    anyFnStorage.SetAuto(fn);
    invoker = &staticFnInvoker;
    FunctorTraits::InitInvokeParams(invokeParams);
}

template <typename Ret, typename... Args>
inline AnyFn::AnyFn(Ret (*fn)(Args...))
{
    static AnyFnDetail::StaticAnyFnInvoker<Ret (*)(Args...), Ret, Args...> staticFnInvoker;

    anyFnStorage.SetAuto(fn);
    invoker = &staticFnInvoker;
    invokeParams.Init<Ret, Args...>();
}

template <typename Ret, typename Cls, typename... Args>
inline AnyFn::AnyFn(Ret (Cls::*fn)(Args...) const)
    : AnyFn(reinterpret_cast<Ret (Cls::*)(Args...)>(fn))
{
}

template <typename Ret, typename Cls, typename... Args>
inline AnyFn::AnyFn(Ret (Cls::*fn)(Args...))
{
    static AnyFnDetail::ClassAnyFnInvoker<Ret, Cls, Args...> classFnInvoker;

    anyFnStorage.SetAuto(fn);
    invoker = &classFnInvoker;
    invokeParams.Init<Ret, Cls*, Args...>();
}

inline bool AnyFn::IsValid() const
{
    return (nullptr != invoker);
}

inline bool AnyFn::IsStatic() const
{
    return invoker->IsStatic();
}

inline const AnyFn::InvokeParams& AnyFn::GetInvokeParams() const
{
    return invokeParams;
}

template <typename... Args>
inline Any AnyFn::Invoke(const Args&... args) const
{
    return invoker->Invoke(anyFnStorage, args...);
}

inline AnyFn AnyFn::BindThis(const void* this_) const
{
    return invoker->BindThis(anyFnStorage, this_);
}

template <typename Ret, typename... Args>
void AnyFn::InvokeParams::Init()
{
    retType = Type::Instance<Ret>();
    argsType.reserve(sizeof...(Args));

    auto args_push_back = [this](const Type* t) {
        argsType.push_back(t);
        return true;
    };

    bool unpack[] = { true, args_push_back(Type::Instance<Args>())... };
}

} // namespace DAVA
