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


#ifndef __DAVAENGINE_MATH2D_H__
#define __DAVAENGINE_MATH2D_H__

//!	
//! All 2D & 3D math represent vectors & points (2D eq to vector) as vector array
//! for example 
//!	Math::Point2 < float32 > p; // [x y] (or [x y 1] for 3x3 matrix equations)
//! 
#include "Base/BaseTypes.h"
#include "Math/Math2DTemplateClasses.h"


// definition of basic 2D types
namespace DAVA
{

#define FLOAT_EQUAL(f1, f2) (fabsf(f1 - f2) < EPSILON)
#define FLOAT_EQUAL_EPS(f1, f2, EPS) (fabsf(f1 - f2) < EPS)

inline float32 FloatClamp(float32 min, float32 max, float32 val);


/*
    Helper classes designed mostly for internal framework usage
    in all general cases use Vector2, Rect and other classes instead
 */
 //! int Point2 
using Point2i = Point2Base<int32>;
//! float Point2 
using Point2f = Point2Base<float32>;
//! int Size2
using Size2i = Size2Base<int32>;
//! float Size2
using Size2f = Size2Base<float32>;
//! int Rect2
using Rect2i = Rect2Base<int32>;
//! float Rect2
using Rect2f = Rect2Base<float32>;


// Implementations
inline float32 FloatClamp(float32 min, float32 max, float32 val)
{
    if (val > max)val = max;
        if (val < min)val = min;
            return val;
};

/**
    \brief Fast function to compute index of bit that is set in a value. Only one bit should be set to make it work correctly.
 */


#ifdef __GNUC__
//     #define CountLeadingZeros(x) __builtin_clz(x)
//     #define CountTrailingZeros(x) __builtin_ctz(x)
#define FastLog2(x) __builtin_ctz(x)
#else
//     inline uint32 popcnt( uint32 x )
//     {
//         x -= ((x >> 1) & 0x55555555);
//         x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
//         x = (((x >> 4) + x) & 0x0f0f0f0f);
//         x += (x >> 8);
//         x += (x >> 16);
//         return x & 0x0000003f;
//     }
//     inline uint32 CountLeadingZeros( uint32 x )
//     {
//         x |= (x >> 1);
//         x |= (x >> 2);
//         x |= (x >> 4);
//         x |= (x >> 8);
//         x |= (x >> 16);
//         return 32 - popcnt(x);
//     }
//     inline uint32 CountTrailingZeros( uint32 x )
//     {
//         return popcnt((x & -x) - 1);
//     }
extern const int MultiplyDeBruijnBitPosition2[32];
#define FastLog2(value) MultiplyDeBruijnBitPosition2[(uint32)(value * 0x077CB531U) >> 27]
#endif


};

#endif	//__DAVAENGINE_MATH2D_H__

