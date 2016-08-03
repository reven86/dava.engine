#pragma once
#include "Reflection/Public/Wrappers.h"

namespace DAVA
{
template <typename C, typename... Args>
class CtorWrapperDefault : public CtorWrapper
{
public:
    CtorWrapperDefault()
    {
        argsList.Init<void, Args...>();
    }

    const AnyFn::InvokeParams& GetInvokeParams() const override
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
    AnyFn::InvokeParams argsList;

    template <typename... A>
    Any CreateInvoke(A&&... args) const
    {
        C* c = new C(args.template Get<Args>()...);
        return Any(c);
    }

    template <typename... A>
    Any CreateImpl(std::true_type, A&&... args) const
    {
        return CreateInvoke(std::forward<A>(args)...);
    }

    template <typename... A>
    Any CreateImpl(std::false_type, A&&... args) const
    {
        return Any();
    }
};

} // namespace DAVA
