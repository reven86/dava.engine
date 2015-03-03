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

#ifndef __DAVA_FUNCTION_H__
#define __DAVA_FUNCTION_H__

#include "TypeHolders.h"
#include <type_traits>

namespace DAVA
{
	// ===================================================================================================================
	// FunctionBase class 
	// TODO: description
	// ===================================================================================================================
	template<typename R, typename P1 = NullType, typename P2 = NullType, typename P3 = NullType, typename P4 = NullType, typename P5 = NullType, typename P6 = NullType, typename P7 = NullType, typename P8 = NullType>
	class FunctionBase
	{
	public:
		typedef enum {} IsFunctionType;

		typedef R ReturnType;
		typedef P1 ParamType1;
		typedef P2 ParamType2;
		typedef P3 ParamType3;
		typedef P4 ParamType4;
		typedef P5 ParamType5;
		typedef P6 ParamType6;
		typedef P7 ParamType7;
		typedef P8 ParamType8;

        // incoming class type
        typedef typename std::remove_pointer<P1>::type Check1;
        typedef typename std::remove_cv<Check1>::type Check2;
        typedef typename std::conditional<std::is_class<Check2>::value, Check2, NullType>::type C;

		// evaluating best functor argument types
		typedef typename TypeTraits<P1>::ParamType ParamRefType1;
		typedef typename TypeTraits<P2>::ParamType ParamRefType2;
		typedef typename TypeTraits<P3>::ParamType ParamRefType3;
		typedef typename TypeTraits<P4>::ParamType ParamRefType4;
		typedef typename TypeTraits<P5>::ParamType ParamRefType5;
		typedef typename TypeTraits<P6>::ParamType ParamRefType6;
		typedef typename TypeTraits<P7>::ParamType ParamRefType7;
		typedef typename TypeTraits<P8>::ParamType ParamRefType8;

		// available static function types
		typedef R(*StaticFunctionType0)();
		typedef R(*StaticFunctionType1)(P1);
		typedef R(*StaticFunctionType2)(P1, P2);
		typedef R(*StaticFunctionType3)(P1, P2, P3);
		typedef R(*StaticFunctionType4)(P1, P2, P3, P4);
		typedef R(*StaticFunctionType5)(P1, P2, P3, P4, P5);
		typedef R(*StaticFunctionType6)(P1, P2, P3, P4, P5, P6);
		typedef R(*StaticFunctionType7)(P1, P2, P3, P4, P5, P6, P7);
		typedef R(*StaticFunctionType8)(P1, P2, P3, P4, P5, P6, P7, P8);

		// evaluating incoming static function type
		typedef StaticFunctionType0 IncomingStaticFunctionType0;
		typedef typename Select<IsNullType<P1>::result, IncomingStaticFunctionType0, StaticFunctionType1>::Result IncomingStaticFunctionType1;
		typedef typename Select<IsNullType<P2>::result, IncomingStaticFunctionType1, StaticFunctionType2>::Result IncomingStaticFunctionType2;
		typedef typename Select<IsNullType<P3>::result, IncomingStaticFunctionType2, StaticFunctionType3>::Result IncomingStaticFunctionType3;
		typedef typename Select<IsNullType<P4>::result, IncomingStaticFunctionType3, StaticFunctionType4>::Result IncomingStaticFunctionType4;
		typedef typename Select<IsNullType<P5>::result, IncomingStaticFunctionType4, StaticFunctionType5>::Result IncomingStaticFunctionType5;
		typedef typename Select<IsNullType<P6>::result, IncomingStaticFunctionType5, StaticFunctionType6>::Result IncomingStaticFunctionType6;
		typedef typename Select<IsNullType<P7>::result, IncomingStaticFunctionType6, StaticFunctionType7>::Result IncomingStaticFunctionType7;
		typedef typename Select<IsNullType<P8>::result, IncomingStaticFunctionType7, StaticFunctionType8>::Result IncomingStaticFunctionType8;
		typedef IncomingStaticFunctionType8 IncomingStaticFunctionType; // evaluated result

		// available class member function types
		typedef R(C::*ClassFunctionType1)();
		typedef R(C::*ClassFunctionType2)(P2);
		typedef R(C::*ClassFunctionType3)(P2, P3);
		typedef R(C::*ClassFunctionType4)(P2, P3, P4);
		typedef R(C::*ClassFunctionType5)(P2, P3, P4, P5);
		typedef R(C::*ClassFunctionType6)(P2, P3, P4, P5, P6);
		typedef R(C::*ClassFunctionType7)(P2, P3, P4, P5, P6, P7);
		typedef R(C::*ClassFunctionType8)(P2, P3, P4, P5, P6, P7, P8);

		typedef R(C::*ClassFunctionConstType1)() const;
		typedef R(C::*ClassFunctionConstType2)(P2) const;
		typedef R(C::*ClassFunctionConstType3)(P2, P3) const;
		typedef R(C::*ClassFunctionConstType4)(P2, P3, P4) const;
		typedef R(C::*ClassFunctionConstType5)(P2, P3, P4, P5) const;
		typedef R(C::*ClassFunctionConstType6)(P2, P3, P4, P5, P6) const;
		typedef R(C::*ClassFunctionConstType7)(P2, P3, P4, P5, P6, P7) const;
		typedef R(C::*ClassFunctionConstType8)(P2, P3, P4, P5, P6, P7, P8) const;

		// evaluating incoming class member function type
		typedef ClassFunctionType1 IncomingClassFunctionType1;
		typedef typename Select<IsNullType<P2>::result, IncomingClassFunctionType1, ClassFunctionType2>::Result IncomingClassFunctionType2;
		typedef typename Select<IsNullType<P3>::result, IncomingClassFunctionType2, ClassFunctionType3>::Result IncomingClassFunctionType3;
		typedef typename Select<IsNullType<P4>::result, IncomingClassFunctionType3, ClassFunctionType4>::Result IncomingClassFunctionType4;
		typedef typename Select<IsNullType<P5>::result, IncomingClassFunctionType4, ClassFunctionType5>::Result IncomingClassFunctionType5;
		typedef typename Select<IsNullType<P6>::result, IncomingClassFunctionType5, ClassFunctionType6>::Result IncomingClassFunctionType6;
		typedef typename Select<IsNullType<P7>::result, IncomingClassFunctionType6, ClassFunctionType7>::Result IncomingClassFunctionType7;
		typedef typename Select<IsNullType<P8>::result, IncomingClassFunctionType7, ClassFunctionType8>::Result IncomingClassFunctionType8;
		typedef IncomingClassFunctionType8 IncomingClassFunctionType; // evaluated result

		// evaluating incoming class member function const type
		typedef ClassFunctionConstType1 IncomingClassFunctionConstType1;
		typedef typename Select<IsNullType<P2>::result, IncomingClassFunctionConstType1, ClassFunctionConstType2>::Result IncomingClassFunctionConstType2;
		typedef typename Select<IsNullType<P3>::result, IncomingClassFunctionConstType2, ClassFunctionConstType3>::Result IncomingClassFunctionConstType3;
		typedef typename Select<IsNullType<P4>::result, IncomingClassFunctionConstType3, ClassFunctionConstType4>::Result IncomingClassFunctionConstType4;
		typedef typename Select<IsNullType<P5>::result, IncomingClassFunctionConstType4, ClassFunctionConstType5>::Result IncomingClassFunctionConstType5;
		typedef typename Select<IsNullType<P6>::result, IncomingClassFunctionConstType5, ClassFunctionConstType6>::Result IncomingClassFunctionConstType6;
		typedef typename Select<IsNullType<P7>::result, IncomingClassFunctionConstType6, ClassFunctionConstType7>::Result IncomingClassFunctionConstType7;
		typedef typename Select<IsNullType<P8>::result, IncomingClassFunctionConstType7, ClassFunctionConstType8>::Result IncomingClassFunctionConstType8;
		typedef IncomingClassFunctionConstType8 IncomingClassFunctionConstType; // evaluated result

		template<typename O>
		struct ObjectType
		{
			typedef R(O::*ObjectFunctionType0)();
			typedef R(O::*ObjectFunctionType1)(P1);
			typedef R(O::*ObjectFunctionType2)(P1, P2);
			typedef R(O::*ObjectFunctionType3)(P1, P2, P3);
			typedef R(O::*ObjectFunctionType4)(P1, P2, P3, P4);
			typedef R(O::*ObjectFunctionType5)(P1, P2, P3, P4, P5);
			typedef R(O::*ObjectFunctionType6)(P1, P2, P3, P4, P5, P6);
			typedef R(O::*ObjectFunctionType7)(P1, P2, P3, P4, P5, P6, P7);
			typedef R(O::*ObjectFunctionType8)(P1, P2, P3, P4, P5, P6, P7, P8);

			typedef R(O::*ObjectFunctionConstType0)() const;
			typedef R(O::*ObjectFunctionConstType1)(P1) const;
			typedef R(O::*ObjectFunctionConstType2)(P1, P2) const;
			typedef R(O::*ObjectFunctionConstType3)(P1, P2, P3) const;
			typedef R(O::*ObjectFunctionConstType4)(P1, P2, P3, P4) const;
			typedef R(O::*ObjectFunctionConstType5)(P1, P2, P3, P4, P5) const;
			typedef R(O::*ObjectFunctionConstType6)(P1, P2, P3, P4, P5, P6) const;
			typedef R(O::*ObjectFunctionConstType7)(P1, P2, P3, P4, P5, P6, P7) const;
			typedef R(O::*ObjectFunctionConstType8)(P1, P2, P3, P4, P5, P6, P7, P8) const;

			// evaluating incoming object member function type
			typedef ObjectFunctionType0 IncomingObjectFunctionType0;
			typedef typename Select<IsNullType<P1>::result, IncomingObjectFunctionType0, ObjectFunctionType1>::Result IncomingObjectFunctionType1;
			typedef typename Select<IsNullType<P2>::result, IncomingObjectFunctionType1, ObjectFunctionType2>::Result IncomingObjectFunctionType2;
			typedef typename Select<IsNullType<P3>::result, IncomingObjectFunctionType2, ObjectFunctionType3>::Result IncomingObjectFunctionType3;
			typedef typename Select<IsNullType<P4>::result, IncomingObjectFunctionType3, ObjectFunctionType4>::Result IncomingObjectFunctionType4;
			typedef typename Select<IsNullType<P5>::result, IncomingObjectFunctionType4, ObjectFunctionType5>::Result IncomingObjectFunctionType5;
			typedef typename Select<IsNullType<P6>::result, IncomingObjectFunctionType5, ObjectFunctionType6>::Result IncomingObjectFunctionType6;
			typedef typename Select<IsNullType<P7>::result, IncomingObjectFunctionType6, ObjectFunctionType7>::Result IncomingObjectFunctionType7;
			typedef typename Select<IsNullType<P8>::result, IncomingObjectFunctionType7, ObjectFunctionType8>::Result IncomingObjectFunctionType8;
			typedef IncomingObjectFunctionType8 IncomingObjectFunctionType; // evaluated result

			// evaluating incoming object member function const type
			typedef ObjectFunctionConstType0 IncomingObjectFunctionConstType0;
			typedef typename Select<IsNullType<P1>::result, IncomingObjectFunctionConstType0, ObjectFunctionConstType1>::Result IncomingObjectFunctionConstType1;
			typedef typename Select<IsNullType<P2>::result, IncomingObjectFunctionConstType1, ObjectFunctionConstType2>::Result IncomingObjectFunctionConstType2;
			typedef typename Select<IsNullType<P3>::result, IncomingObjectFunctionConstType2, ObjectFunctionConstType3>::Result IncomingObjectFunctionConstType3;
			typedef typename Select<IsNullType<P4>::result, IncomingObjectFunctionConstType3, ObjectFunctionConstType4>::Result IncomingObjectFunctionConstType4;
			typedef typename Select<IsNullType<P5>::result, IncomingObjectFunctionConstType4, ObjectFunctionConstType5>::Result IncomingObjectFunctionConstType5;
			typedef typename Select<IsNullType<P6>::result, IncomingObjectFunctionConstType5, ObjectFunctionConstType6>::Result IncomingObjectFunctionConstType6;
			typedef typename Select<IsNullType<P7>::result, IncomingObjectFunctionConstType6, ObjectFunctionConstType7>::Result IncomingObjectFunctionConstType7;
			typedef typename Select<IsNullType<P8>::result, IncomingObjectFunctionConstType7, ObjectFunctionConstType8>::Result IncomingObjectFunctionConstType8;
			typedef IncomingObjectFunctionConstType8 IncomingObjectFunctionConstType; // evaluated result
		};

		// default empty constructor
		FunctionBase() { invoker0 = NULL; }

		// constructors for static functions
		FunctionBase(StaticFunctionType0 fn) : fnPointerHolder(fn) { invoker0 = &StaticInvokerImpl0; }
		FunctionBase(StaticFunctionType1 fn) : fnPointerHolder(fn) { invoker1 = &StaticInvokerImpl1; }
		FunctionBase(StaticFunctionType2 fn) : fnPointerHolder(fn) { invoker2 = &StaticInvokerImpl2; }
		FunctionBase(StaticFunctionType3 fn) : fnPointerHolder(fn) { invoker3 = &StaticInvokerImpl3; }
		FunctionBase(StaticFunctionType4 fn) : fnPointerHolder(fn) { invoker4 = &StaticInvokerImpl4; }
		FunctionBase(StaticFunctionType5 fn) : fnPointerHolder(fn) { invoker5 = &StaticInvokerImpl5; }
		FunctionBase(StaticFunctionType6 fn) : fnPointerHolder(fn) { invoker6 = &StaticInvokerImpl6; }
		FunctionBase(StaticFunctionType7 fn) : fnPointerHolder(fn) { invoker7 = &StaticInvokerImpl7; }
		FunctionBase(StaticFunctionType8 fn) : fnPointerHolder(fn) { invoker8 = &StaticInvokerImpl8; }

		// constructors for class member functions
		FunctionBase(ClassFunctionType1 fn) : fnPointerHolder(fn) { invoker1 = &ClassInvokerImpl1; }
		FunctionBase(ClassFunctionType2 fn) : fnPointerHolder(fn) { invoker2 = &ClassInvokerImpl2; }
		FunctionBase(ClassFunctionType3 fn) : fnPointerHolder(fn) { invoker3 = &ClassInvokerImpl3; }
		FunctionBase(ClassFunctionType4 fn) : fnPointerHolder(fn) { invoker4 = &ClassInvokerImpl4; }
		FunctionBase(ClassFunctionType5 fn) : fnPointerHolder(fn) { invoker5 = &ClassInvokerImpl5; }
		FunctionBase(ClassFunctionType6 fn) : fnPointerHolder(fn) { invoker6 = &ClassInvokerImpl6; }
		FunctionBase(ClassFunctionType7 fn) : fnPointerHolder(fn) { invoker7 = &ClassInvokerImpl7; }
		FunctionBase(ClassFunctionType8 fn) : fnPointerHolder(fn) { invoker8 = &ClassInvokerImpl8; }

		// constructors for class member const functions
		FunctionBase(ClassFunctionConstType1 fn) : fnPointerHolder(fn) { invoker1 = &ClassInvokerImpl1; }
		FunctionBase(ClassFunctionConstType2 fn) : fnPointerHolder(fn) { invoker2 = &ClassInvokerImpl2; }
		FunctionBase(ClassFunctionConstType3 fn) : fnPointerHolder(fn) { invoker3 = &ClassInvokerImpl3; }
		FunctionBase(ClassFunctionConstType4 fn) : fnPointerHolder(fn) { invoker4 = &ClassInvokerImpl4; }
		FunctionBase(ClassFunctionConstType5 fn) : fnPointerHolder(fn) { invoker5 = &ClassInvokerImpl5; }
		FunctionBase(ClassFunctionConstType6 fn) : fnPointerHolder(fn) { invoker6 = &ClassInvokerImpl6; }
		FunctionBase(ClassFunctionConstType7 fn) : fnPointerHolder(fn) { invoker7 = &ClassInvokerImpl7; }
		FunctionBase(ClassFunctionConstType8 fn) : fnPointerHolder(fn) { invoker8 = &ClassInvokerImpl8; }

		// constructors for object member functions
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType0 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker0 = &ObjectInvokerImpl0<O>; }
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType1 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker1 = &ObjectInvokerImpl1<O>; }
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType2 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker2 = &ObjectInvokerImpl2<O>; }
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType3 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker3 = &ObjectInvokerImpl3<O>; }
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType4 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker4 = &ObjectInvokerImpl4<O>; }
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType5 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker5 = &ObjectInvokerImpl5<O>; }
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType6 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker6 = &ObjectInvokerImpl6<O>; }
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType7 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker7 = &ObjectInvokerImpl7<O>; }
		template<typename O> FunctionBase(O* obj, typename ObjectType<O>::ObjectFunctionType8 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker8 = &ObjectInvokerImpl8<O>; }

		// constructors for object member const functions
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType0 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker0 = &ObjectInvokerImpl0<O>; }
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType1 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker1 = &ObjectInvokerImpl1<O>; }
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType2 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker2 = &ObjectInvokerImpl2<O>; }
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType3 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker3 = &ObjectInvokerImpl3<O>; }
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType4 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker4 = &ObjectInvokerImpl4<O>; }
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType5 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker5 = &ObjectInvokerImpl5<O>; }
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType6 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker6 = &ObjectInvokerImpl6<O>; }
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType7 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker7 = &ObjectInvokerImpl7<O>; }
		template<typename O> FunctionBase(const O* obj, typename ObjectType<O>::ObjectFunctionConstType8 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker8 = &ObjectInvokerImpl8<O>; }

        // constructors for object member functions
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType0 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker0 = &ObjectInvokerImpl0<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType1 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker1 = &ObjectInvokerImpl1<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType2 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker2 = &ObjectInvokerImpl2<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType3 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker3 = &ObjectInvokerImpl3<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType4 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker4 = &ObjectInvokerImpl4<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType5 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker5 = &ObjectInvokerImpl5<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType6 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker6 = &ObjectInvokerImpl6<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType7 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker7 = &ObjectInvokerImpl7<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionType8 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker8 = &ObjectInvokerImpl8<O>; }

        // constructors for object member const functions
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType0 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker0 = &ObjectInvokerImpl0<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType1 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker1 = &ObjectInvokerImpl1<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType2 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker2 = &ObjectInvokerImpl2<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType3 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker3 = &ObjectInvokerImpl3<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType4 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker4 = &ObjectInvokerImpl4<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType5 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker5 = &ObjectInvokerImpl5<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType6 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker6 = &ObjectInvokerImpl6<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType7 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker7 = &ObjectInvokerImpl7<O>; }
        template<typename O> FunctionBase(const PointerWrapper<O> &obj, typename ObjectType<O>::ObjectFunctionConstType8 fn) : fnPointerHolder(fn), objPointerHolder(obj) { invoker8 = &ObjectInvokerImpl8<O>; }

		// operators
		bool operator==(const FunctionBase &f) const { return (fnPointerHolder == f.fnPointerHolder && objPointerHolder.object == f.objPointerHolder.object);	}
		bool operator!=(const FunctionBase &f) const { return !operator==(f); }

		bool operator==(int ptr) const { return (0 == ptr && objPointerHolder.object == NULL && fnPointerHolder.IsNull()); }
		bool operator!=(int ptr) const { return !operator==(ptr); }

        friend bool operator==(int ptr, const FunctionBase& fn) { return (0 == ptr && fn.objPointerHolder.object == NULL && fn.fnPointerHolder.IsNull()); }
        friend bool operator!=(int ptr, const FunctionBase& fn) { return (0 != ptr || (fn.objPointerHolder.object != NULL && !fn.fnPointerHolder.IsNull())); }

		template<typename CR, typename CP1, typename CP2, typename CP3, typename CP4, typename CP5, typename CP6, typename CP7, typename CP8>
		bool operator==(const FunctionBase<CR, CP1, CP2, CP3, CP4, CP5, CP6, CP7, CP8> &f) const
		{
			return false;
		}

		template<typename CR, typename CP1, typename CP2, typename CP3, typename CP4, typename CP5, typename CP6, typename CP7, typename CP8>
		bool operator!=(const FunctionBase<CR, CP1, CP2, CP3, CP4, CP5, CP6, CP7, CP8> &f) const
		{
			return true;
		}

	protected:
		FunctionPointerHolder fnPointerHolder;
		ObjectPointerHolder objPointerHolder;

		typedef R(*InvokerType0)(void *obj, const FunctionPointerHolder&);
		typedef R(*InvokerType1)(void *obj, const FunctionPointerHolder&, ParamRefType1);
		typedef R(*InvokerType2)(void *obj, const FunctionPointerHolder&, ParamRefType1, ParamRefType2);
		typedef R(*InvokerType3)(void *obj, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3);
		typedef R(*InvokerType4)(void *obj, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3, ParamRefType4);
		typedef R(*InvokerType5)(void *obj, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3, ParamRefType4, ParamRefType5);
		typedef R(*InvokerType6)(void *obj, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3, ParamRefType4, ParamRefType5, ParamRefType6);
		typedef R(*InvokerType7)(void *obj, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3, ParamRefType4, ParamRefType5, ParamRefType6, ParamRefType7);
		typedef R(*InvokerType8)(void *obj, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3, ParamRefType4, ParamRefType5, ParamRefType6, ParamRefType7, ParamRefType8);

		union
		{
			InvokerType0 invoker0;
			InvokerType1 invoker1;
			InvokerType2 invoker2;
			InvokerType3 invoker3;
			InvokerType4 invoker4;
			InvokerType5 invoker5;
			InvokerType6 invoker6;
			InvokerType7 invoker7;
			InvokerType8 invoker8;
		};

		inline static R StaticInvokerImpl0(void *obj, const FunctionPointerHolder &holder)
		{
			StaticFunctionType0 fn = holder.GetPointer<StaticFunctionType0>();
			return (*fn)();
		}

		template<typename O>
		inline static R ObjectInvokerImpl0(void *obj, const FunctionPointerHolder &holder)
		{
			typedef typename ObjectType<O>::ObjectFunctionType0 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *) obj)->*fn)();
		}

		inline static R ClassInvokerImpl1(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1)
		{
			ClassFunctionType1 fn = holder.GetPointer<ClassFunctionType1>();
			return (((C*)p1)->*fn)();
		}

		inline static R StaticInvokerImpl1(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1)
		{
			StaticFunctionType1 fn = holder.GetPointer<StaticFunctionType1>();
			return (*fn)(p1);
		}

		template<typename O>
		inline static R ObjectInvokerImpl1(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1)
		{
			typedef typename ObjectType<O>::ObjectFunctionType1 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *)obj)->*fn)(p1);
		}

		inline static R ClassInvokerImpl2(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2)
		{
			ClassFunctionType2 fn = holder.GetPointer<ClassFunctionType2>();
			return (((C*)p1)->*fn)(p2);
		}

		inline static R StaticInvokerImpl2(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2)
		{
			StaticFunctionType2 fn = holder.GetPointer<StaticFunctionType2>();
			return (*fn)(p1, p2);
		}

		template<typename O>
		inline static R ObjectInvokerImpl2(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2)
		{
			typedef typename ObjectType<O>::ObjectFunctionType2 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *)obj)->*fn)(p1, p2);
		}

		inline static R ClassInvokerImpl3(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3)
		{
			ClassFunctionType3 fn = holder.GetPointer<ClassFunctionType3>();
			return (((C*)p1)->*fn)(p2, p3);
		}

		inline static R StaticInvokerImpl3(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3)
		{
			StaticFunctionType3 fn = holder.GetPointer<StaticFunctionType3>();
			return (*fn)(p1, p2, p3);
		}

		template<typename O>
		inline static R ObjectInvokerImpl3(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3)
		{
			typedef typename ObjectType<O>::ObjectFunctionType3 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *)obj)->*fn)(p1, p2, p3);
		}

		inline static R ClassInvokerImpl4(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4)
		{
			ClassFunctionType4 fn = holder.GetPointer<ClassFunctionType4>();
			return (((C*)p1)->*fn)(p2, p3, p4);
		}

		inline static R StaticInvokerImpl4(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4)
		{
			StaticFunctionType4 fn = holder.GetPointer<StaticFunctionType4>();
			return (*fn)(p1, p2, p3, p4);
		}

		template<typename O>
		inline static R ObjectInvokerImpl4(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4)
		{
			typedef typename ObjectType<O>::ObjectFunctionType4 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *)obj)->*fn)(p1, p2, p3, p4);
		}

		inline static R ClassInvokerImpl5(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5)
		{
			ClassFunctionType5 fn = holder.GetPointer<ClassFunctionType5>();
			return (((C*)p1)->*fn)(p2, p3, p4, p5);
		}

		inline static R StaticInvokerImpl5(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5)
		{
			StaticFunctionType5 fn = holder.GetPointer<StaticFunctionType5>();
			return (*fn)(p1, p2, p3, p4, p5);
		}

		template<typename O>
		inline static R ObjectInvokerImpl5(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5)
		{
			typedef typename ObjectType<O>::ObjectFunctionType5 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *)obj)->*fn)(p1, p2, p3, p4, p5);
		}

		inline static R ClassInvokerImpl6(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6)
		{
			ClassFunctionType6 fn = holder.GetPointer<ClassFunctionType6>();
			return (((C*)p1)->*fn)(p2, p3, p4, p5, p6);
		}

		inline static R StaticInvokerImpl6(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6)
		{
			StaticFunctionType6 fn = holder.GetPointer<StaticFunctionType6>();
			return (*fn)(p1, p2, p3, p4, p5, p6);
		}

		template<typename O>
		inline static R ObjectInvokerImpl6(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6)
		{
			typedef typename ObjectType<O>::ObjectFunctionType6 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *)obj)->*fn)(p1, p2, p3, p4, p5, p6);
		}

		inline static R ClassInvokerImpl7(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6, ParamRefType7 p7)
		{
			ClassFunctionType7 fn = holder.GetPointer<ClassFunctionType7>();
			return (((C*)p1)->*fn)(p2, p3, p4, p5, p6, p7);
		}

		inline static R StaticInvokerImpl7(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6, ParamRefType7 p7)
		{
			StaticFunctionType7 fn = holder.GetPointer<StaticFunctionType7>();
			return (*fn)(p1, p2, p3, p4, p5, p6, p7);
		}

		template<typename O>
		inline static R ObjectInvokerImpl7(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6, ParamRefType7 p7)
		{
			typedef typename ObjectType<O>::ObjectFunctionType7 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *)obj)->*fn)(p1, p2, p3, p4, p5, p6, p7);
		}

		inline static R ClassInvokerImpl8(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6, ParamRefType7 p7, ParamRefType8 p8)
		{
			ClassFunctionType8 fn = holder.GetPointer<ClassFunctionType8>();
			return (((C*)p1)->*fn)(p2, p3, p4, p5, p6, p7, p8);
		}

		inline static R StaticInvokerImpl8(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6, ParamRefType7 p7, ParamRefType8 p8)
		{
			StaticFunctionType8 fn = holder.GetPointer<StaticFunctionType8>();
			return (*fn)(p1, p2, p3, p4, p5, p6, p7, p8);
		}

		template<typename O>
		inline static R ObjectInvokerImpl8(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5, ParamRefType6 p6, ParamRefType7 p7, ParamRefType8 p8)
		{
			typedef typename ObjectType<O>::ObjectFunctionType8 ObjectFunctionType;
			ObjectFunctionType fn = holder.GetPointer<ObjectFunctionType>();
			return (((O *)obj)->*fn)(p1, p2, p3, p4, p5, p6, p7, p8);
		}
	};

	// ===================================================================================================================
	// Function class
	// TODO: description
	// ===================================================================================================================
	template<typename T>
	struct Function
	{ };

	// 0 arguments specialization
	template<typename R>
	struct Function<R()> : public FunctionBase<R>
	{
		typedef FunctionBase<R> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()() const { return Base::invoker0(Base::objPointerHolder.object, Base::fnPointerHolder); }
	};

	// 1 argument specialization
	template<typename R, typename P1>
	struct Function<R(P1)> : public FunctionBase<R, P1>
	{
		typedef FunctionBase<R, P1> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()(P1 p1) const { return Base::invoker1(Base::objPointerHolder.object, Base::fnPointerHolder, p1); }
	};

	// 2 argument specialization
	template<typename R, typename P1, typename P2>
	struct Function<R(P1, P2)> : public FunctionBase<R, P1, P2>
	{
		typedef FunctionBase<R, P1, P2> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()(P1 p1, P2 p2) const { return Base::invoker2(Base::objPointerHolder.object, Base::fnPointerHolder, p1, p2); }
	};

	// 3 argument specialization
	template<typename R, typename P1, typename P2, typename P3>
	struct Function<R(P1, P2, P3)> : public FunctionBase<R, P1, P2, P3>
	{
		typedef FunctionBase<R, P1, P2, P3> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()(P1 p1, P2 p2, P3 p3) const { return Base::invoker3(Base::objPointerHolder.object, Base::fnPointerHolder, p1, p2, p3); }
	};

	// 4 argument specialization
	template<typename R, typename P1, typename P2, typename P3, typename P4>
	struct Function<R(P1, P2, P3, P4)> : public FunctionBase<R, P1, P2, P3, P4>
	{
		typedef FunctionBase<R, P1, P2, P3, P4> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()(P1 p1, P2 p2, P3 p3, P4 p4) const { return Base::invoker4(Base::objPointerHolder.object, Base::fnPointerHolder, p1, p2, p3, p4); }
	};

	// 5 argument specialization
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	struct Function<R(P1, P2, P3, P4, P5)> : public FunctionBase<R, P1, P2, P3, P4, P5>
	{
		typedef FunctionBase<R, P1, P2, P3, P4, P5> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const { return Base::invoker5(Base::objPointerHolder.object, Base::fnPointerHolder, p1, p2, p3, p4, p5); }
	};

	// 6 argument specialization
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	struct Function<R(P1, P2, P3, P4, P5, P6)> : public FunctionBase<R, P1, P2, P3, P4, P5, P6>
	{
		typedef FunctionBase<R, P1, P2, P3, P4, P5, P6> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const { return Base::invoker6(Base::objPointerHolder.object, Base::fnPointerHolder, p1, p2, p3, p4, p5, p6); }
	};

	// 7 argument specialization
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	struct Function<R(P1, P2, P3, P4, P5, P6, P7)> : public FunctionBase<R, P1, P2, P3, P4, P5, P6, P7>
	{
		typedef FunctionBase<R, P1, P2, P3, P4, P5, P6, P7> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) const { return Base::invoker7(Base::objPointerHolder.object, Base::fnPointerHolder, p1, p2, p3, p4, p5, p6, p7); }
	};

	// 8 argument specialization
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	struct Function<R(P1, P2, P3, P4, P5, P6, P7, P8)> : public FunctionBase<R, P1, P2, P3, P4, P5, P6, P7, P8>
	{
		typedef FunctionBase<R, P1, P2, P3, P4, P5, P6, P7, P8> Base;
        Function() {}
		Function(int ptr) {}
		Function(long int ptr) {}
		Function(typename Base::IncomingStaticFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionType fn) : Base(fn) {}
		Function(typename Base::IncomingClassFunctionConstType fn) : Base(fn) {}
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
		template<typename O> Function(O* obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionType fn) : Base(obj, fn) { }
        template<typename O> Function(PointerWrapper<O> obj, typename Base::template ObjectType<O>::IncomingObjectFunctionConstType fn) : Base(obj, fn) { }

		inline R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) const { return Base::invoker8(Base::objPointerHolder.object, Base::fnPointerHolder, p1, p2, p3, p4, p5, p6, p7, p8); }
	};
}

#endif // __DAVA_FUNCTION_H__
