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

#ifndef __DAVAENGINE_TYPELIST_H__
#define __DAVAENGINE_TYPELIST_H__

#include "NullType.h"

namespace DAVA
{

// type list
template <class T, class U>
struct Typelist
{
	typedef T Head;
	typedef U Tail;
};

// type list operations
namespace TL
{
	// append type
	template <class TList, class T>
	struct Append;

	template <>
	struct Append<NullType, NullType>
	{
		typedef NullType Result;
	};

	template <class T>
	struct Append<NullType, T>
	{
		typedef Typelist<T, NullType> Result;
	};

	template <class Head, class Tail>
	struct Append<NullType, Typelist<Head, Tail> >
	{
		typedef Typelist<Head, Tail> Result;
	};

	template <class Head, class Tail, class T>
	struct Append<Typelist<Head, Tail>, T>
	{
		typedef Typelist<Head, typename Append<Tail, T>::Result> Result;
	};

	template <class Head, class Tail>
	struct Append<Typelist<Head, Tail>, NullType>
	{
		typedef Typelist<Head, Tail> Result;
	};

	// type at given index
	template <class TList, unsigned int index>
	struct TypeAt;

	template <class Head, class Tail>
	struct TypeAt<Typelist<Head, Tail>, 0>
	{
		typedef Head Result;
	};

	template <class Head, class Tail, unsigned int i>
	struct TypeAt<Typelist<Head, Tail>, i>
	{
		typedef typename TypeAt<Tail, i - 1>::Result Result;
	};

	// type at given index with default type, when no such index 
	template <class TList, unsigned int index, typename DefaultType = NullType>
	struct TypeAtNonStrict
	{
		typedef DefaultType Result;
	};

	template <class Head, class Tail, typename DefaultType>
	struct TypeAtNonStrict<Typelist<Head, Tail>, 0, DefaultType>
	{
		typedef Head Result;
	};

	template <class Head, class Tail, unsigned int i, typename DefaultType>
	struct TypeAtNonStrict<Typelist<Head, Tail>, i, DefaultType>
	{
		typedef typename TypeAtNonStrict<Tail, i - 1, DefaultType>::Result Result;
	};

    // finds the index of a given type
    template <class TList, class T> struct IndexOf;

    template <class T>
    struct IndexOf<NullType, T>
    {
        enum { value = -1 };
    };

    template <class T, class Tail>
    struct IndexOf<Typelist<T, Tail>, T>
    {
        enum { value = 0 };
    };

    template <class Head, class Tail, class T>
    struct IndexOf<Typelist<Head, Tail>, T>
    {
    private:
        enum { temp = IndexOf<Tail, T>::value };
    public:
        enum { value = (temp == -1 ? -1 : 1 + temp) };
    };
};

}

#define DAVA_TYPELIST_1(T1) DAVA::Typelist<T1, NullType>
#define DAVA_TYPELIST_2(T1, T2) DAVA::Typelist<T1, DAVA_TYPELIST_1(T2) >
#define DAVA_TYPELIST_3(T1, T2, T3) DAVA::Typelist<T1, DAVA_TYPELIST_2(T2, T3) >
#define DAVA_TYPELIST_4(T1, T2, T3, T4) DAVA::Typelist<T1, DAVA_TYPELIST_3(T2, T3, T4) >
#define DAVA_TYPELIST_5(T1, T2, T3, T4, T5) DAVA::Typelist<T1, DAVA_TYPELIST_4(T2, T3, T4, T5) >
#define DAVA_TYPELIST_6(T1, T2, T3, T4, T5, T6) DAVA::Typelist<T1, DAVA_TYPELIST_5(T2, T3, T4, T5, T6) >
#define DAVA_TYPELIST_7(T1, T2, T3, T4, T5, T6, T7) DAVA::Typelist<T1, DAVA_TYPELIST_6(T2, T3, T4, T5, T6, T7) >
#define DAVA_TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8) DAVA::Typelist<T1, DAVA_TYPELIST_7(T2, T3, T4, T5, T6, T7, T8) >
#define DAVA_TYPELIST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) DAVA::Typelist<T1, DAVA_TYPELIST_8(T2, T3, T4, T5, T6, T7, T8, T9) >
#define DAVA_TYPELIST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) DAVA::Typelist<T1, DAVA_TYPELIST_9(T2, T3, T4, T5, T6, T7, T8, T9, T10) >
#define DAVA_TYPELIST_11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) DAVA::Typelist<T1, DAVA_TYPELIST_10(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) >
#define DAVA_TYPELIST_12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) DAVA::Typelist<T1, DAVA_TYPELIST_11(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) >
#define DAVA_TYPELIST_13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) DAVA::Typelist<T1, DAVA_TYPELIST_12(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) >
#define DAVA_TYPELIST_14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) DAVA::Typelist<T1, DAVA_TYPELIST_13(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) >
#define DAVA_TYPELIST_15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) DAVA::Typelist<T1, DAVA_TYPELIST_14(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) >
#define DAVA_TYPELIST_16(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) DAVA::Typelist<T1, DAVA_TYPELIST_15(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) >

#endif // __DAVAENGINE_TYPELIST_H__
