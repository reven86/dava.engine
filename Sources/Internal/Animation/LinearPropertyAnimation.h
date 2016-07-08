#ifndef __DAVAENGINE_LINEAR_PROPERTY_ANIMATION_H__
#define __DAVAENGINE_LINEAR_PROPERTY_ANIMATION_H__

#include "Animation/Animation.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
template <class T>
class LinearPropertyAnimation : public Animation
{
protected:
    ~LinearPropertyAnimation()
    {
    }

public:
    LinearPropertyAnimation(AnimatedObject* _owner, void* _object, const InspMember* _inspMember, const T& _startValue, const T& _endValue, float32 _animationTimeLength, Interpolation::FuncType _iFuncType);

    virtual void Update(float32 timeElapsed);

    const T& GetStartValue() const;
    const T& GetEndValue() const;

protected:
    void* object;
    const InspMember* inspMember;
    T startValue;
    T endValue;
};

template <class T>
LinearPropertyAnimation<T>::LinearPropertyAnimation(AnimatedObject* _owner, void* _object, const InspMember* _inspMember, const T& _startValue, const T& _endValue, float32 _animationTimeLength, Interpolation::FuncType _iFuncType)
    : Animation(_owner, _animationTimeLength, _iFuncType)
    , object(_object)
    , inspMember(_inspMember)
    , startValue(_startValue)
    , endValue(_endValue)
{
}

template <class T>
void LinearPropertyAnimation<T>::Update(float32 timeElapsed)
{
    Animation::Update(timeElapsed);
    T val = startValue + (endValue - startValue) * normalizedTime;
    inspMember->SetValueRaw(object, &val);
}

template <class T>
const T& LinearPropertyAnimation<T>::GetStartValue() const
{
    return startValue;
}

template <class T>
const T& LinearPropertyAnimation<T>::GetEndValue() const
{
    return endValue;
}
};

#endif
