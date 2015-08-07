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


#ifndef __DAVAENGINE_INTERPOLATION_H__
#define __DAVAENGINE_INTERPOLATION_H__

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

namespace DAVA
{

/**
	\ingroup animationsystem
	\brief Interpolation class implements all interpolation functions supported by our animation manager.
 */
class Interpolation
{
public:
	enum FuncType
	{
		LINEAR = 0,
		//EASE
		EASE_IN,
		EASE_OUT,
		EASE_IN_EASY_OUT,
		//SINE
		SINE_IN,
		SINE_OUT,
		SINE_IN_SINE_OUT,
		//ELASTIC
		ELASTIC_IN,
		ELASTIC_OUT,
		ELASTIC_IN_ELASTIC_OUT,
		//BOUNCE
		BOUNCE_IN,
		BOUNCE_OUT,
		BOUNCE_IN_BOUNCE_OUT,

        EASY_IN, //deprecated
        EASY_OUT, //deprecated
        EASY_IN_EASY_OUT, //deprecated
        
        FUNC_TYPE_COUNT
	};
	/**
		\brief This type describes our interpolation functions. 
		It implement math equation - y = f(x) where x and y must lie in range [0; 1] 
	 */
    using Func = Function<float32(float32)>;
	/**
		\brief Return interpolation function by it's type
		\param[in] type function type you want to get 
		\returns function that implements this type of interpolation
	 */
    static Func GetFunction(FuncType type);
	
	/**
		\brief returns linear interpolation y = x
	 */
	static float32 Linear(float32 currentVal);
	/**
		\brief function with ease in
	 */
	static float32 EaseIn(float32 currentVal);
	/**
		\brief function with ease out
	 */
	static float32 EaseOut(float32 currentVal);
	/**
		\brief function with ease in & ease out
	 */
	static float32 EaseInEaseOut(float32 currentVal);

	static float32 Linear(float32 moveFrom, float32 moveTo, float32 startVal, float32 currentVal, float32 endVal);
	static float32 EaseIn(float32 moveFrom, float32 moveTo, float32 startVal, float32 currentVal, float32 endVal);
	static float32 EaseOut(float32 moveFrom, float32 moveTo, float32 startVal, float32 currentVal, float32 endVal);
	static float32 EaseInEaseOut(float32 moveFrom, float32 moveTo, float32 startVal, float32 currentVal, float32 endVal);

    /**
        \brief function with sine in
    */
    static float32 SineIn(float32 t);
    /**
        \brief function with sine out
    */
    static float32 SineOut(float32 t);
    /**
        \brief function with sine in & sine out
    */
    static float32 SineInSineOut(float32 t);

    /**
    \brief function with elastic in
    */
    static float32 ElasticIn(float32 t);
    /**
    \brief function with elastic out
    */
    static float32 ElasticOut(float32 t);
    /**
    \brief function with elastic in & elastic out
    */
    static float32 ElasticInElasticOut(float32 t);

    /**
    \brief function with bounce in
    */
    static float32 BounceOut(float32 t);
    /**
    \brief function with bounce out
    */
    static float32 BounceIn(float32 t);
    /**
    \brief function with bounce in & bounce out
    */
    static float32 BounceInBounceOut(float32 t);
	
};
	
};

#endif // __DAVAENGINE_INTERPOLATION_H__
