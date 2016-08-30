#pragma once
#include "Reflection/Wrappers.h"

namespace DAVA
{
template <typename C, typename... Args>
class CtorWrapperDefault : public CtorWrapper
{
public:
    CtorWrapperDefault()
    {
        argsList.Set<void, Args...>();
    }

    const AnyFn::Params& GetInvokeParams() const override
    {
        return argsList;
    };

    // TODO:
    // Create policy should be added
    // Policies can be:
    //      as pointer,
    //      as value,
    //      possible as unique_ptr
    //      or shared_ptr
    // ...

    Any Create(CtorWrapper::Policy policy) const override
    {
        auto tp = std::integral_constant<bool, 0 == sizeof...(Args)>();
        return CreateImpl(tp, policy);
    }

    Any Create(CtorWrapper::Policy policy, const Any& a1) const override
    {
        auto tp = std::integral_constant<bool, 1 == sizeof...(Args)>();
        return CreateImpl(tp, policy, a1);
    }

    Any Create(CtorWrapper::Policy policy, const Any& a1, const Any& a2) const override
    {
        auto tp = std::integral_constant<bool, 2 == sizeof...(Args)>();
        return CreateImpl(tp, policy, a1, a2);
    }

    Any Create(CtorWrapper::Policy policy, const Any& a1, const Any& a2, const Any& a3) const override
    {
        auto tp = std::integral_constant<bool, 3 == sizeof...(Args)>();
        return CreateImpl(tp, policy, a1, a2, a3);
    }

    Any Create(CtorWrapper::Policy policy, const Any& a1, const Any& a2, const Any& a3, const Any& a4) const override
    {
        auto tp = std::integral_constant<bool, 4 == sizeof...(Args)>();
        return CreateImpl(tp, policy, a1, a2, a3, a4);
    }

    Any Create(CtorWrapper::Policy policy, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) const override
    {
        auto tp = std::integral_constant<bool, 5 == sizeof...(Args)>();
        return CreateImpl(tp, policy, a1, a2, a3, a4, a5);
    }

protected:
    AnyFn::Params argsList;

    template <typename... A>
    Any CreateByValue(A&&... args) const
    {
        return Any(C(args.template Get<Args>()...));
    }

    template <typename... A>
    Any CreateByPointer(A&&... args) const
    {
        C* c = new C(args.template Get<Args>()...);
        return Any(c);
    }

    template <typename... A>
    Any CreateImpl(std::true_type, CtorWrapper::Policy policy, A&&... args) const
    {
        switch (policy)
        {
        case CtorWrapper::Policy::ByValue:
            return CreateByValue(std::forward<A>(args)...);
        case CtorWrapper::Policy::ByPointer:
            return CreateByPointer(std::forward<A>(args)...);
        default:
            assert(false && "Creation policy not handled");
        }

        return Any();
    }

    template <typename... A>
    Any CreateImpl(std::false_type, CtorWrapper::Policy policy, A&&... args) const
    {
        return Any();
    }
};

} // namespace DAVA
