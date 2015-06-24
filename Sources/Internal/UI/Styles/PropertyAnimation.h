/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_PROPERTY_ANIMATION_H__
#define __DAVAENGINE_PROPERTY_ANIMATION_H__

#include "Animation/Animation.h"
#include "Base/BaseTypes.h"

namespace DAVA
{

template<class T>
class PropertyAnimation : public Animation
{
protected:
    ~PropertyAnimation(){}
public:
    PropertyAnimation(AnimatedObject * _owner, void* _object, const InspMember* _inspMember, T _startValue, T _endValue, float32 _animationTimeLength, Interpolation::FuncType _iFuncType);

    virtual void Update(float32 timeElapsed);
protected:
    void* object;
    const InspMember* inspMember;
    T startValue;
    T endValue;
};

template<class T>
PropertyAnimation<T>::PropertyAnimation(AnimatedObject * _owner, void* _object, const InspMember* _inspMember, T _startValue, T _endValue, float32 _animationTimeLength, Interpolation::FuncType _iFuncType)
    : Animation(_owner, _animationTimeLength, _iFuncType)
    , object(_object)
    , inspMember(_inspMember)
    , startValue(_startValue)
    , endValue(_endValue)
{

}

template<class T>
void PropertyAnimation<T>::Update(float32 timeElapsed)
{
    Animation::Update(timeElapsed);
    T val = startValue + (endValue - startValue) * normalizedTime;
    inspMember->SetValueRaw(object, &val);
}

};

#endif
