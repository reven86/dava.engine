#ifndef __DAVA_FUNCTION_H__
#define __DAVA_FUNCTION_H__

#include <new>
#include "TemplateHelpers.h"

namespace DAVA
{
	class BoundBase
	{
	public:
		BoundBase() : refCount(1)
		{}

		void AddRef()
		{
			refCount++;
		}

		void Release()
		{
			refCount--;
			if (0 == refCount)
			{
				delete this;
			}
		}

	private:
		unsigned int refCount;
	};

	// ====================================================================================================================================================
	// FunctionPointerHolder struct 
	// TODO: description
	// ====================================================================================================================================================
	struct FunctionPointerHolder
	{
		enum
		{
			FN_PTR_MAX_SIZE = (sizeof(void *)* 2)
		};

		unsigned char buffer[FN_PTR_MAX_SIZE];

		template<typename FunctionType>
		FunctionPointerHolder(FunctionType function)
		{
			// Function pointer with size greater than FN_PTR_MAX_SIZE
			// can't be added to this holder. This will be checked on compile time.
			COMPILER_ASSERT(sizeof(FunctionType) <= FN_PTR_MAX_SIZE);

			memset(buffer, 0, FN_PTR_MAX_SIZE);
			new(buffer) FunctionType(function);
		}

		template<typename FunctionType>
		inline FunctionType GetPointer() const
		{
			return *reinterpret_cast<FunctionType const *>(buffer);
		}

		bool operator==(const FunctionPointerHolder &f)
		{
			return (0 == memcmp(buffer, f.buffer, FN_PTR_MAX_SIZE));
		}
	};

    struct ObjectPointerHolder
    {
    public:
        ObjectPointerHolder(BoundBase *obj) : object(obj), isBoundHolder(true) 
        {}

        ObjectPointerHolder(void *obj) : object(obj), isBoundHolder(false) 
        {}
        
        ObjectPointerHolder(const ObjectPointerHolder& holder)
        {
            isBoundHolder = holder.isBoundHolder;
            object = holder.object;

            AddRef();
        }

        ObjectPointerHolder& operator=(const ObjectPointerHolder& holder)
        {
            Release();

            object = holder.object;
            isBoundHolder = holder.isBoundHolder;

            AddRef();

            return *this;
        }

        ~ObjectPointerHolder()
        {
            Release();
        }

    protected:
		bool isBoundHolder;
		void *object;

        inline void Release()
        {
            if(isBoundHolder && NULL != object)
            {
                ((BoundBase *) object)->Release();
                object = NULL;
            }
        }

        inline void AddRef()
        {
            if(isBoundHolder)
            {
                ((BoundBase *) object)->AddRef();
            }
        }
    };

	// ====================================================================================================================================================
	// FunctionBase class 
	// TODO: description
	// ====================================================================================================================================================
	template<typename R = NullType, typename P1 = NullType, typename P2 = NullType, typename P3 = NullType, typename P4 = NullType, typename P5 = NullType>
	class FunctionBase : public ObjectPointerHolder
	{
	public:
		typedef R ReturnType;
		typedef P1 ParamType1;
		typedef P2 ParamType2;
		typedef P3 ParamType3;
		typedef P4 ParamType4;
		typedef P5 ParamType5;

		// evaluating best functor argument types
		typedef typename TypeTraits<P1>::ParamType ParamRefType1;
		typedef typename TypeTraits<P2>::ParamType ParamRefType2;
		typedef typename TypeTraits<P3>::ParamType ParamRefType3;
		typedef typename TypeTraits<P4>::ParamType ParamRefType4;
		typedef typename TypeTraits<P5>::ParamType ParamRefType5;

		// available static function types
		typedef ReturnType(*StaticFunctionType0)();
		typedef ReturnType(*StaticFunctionType1)(P1);
		typedef ReturnType(*StaticFunctionType2)(P1, P2);
		typedef ReturnType(*StaticFunctionType3)(P1, P2, P3);
		typedef ReturnType(*StaticFunctionType4)(P1, P2, P3, P4);
		typedef ReturnType(*StaticFunctionType5)(P1, P2, P3, P4, P5);

		// evaluating incoming static function type
		typedef StaticFunctionType0 IncomingStaticFunctionType0;
		typedef typename Select<IsNullType<P1>::result, IncomingStaticFunctionType0, StaticFunctionType1>::Result IncomingStaticFunctionType1;
		typedef typename Select<IsNullType<P2>::result, IncomingStaticFunctionType1, StaticFunctionType2>::Result IncomingStaticFunctionType2;
		typedef typename Select<IsNullType<P3>::result, IncomingStaticFunctionType2, StaticFunctionType3>::Result IncomingStaticFunctionType3;
		typedef typename Select<IsNullType<P2>::result, IncomingStaticFunctionType3, StaticFunctionType4>::Result IncomingStaticFunctionType4;
		typedef typename Select<IsNullType<P2>::result, IncomingStaticFunctionType4, StaticFunctionType5>::Result IncomingStaticFunctionType5;
		typedef IncomingStaticFunctionType5 IncomingStaticFunctionType; // evaluated result

		template<typename C>
		struct ClassFunction
		{
			// available class member function types
			typedef R(C::*ClassFunctionType0)();
			typedef R(C::*ClassFunctionType1)(P1);
			typedef R(C::*ClassFunctionType2)(P1, P2);
			typedef R(C::*ClassFunctionType3)(P1, P2, P3);
			typedef R(C::*ClassFunctionType4)(P1, P2, P3, P4);
			typedef R(C::*ClassFunctionType5)(P1, P2, P3, P4, P5);

			// available class member function const types
			typedef R(C::*ClassFunctionConstType0)() const;
			typedef R(C::*ClassFunctionConstType1)(P1) const;
			typedef R(C::*ClassFunctionConstType2)(P1, P2) const;
			typedef R(C::*ClassFunctionConstType3)(P1, P2, P3) const;
			typedef R(C::*ClassFunctionConstType4)(P1, P2, P3, P4) const;
			typedef R(C::*ClassFunctionConstType5)(P1, P2, P3, P4, P5) const;

			// evaluating incoming class member function type
			typedef ClassFunctionType0 IncomingClassFunctionType0;
            typedef typename Select<IsNullType<P1>::result, IncomingClassFunctionType0, ClassFunctionType1>::Result IncomingClassFunctionType1;
			typedef typename Select<IsNullType<P2>::result, IncomingClassFunctionType1, ClassFunctionType2>::Result IncomingClassFunctionType2;
			typedef typename Select<IsNullType<P3>::result, IncomingClassFunctionType2, ClassFunctionType3>::Result IncomingClassFunctionType3;
			typedef typename Select<IsNullType<P4>::result, IncomingClassFunctionType3, ClassFunctionType4>::Result IncomingClassFunctionType4;
			typedef typename Select<IsNullType<P5>::result, IncomingClassFunctionType4, ClassFunctionType5>::Result IncomingClassFunctionType5;
			typedef IncomingClassFunctionType5 IncomingClassFunctionType; // evaluated result

			// evaluating incoming class member function const type
			typedef ClassFunctionConstType0 IncomingClassFunctionConstType0;
			typedef typename Select<IsNullType<P1>::result, IncomingClassFunctionConstType0, ClassFunctionConstType1>::Result IncomingClassFunctionConstType1;
			typedef typename Select<IsNullType<P2>::result, IncomingClassFunctionConstType1, ClassFunctionConstType2>::Result IncomingClassFunctionConstType2;
			typedef typename Select<IsNullType<P3>::result, IncomingClassFunctionConstType2, ClassFunctionConstType3>::Result IncomingClassFunctionConstType3;
			typedef typename Select<IsNullType<P4>::result, IncomingClassFunctionConstType3, ClassFunctionConstType4>::Result IncomingClassFunctionConstType4;
			typedef typename Select<IsNullType<P5>::result, IncomingClassFunctionConstType4, ClassFunctionConstType5>::Result IncomingClassFunctionConstType5;
			typedef IncomingClassFunctionConstType5 IncomingClassFunctionConstType; // evaluated result
		};

        // empty constructor
        //FunctionBase() : ObjectPointerHolder((void *) NULL) {}

		// constructors for static functions
		FunctionBase(StaticFunctionType0 fn) : ObjectPointerHolder((void *) NULL), fnPointerHolder(fn) { invoker0 = &StaticInvokerImpl0; }
		FunctionBase(StaticFunctionType1 fn) : ObjectPointerHolder((void *) NULL), fnPointerHolder(fn) { invoker1 = &StaticInvokerImpl1; }
		FunctionBase(StaticFunctionType2 fn) : ObjectPointerHolder((void *) NULL), fnPointerHolder(fn) { invoker2 = &StaticInvokerImpl2; }
		FunctionBase(StaticFunctionType3 fn) : ObjectPointerHolder((void *) NULL), fnPointerHolder(fn) { invoker3 = &StaticInvokerImpl3; }
		FunctionBase(StaticFunctionType4 fn) : ObjectPointerHolder((void *) NULL), fnPointerHolder(fn) { invoker4 = &StaticInvokerImpl4; }
		FunctionBase(StaticFunctionType5 fn) : ObjectPointerHolder((void *) NULL), fnPointerHolder(fn) { invoker5 = &StaticInvokerImpl5; }

        // constructors for class member functions
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionType0 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker0 = &ClassInvokerImpl0<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionType1 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker1 = &ClassInvokerImpl1<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionType2 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker2 = &ClassInvokerImpl2<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionType3 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker3 = &ClassInvokerImpl3<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionType4 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker4 = &ClassInvokerImpl4<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionType5 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker5 = &ClassInvokerImpl5<C>; }
        
        // constructors for class member const functions
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionConstType0 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker0 = &ClassInvokerImpl0<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionConstType1 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker1 = &ClassInvokerImpl1<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionConstType2 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker2 = &ClassInvokerImpl2<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionConstType3 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker3 = &ClassInvokerImpl3<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionConstType4 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker4 = &ClassInvokerImpl4<C>; }
		template<typename C> FunctionBase(C *obj, typename ClassFunction<C>::ClassFunctionConstType5 fn) : ObjectPointerHolder(obj), fnPointerHolder(fn) { invoker5 = &ClassInvokerImpl5<C>; }

		// operators
		bool operator==(const FunctionBase &f)
		{
			return (object == f.object && fnPointerHolder == f.fnPointerHolder);
		}

		bool operator!=(const FunctionBase &f)
		{
			return !operator==(f);
		}

		template<typename CR, typename CP1, typename CP2, typename CP3, typename CP4, typename CP5>
		bool operator==(const FunctionBase<CR, CP1, CP2, CP3, CP4, CP5> &f)
		{
			return false;
		}

		template<typename CR, typename CP1, typename CP2, typename CP3, typename CP4, typename CP5>
		bool operator!=(const FunctionBase<CR, CP1, CP2, CP3, CP4, CP5> &f)
		{
			return true;
		}

	protected:
		FunctionPointerHolder fnPointerHolder;

		typedef R(*InvokerType0)(void *, const FunctionPointerHolder&);
		typedef R(*InvokerType1)(void *, const FunctionPointerHolder&, ParamRefType1);
		typedef R(*InvokerType2)(void *, const FunctionPointerHolder&, ParamRefType1, ParamRefType2);
		typedef R(*InvokerType3)(void *, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3);
		typedef R(*InvokerType4)(void *, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3, ParamRefType4);
		typedef R(*InvokerType5)(void *, const FunctionPointerHolder&, ParamRefType1, ParamRefType2, ParamRefType3, ParamRefType4, ParamRefType5);

		union
		{
			InvokerType0 invoker0;
			InvokerType1 invoker1;
			InvokerType2 invoker2;
			InvokerType3 invoker3;
			InvokerType4 invoker4;
			InvokerType5 invoker5;
		};

		template <typename C>
		inline static R ClassInvokerImpl0(void* obj, const FunctionPointerHolder &holder)
		{
			typedef typename ClassFunction<C>::ClassFunctionType0 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(((C*)obj)->*fn)();
		}

		inline static R StaticInvokerImpl0(void *obj, const FunctionPointerHolder &holder)
		{
			typedef StaticFunctionType0 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(*fn)();
		}

		template <typename C>
		inline static R ClassInvokerImpl1(void* obj, const FunctionPointerHolder &holder, ParamRefType1 p1)
		{
			typedef typename ClassFunction<C>::ClassFunctionType1 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(((C*)obj)->*fn)(p1);
		}

		inline static R StaticInvokerImpl1(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1)
		{
			typedef StaticFunctionType1 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(*fn)(p1);
		}

		template <typename C>
		inline static R ClassInvokerImpl2(void* obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2)
		{
			typedef typename ClassFunction<C>::ClassFunctionType2 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(((C*)obj)->*fn)(p1, p2);
		}

		inline static R StaticInvokerImpl2(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2)
		{
			typedef StaticFunctionType2 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(*fn)(p1, p2);
		}

		template <typename C>
		inline static R ClassInvokerImpl3(void* obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3)
		{
			typedef typename ClassFunction<C>::ClassFunctionType3 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(((C*)obj)->*fn)(p1, p2, p3);
		}

		inline static R StaticInvokerImpl3(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3)
		{
			typedef StaticFunctionType3 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(*fn)(p1, p2, p3);
		}

		template <typename C>
		inline static R ClassInvokerImpl4(void* obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4)
		{
			typedef typename ClassFunction<C>::ClassFunctionType4 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(((C*)obj)->*fn)(p1, p2, p3, p4);
		}

		inline static R StaticInvokerImpl4(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4)
		{
			typedef StaticFunctionType4 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(*fn)(p1, p2, p3, p4);
		}

		template <typename C>
		inline static R ClassInvokerImpl5(void* obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5)
		{
			typedef typename ClassFunction<C>::ClassFunctionType5 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(((C*)obj)->*fn)(p1, p2, p3, p4, p5);
		}

		inline static R StaticInvokerImpl5(void *obj, const FunctionPointerHolder &holder, ParamRefType1 p1, ParamRefType2 p2, ParamRefType3 p3, ParamRefType4 p4, ParamRefType5 p5)
		{
			typedef StaticFunctionType5 HolderFunctionType;
			HolderFunctionType fn = holder.GetPointer<HolderFunctionType>();
			(*fn)(p1, p2, p3, p4, p5);
		}
	};

	// ====================================================================================================================================================
	// Function class 
	// TODO: description
	// ====================================================================================================================================================
	template<typename R = NullType, typename P1 = NullType, typename P2 = NullType, typename P3 = NullType, typename P4 = NullType, typename P5 = NullType, typename P6 = NullType>
 	struct Function : public FunctionBase<R, P1, P2, P3, P4, P5>
 	{ };

    #define FUNCTION_CONSTRUCTORS \
        typedef typename MyBase::ParamRefType1 MyParamRefType1; \
        typedef typename MyBase::ParamRefType2 MyParamRefType2; \
        typedef typename MyBase::ParamRefType3 MyParamRefType3; \
        typedef typename MyBase::ParamRefType4 MyParamRefType4; \
        typedef typename MyBase::ParamRefType5 MyParamRefType5; \
        Function(typename MyBase::IncomingStaticFunctionType fn) : MyBase(fn) { } \
		template<typename C> Function(C *obj, typename MyBase::template ClassFunction<C>::IncomingClassFunctionType fn) : MyBase(obj, fn) {} \
		template<typename C> Function(C *obj, typename MyBase::template ClassFunction<C>::IncomingClassFunctionConstType fn) : MyBase(obj, fn) {} \

	// 0 arguments specialization
	template<typename R>
	struct Function<R> : public FunctionBase<R>
	{
        typedef FunctionBase<R> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()() const { MyBase::invoker0(MyBase::object, MyBase::fnPointerHolder); }
	};

	template<typename R>
	struct Function<R()> : public FunctionBase<R>
	{
        typedef FunctionBase<R> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()() const { MyBase::invoker0(MyBase::object, MyBase::fnPointerHolder); }
	};

	// 1 argument specialization
	template<typename R, typename P1>
	struct Function<R, P1> : public FunctionBase<R, P1>
	{
        typedef FunctionBase<R, P1> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1) const { MyBase::invoker1(MyBase::object, MyBase::fnPointerHolder, p1); }
	};

	template<typename R, typename P1>
	struct Function<R(P1)> : public FunctionBase<R, P1>
	{
        typedef FunctionBase<R, P1> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1) const { MyBase::invoker1(MyBase::object, MyBase::fnPointerHolder, p1); }
	};

	// 2 argument specialization
	template<typename R, typename P1, typename P2>
	struct Function<R, P1, P2> : public FunctionBase<R, P1, P2>
	{
        typedef FunctionBase<R, P1, P2> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1, MyParamRefType2 p2) const { MyBase::invoker2(MyBase::object, MyBase::fnPointerHolder, p1, p2); }
	};

	template<typename R, typename P1, typename P2>
	struct Function<R (P1, P2)> : public FunctionBase<R, P1, P2>
	{
        typedef FunctionBase<R, P1, P2> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1, MyParamRefType2 p2) const { MyBase::invoker2(MyBase::object, MyBase::fnPointerHolder, p1, p2); }
	};

	// 3 argument specialization
	template<typename R, typename P1, typename P2, typename P3>
	struct Function<R, P1, P2, P3> : public FunctionBase<R, P1, P2, P3>
	{
        typedef FunctionBase<R, P1, P2, P3> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1, MyParamRefType2 p2, MyParamRefType3 p3) const { MyBase::invoker3(MyBase::object, MyBase::fnPointerHolder, p1, p2, p3); }
	};

	template<typename R, typename P1, typename P2, typename P3>
	struct Function<R (P1, P2, P3)> : public FunctionBase<R, P1, P2, P3>
	{
        typedef FunctionBase<R, P1, P2, P3> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1, MyParamRefType2 p2, MyParamRefType3 p3) const { MyBase::invoker3(MyBase::object, MyBase::fnPointerHolder, p1, p2, p3); }
	};

	// 4 argument specialization
	template<typename R, typename P1, typename P2, typename P3, typename P4>
	struct Function<R, P1, P2, P3, P4> : public FunctionBase<R, P1, P2, P3, P4>
	{
        typedef FunctionBase<R, P1, P2, P3, P4> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1, MyParamRefType2 p2, MyParamRefType3 p3, MyParamRefType4 p4) const { MyBase::invoker4(MyBase::object, MyBase::fnPointerHolder, p1, p2, p3, p4); }
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4>
	struct Function<R (P1, P2, P3, P4)> : public FunctionBase<R, P1, P2, P3, P4>
	{
        typedef FunctionBase<R, P1, P2, P3, P4> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1, MyParamRefType2 p2, MyParamRefType3 p3, MyParamRefType4 p4) const { MyBase::invoker4(MyBase::object, MyBase::fnPointerHolder, p1, p2, p3, p4); }
	};

	// 5 argument specialization
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	struct Function<R, P1, P2, P3, P4, P5> : public FunctionBase<R, P1, P2, P3, P4, P5>
	{
        typedef FunctionBase<R, P1, P2, P3, P4, P5> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1, MyParamRefType2 p2, MyParamRefType3 p3, MyParamRefType4 p4, MyParamRefType5 p5) const { MyBase::invoker5(MyBase::object, MyBase::fnPointerHolder, p1, p2, p3, p4, p5); }
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	struct Function<R (P1, P2, P3, P4, P5)> : public FunctionBase<R, P1, P2, P3, P4, P5>
	{
        typedef FunctionBase<R, P1, P2, P3, P4, P5> MyBase;
        FUNCTION_CONSTRUCTORS

		inline R operator()(MyParamRefType1 p1, MyParamRefType2 p2, MyParamRefType3 p3, MyParamRefType4 p4, MyParamRefType5 p5) const { MyBase::invoker5(MyBase::object, MyBase::fnPointerHolder, p1, p2, p3, p4, p5); }
	};
};

#endif // __DAVA_FUNCTION_H__
