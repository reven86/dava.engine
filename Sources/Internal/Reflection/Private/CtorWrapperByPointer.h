#pragma once
#include "Reflection/Wrappers.h"

namespace DAVA
{
template <typename C, typename... Args>
class CtorWrapperByPointer : public CtorWrapper
{
public:
    CtorWrapperByPointer()
        : creator([](Args... args) { return new C(args...); })
    {
    }

    CtorWrapperByPointer(C* (*creator_)())
        : creator(creator_)
    {
        argsList.Set<void, Args...>();
    }

    CtorWrapper::Policy GetCtorPolicy() const override
    {
        return CtorWrapper::Policy::ByPointer;
    }

    const AnyFn::Params& GetInvokeParams() const override
    {
        return argsList;
    };

    Any Create() const override
    {
        auto tp = std::integral_constant<bool, 0 == sizeof...(Args)>();
        return CreateImpl(tp);
    }

    Any Create(const Any& a1) const override
    {
        auto tp = std::integral_constant<bool, 1 == sizeof...(Args)>();
        return CreateImpl(tp, a1);
    }

    Any Create(const Any& a1, const Any& a2) const override
    {
        auto tp = std::integral_constant<bool, 2 == sizeof...(Args)>();
        return CreateImpl(tp, a1, a2);
    }

    Any Create(const Any& a1, const Any& a2, const Any& a3) const override
    {
        auto tp = std::integral_constant<bool, 3 == sizeof...(Args)>();
        return CreateImpl(tp, a1, a2, a3);
    }

    Any Create(const Any& a1, const Any& a2, const Any& a3, const Any& a4) const override
    {
        auto tp = std::integral_constant<bool, 4 == sizeof...(Args)>();
        return CreateImpl(tp, a1, a2, a3, a4);
    }

    Any Create(const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) const override
    {
        auto tp = std::integral_constant<bool, 5 == sizeof...(Args)>();
        return CreateImpl(tp, a1, a2, a3, a4, a5);
    }

protected:
    C* (*creator)(Args...);
    AnyFn::Params argsList;

    template <typename... A>
    Any CreateImpl(std::true_type, A&&... args) const
    {
        C* c = (*creator)(args.template Get<Args>()...);
        return Any(c);
    }

    template <typename... A>
    Any CreateImpl(std::false_type, A&&... args) const
    {
        return Any();
    }
};

} // namespace DAVA
