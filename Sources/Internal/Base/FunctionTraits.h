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

#ifndef __DAVAENGINE_FUNCTION_TRAITS_H__
#define __DAVAENGINE_FUNCTION_TRAITS_H__

#include "Function.h"

namespace DAVA
{

// FuncTraits
template<typename T>
struct FuncTraits
{};

// Static functions
template<typename R>
struct FuncTraits<R(*)()>
{
	typedef R ReturnType;
	typedef Function<R()> FunctionType;
};

template<typename R, typename P1>
struct FuncTraits<R(*)(P1)>
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef Function<R (P1)> FunctionType;
};

template<typename R, typename P1, typename P2>
struct FuncTraits<R(*)(P1, P2)>
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef Function<R (P1, P2)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3>
struct FuncTraits<R(*)(P1, P2, P3)>
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef Function<R (P1, P2, P3)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct FuncTraits<R(*)(P1, P2, P3, P4)>
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef Function<R (P1, P2, P3, P4)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
struct FuncTraits<R(*)(P1, P2, P3, P4, P5)>
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef Function<R (P1, P2, P3, P4, P5)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct FuncTraits<R(*)(P1, P2, P3, P4, P5, P6)>
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
	typedef Function<R (P1, P2, P3, P4, P5, P6)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct FuncTraits<R(*)(P1, P2, P3, P4, P5, P6, P7)>
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
	typedef P7 ParamType7;
	typedef Function<R (P1, P2, P3, P4, P5, P6, P7)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncTraits<R(*)(P1, P2, P3, P4, P5, P6, P7, P8)>
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
	typedef P7 ParamType7;
	typedef P8 ParamType8;
	typedef Function<R (P1, P2, P3, P4, P5, P6, P7, P8)> FunctionType;
};

// Member functions
template<typename R, typename P1>
struct FuncTraits<R(P1::*)()>
{
	typedef R ReturnType;
	typedef P1* ParamType1;
	typedef P1 ObjType;
    typedef Function<R ()> ObjFunctionType;
	typedef Function<R (P1*)> FunctionType;
};

template<typename R, typename P1, typename P2>
struct FuncTraits<R(P1::*)(P2)>
{
	typedef R ReturnType;
	typedef P1* ParamType1;
	typedef P2 ParamType2;
	typedef P1 ObjType;
    typedef Function<R (P2)> ObjFunctionType;
	typedef Function<R (P1*, P2)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3>
struct FuncTraits<R(P1::*)(P2, P3)>
{
	typedef R ReturnType;
	typedef P1* ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P1 ObjType;
    typedef Function<R (P2, P3)> ObjFunctionType;
	typedef Function<R (P1*, P2, P3)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct FuncTraits<R(P1::*)(P2, P3, P4)>
{
	typedef R ReturnType;
	typedef P1* ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P1 ObjType;
    typedef Function<R (P2, P3, P4)> ObjFunctionType;
	typedef Function<R (P1*, P2, P3, P4)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
struct FuncTraits<R(P1::*)(P2, P3, P4, P5)>
{
	typedef R ReturnType;
	typedef P1* ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P1 ObjType;
    typedef Function<R (P2, P3, P4, P5)> ObjFunctionType;
	typedef Function<R (P1*, P2, P3, P4, P5)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct FuncTraits<R(P1::*)(P2, P3, P4, P5, P6)>
{
	typedef R ReturnType;
	typedef P1* ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
	typedef P1 ObjType;
    typedef Function<R (P2, P3, P4, P5, P6)> ObjFunctionType;
	typedef Function<R (P1*, P2, P3, P4, P5, P6)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct FuncTraits<R(P1::*)(P2, P3, P4, P5, P6, P7)>
{
	typedef R ReturnType;
	typedef P1* ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
	typedef P7 ParamType7;
	typedef P1 ObjType;
    typedef Function<R (P2, P3, P4, P5, P6, P7)> ObjFunctionType;
	typedef Function<R (P1*, P2, P3, P4, P5, P6, P7)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncTraits<R(P1::*)(P2, P3, P4, P5, P6, P7, P8)>
{
	typedef R ReturnType;
	typedef P1* ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
	typedef P7 ParamType7;
	typedef P8 ParamType8;
	typedef P1 ObjType;
    typedef Function<R (P2, P3, P4, P5, P6, P7, P8)> ObjFunctionType;
	typedef Function<R (P1*, P2, P3, P4, P5, P6, P7, P8)> FunctionType;
};

// Member const functions
template<typename R, typename P1>
struct FuncTraits<R(P1::*)() const>
{
    typedef R ReturnType;
    typedef const P1* ParamType1;
    typedef P1 ObjType;
    typedef Function<R()> ObjFunctionType;
    typedef Function<R(P1*)> FunctionType;
};

template<typename R, typename P1, typename P2>
struct FuncTraits<R(P1::*)(P2) const>
{
    typedef R ReturnType;
    typedef const P1* ParamType1;
    typedef P2 ParamType2;
    typedef P1 ObjType;
    typedef Function<R(P2)> ObjFunctionType;
    typedef Function<R(P1*, P2)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3>
struct FuncTraits<R(P1::*)(P2, P3) const>
{
    typedef R ReturnType;
    typedef const P1* ParamType1;
    typedef P2 ParamType2;
    typedef P3 ParamType3;
    typedef P1 ObjType;
    typedef Function<R(P2, P3)> ObjFunctionType;
    typedef Function<R(P1*, P2, P3)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct FuncTraits<R(P1::*)(P2, P3, P4) const>
{
    typedef R ReturnType;
    typedef const P1* ParamType1;
    typedef P2 ParamType2;
    typedef P3 ParamType3;
    typedef P4 ParamType4;
    typedef P1 ObjType;
    typedef Function<R(P2, P3, P4)> ObjFunctionType;
    typedef Function<R(P1*, P2, P3, P4)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
struct FuncTraits<R(P1::*)(P2, P3, P4, P5) const>
{
    typedef R ReturnType;
    typedef const P1* ParamType1;
    typedef P2 ParamType2;
    typedef P3 ParamType3;
    typedef P4 ParamType4;
    typedef P5 ParamType5;
    typedef P1 ObjType;
    typedef Function<R(P2, P3, P4, P5)> ObjFunctionType;
    typedef Function<R(P1*, P2, P3, P4, P5)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct FuncTraits<R(P1::*)(P2, P3, P4, P5, P6) const>
{
    typedef R ReturnType;
    typedef const P1* ParamType1;
    typedef P2 ParamType2;
    typedef P3 ParamType3;
    typedef P4 ParamType4;
    typedef P5 ParamType5;
    typedef P6 ParamType6;
    typedef P1 ObjType;
    typedef Function<R(P2, P3, P4, P5, P6)> ObjFunctionType;
    typedef Function<R(P1*, P2, P3, P4, P5, P6)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct FuncTraits<R(P1::*)(P2, P3, P4, P5, P6, P7) const>
{
    typedef R ReturnType;
    typedef const P1* ParamType1;
    typedef P2 ParamType2;
    typedef P3 ParamType3;
    typedef P4 ParamType4;
    typedef P5 ParamType5;
    typedef P6 ParamType6;
    typedef P7 ParamType7;
    typedef P1 ObjType;
    typedef Function<R(P2, P3, P4, P5, P6, P7)> ObjFunctionType;
    typedef Function<R(P1*, P2, P3, P4, P5, P6, P7)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncTraits<R(P1::*)(P2, P3, P4, P5, P6, P7, P8) const>
{
    typedef R ReturnType;
    typedef const P1* ParamType1;
    typedef P2 ParamType2;
    typedef P3 ParamType3;
    typedef P4 ParamType4;
    typedef P5 ParamType5;
    typedef P6 ParamType6;
    typedef P7 ParamType7;
    typedef P8 ParamType8;
    typedef P1 ObjType;
    typedef Function<R(P2, P3, P4, P5, P6, P7, P8)> ObjFunctionType;
    typedef Function<R(P1*, P2, P3, P4, P5, P6, P7, P8)> FunctionType;
};

// Function class
template<typename R>
struct FuncTraits< Function<R()> >
{
	typedef R ReturnType;
	typedef Function<R()> FunctionType;
};

template<typename R, typename P1>
struct FuncTraits< Function<R(P1)> >
{
	typedef R ReturnType;
	typedef P1 ParamType1;
    typedef typename PointerTraits<P1>::PointerType ObjType;
	typedef Function<R(P1)> FunctionType;
};

template<typename R, typename P1, typename P2>
struct FuncTraits< Function<R(P1, P2)> >
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
    typedef typename PointerTraits<P1>::PointerType ObjType;
    typedef Function<R(P1, P2)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3>
struct FuncTraits< Function<R(P1, P2, P3)> >
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
    typedef typename PointerTraits<P1>::PointerType ObjType;
    typedef Function<R(P1, P2, P3)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct FuncTraits< Function<R(P1, P2, P3, P4)> >
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
    typedef typename PointerTraits<P1>::PointerType ObjType;
    typedef Function<R(P1, P2, P3, P4)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
struct FuncTraits< Function<R(P1, P2, P3, P4, P5)> >
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
    typedef typename PointerTraits<P1>::PointerType ObjType;
    typedef Function<R(P1, P2, P3, P4, P5)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct FuncTraits< Function<R(P1, P2, P3, P4, P5, P6)> >
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
    typedef typename PointerTraits<P1>::PointerType ObjType;
    typedef Function<R(P1, P2, P3, P4, P5, P6)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct FuncTraits< Function<R(P1, P2, P3, P4, P5, P6, P7)> >
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
	typedef P7 ParamType7;
    typedef typename PointerTraits<P1>::PointerType ObjType;
    typedef Function<R(P1, P2, P3, P4, P5, P6, P7)> FunctionType;
};

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncTraits< Function<R(P1, P2, P3, P4, P5, P6, P7, P8)> >
{
	typedef R ReturnType;
	typedef P1 ParamType1;
	typedef P2 ParamType2;
	typedef P3 ParamType3;
	typedef P4 ParamType4;
	typedef P5 ParamType5;
	typedef P6 ParamType6;
	typedef P7 ParamType7;
	typedef P8 ParamType8;
    typedef typename PointerTraits<P1>::PointerType ObjType;
    typedef Function<R(P1, P2, P3, P4, P5, P6, P7, P8)> FunctionType;
};

// Make Function Helpers
template<typename F> 
typename FuncTraits<F>::FunctionType MakeFunction(const F &fn) 
{ 
	return typename FuncTraits<F>::FunctionType(fn);
}

template<typename F>
typename FuncTraits<F>::ObjFunctionType MakeFunction(typename FuncTraits<F>::ParamType1 obj, const F& fn)
{
    return typename FuncTraits<F>::ObjFunctionType(obj, fn);
}

template<typename F, typename T>
typename FuncTraits<F>::ObjFunctionType MakeFunction(const PointerWrapper<T> &owner, const F& fn)
{
    return typename FuncTraits<F>::ObjFunctionType(owner, fn);
}

} // namespace DAVA

#endif // __DAVAENGINE_FUNCTION_TRAITS_H__
