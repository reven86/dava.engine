#pragma once
#include "Functional/Function.h"
#include "Reflection/Wrappers.h"

namespace DAVA
{
template <typename C, typename GetT, typename SetT>
class ValueWrapperClassFn : public PropertieWrapper
{
    using Getter = Function<GetT(C*)>;
    using Setter = Function<void(C*, SetT)>;

public:
    ValueWrapperClassFn(Getter getter_, Setter setter_ = nullptr)
        : PropertieWrapper()
        , getter(getter_)
        , setter(setter_)
    {
    }

    bool IsReadonly() const override
    {
        return (nullptr == setter);
    }

    const RttiType* GetType() const override
    {
        return RttiType::Instance<GetT>();
    }

    Any GetValue(const ReflectedObject& object) const override
    {
        using UnrefGetT = typename std::remove_reference<GetT>::type;

        C* cls = object.GetPtr<C>();

        Any ret;
        UnrefGetT v = getter(cls);
        ret.Set(std::move(v));
        return ret;
    }

    bool SetValue(const ReflectedObject& object, const Any& value) const override
    {
        using UnrefSetT = typename std::remove_reference<SetT>::type;

        bool ret = false;
        C* cls = object.GetPtr<C>();

        if (nullptr != setter)
        {
            const SetT& v = value.Get<UnrefSetT>();
            setter(cls, v);

            ret = true;
        }

        return ret;
    }

    ReflectedObject GetPropertieObject(const ReflectedObject& object) const override
    {
        auto is_pointer = std::integral_constant<bool, std::is_pointer<GetT>::value>();
        auto is_reference = std::integral_constant<bool, std::is_reference<GetT>::value>();

        return GetValueObjectImpl(object, is_pointer, is_reference);
    }

protected:
    Getter getter = nullptr;
    Setter setter = nullptr;

private:
    inline ReflectedObject GetValueObjectImpl(const ReflectedObject& object, std::false_type /* is_pointer */, std::false_type /* is_reference */) const
    {
        return ReflectedObject();
    }

    inline ReflectedObject GetValueObjectImpl(const ReflectedObject& object, std::true_type /* is_pointer */, std::false_type /* is_reference */) const
    {
        C* cls = object.GetPtr<C>();
        return ReflectedObject(getter(cls));
    }

    inline ReflectedObject GetValueObjectImpl(const ReflectedObject& object, std::false_type /* is_pointer */, std::true_type /* is_reference */) const
    {
        C* cls = object.GetPtr<C>();
        GetT v = getter(cls);
        return ReflectedObject(&v);
    }
};

} // namespace DAVA
