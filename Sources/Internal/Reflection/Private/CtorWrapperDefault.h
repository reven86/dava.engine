#pragma once
#include "Reflection/ReflectionWrappers.h"

#if !defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
template <typename C, typename... Args>
class CtorWrapperDefault : public CtorWrapper
{
public:
    CtorWrapperDefault()
    {
        auto tp = std::integral_constant<bool, 0 != sizeof...(Args)>();
        Fill<Args...>(tp);
    }

    const Vector<const Type*>& GetParamsList() const override
    {
        return paramsList;
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

protected:
    Vector<const Type*> paramsList;

    template <typename... A>
    void Fill(std::false_type)
    {
    }

    template <typename T, typename... A>
    void Fill(std::true_type)
    {
        paramsList.push_back(Type::Instance<T>());

        auto tp = std::integral_constant<bool, 0 != sizeof...(A)>();
        Fill<A...>(tp);
    }

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

#endif