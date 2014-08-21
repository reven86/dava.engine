#ifndef __DAVA_BIND_H__
#define __DAVA_BIND_H__

#include "Function.h"

namespace DAVA
{
	// Placeholder
	template<int>
	struct Placeholder
	{ };

	template<typename U> struct IsPlaceholder { enum { result = false }; };
	template<> struct IsPlaceholder< Placeholder<1> > { enum { result = true }; };
	template<> struct IsPlaceholder< Placeholder<2> > { enum { result = true }; };
	template<> struct IsPlaceholder< Placeholder<3> > { enum { result = true }; };
	template<> struct IsPlaceholder< Placeholder<4> > { enum { result = true }; };
	template<> struct IsPlaceholder< Placeholder<5> > { enum { result = true }; };

	// Argument holder class
	template<typename T>
	struct ParamHolder
	{
		typedef typename TypeTraits<T>::NonRefType ParamType;

		ParamHolder(const T& p) : param(p)
		{}

		const T& GetParameter() const
		{ 
			return param;
		}

		template <typename P1>
		const T& GetParameter(const P1&) const
		{
			return param;
		}

		template <typename P1, typename P2>
		const T& GetParameter(const P1&, const P2&) const
		{
			return param;
		}

		template <typename P1, typename P2, typename P3>
		const T& GetParameter(const P1&, const P2&, const P3&) const
		{
			return param;
		}

		template <typename P1, typename P2, typename P3, typename P4>
		const T& GetParameter(const P1&, const P2&, const P3&, const P4&) const
		{
			return param;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5>
		const T& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&) const
		{
			return param;
		}

	protected:
		ParamType param;
	};

	template<>
	struct ParamHolder< Placeholder<1> >
	{
		ParamHolder(const Placeholder<1> &) 
		{}

		template <typename P1>
		const P1& GetParameter(const P1 &p1) const 
		{ 
			return p1; 
		}

		template <typename P1, typename P2>
		const P1 & GetParameter(const P1& p1, const P2&) const 
		{
			return p1;
		}

		template <typename P1, typename P2, typename P3>
		const P1& GetParameter(const P1& p1, const P2&, const P3&) const 
		{
			return p1;
		}

		template <typename P1, typename P2, typename P3, typename P4>
		const P1& GetParameter(const P1& p1, const P2&, const P3&, const P4&) const 
		{
			return p1;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5>
		const P1& GetParameter(const P1& p1, const P2&, const P3&, const P4&, const P5&) const 
		{
			return p1;
		}
	};

	template<>
	struct ParamHolder< Placeholder<2> >
	{
		ParamHolder(const Placeholder<2> &) 
		{}

		template <typename P1, typename P2>
		const P2& GetParameter(const P1&, const P2& p2) const 
		{
			return p2;
		}

		template <typename P1, typename P2, typename P3>
		const P2& GetParameter(const P1&, const P2& p2, const P3&) const 
		{
			return p2;
		}

		template <typename P1, typename P2, typename P3, typename P4>
		const P2& GetParameter(const P1&, const P2& p2, const P3&, const P4&) const 
		{
			return p2;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5>
		const P2& GetParameter(const P1&, const P2& p2, const P3&, const P4&, const P5&) const 
		{
			return p2;
		}
	};


	template<>
	struct ParamHolder< Placeholder<3> >
	{
		ParamHolder(const Placeholder<3> &) 
		{}

		template <typename P1, typename P2, typename P3>
		const P3& GetParameter(const P1&, const P2&, const P3& p3) const 
		{
			return p3;
		}

		template <typename P1, typename P2, typename P3, typename P4>
		const P3& GetParameter(const P1&, const P2&, const P3& p3, const P4&) const 
		{
			return p3;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5>
		const P3& GetParameter(const P1&, const P2&, const P3& p3, const P4&, const P5&) const 
		{
			return p3;
		}
	};

	template<>
	struct ParamHolder< Placeholder<4> >
	{
		ParamHolder(const Placeholder<4> &) 
		{}

		template <typename P1, typename P2, typename P3, typename P4>
		const P4& GetParameter(const P1&, const P2&, const P3&,	const P4& p4) const 
		{
			return p4;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5>
		const P4& GetParameter(const P1&, const P2&, const P3&,	const P4& p4, const P5&) const 
		{
			return p4;
		}
	};

	template<>
	struct ParamHolder< Placeholder<5> >
	{
		ParamHolder(const Placeholder<5> &) 
		{}

		template <typename P1, typename P2, typename P3, typename P4, typename P5>
		const P5& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5& p5) const 
		{
			return p5;
		}
	};

	// Bound helper class
	template<typename IncomingFunctionType, typename P1 = NullType, typename P2 = NullType, typename P3 = NullType, typename P4 = NullType, typename P5 = NullType>
	struct BoundHelper
	{
		// It is allowed only DAVA::Function to be IncomingFunctionType
		typedef typename IncomingFunctionType::IsFunctionType OnlyDAVAFunctionIsAllowedToBeBinded;

		typedef typename IncomingFunctionType::ReturnType ReturnType;
		typedef typename IncomingFunctionType::ParamType1 ParamType1;
		typedef typename IncomingFunctionType::ParamType2 ParamType2;
		typedef typename IncomingFunctionType::ParamType3 ParamType3;
		typedef typename IncomingFunctionType::ParamType4 ParamType4;
		typedef typename IncomingFunctionType::ParamType5 ParamType5;

		typedef typename Select<IsPlaceholder<P1>::result, ParamType1, NullType>::Result RealParamType1;
		typedef typename Select<IsPlaceholder<P2>::result, ParamType2, NullType>::Result RealParamType2;
		typedef typename Select<IsPlaceholder<P3>::result, ParamType3, NullType>::Result RealParamType3;
		typedef typename Select<IsPlaceholder<P4>::result, ParamType4, NullType>::Result RealParamType4;
		typedef typename Select<IsPlaceholder<P5>::result, ParamType5, NullType>::Result RealParamType5;

		typedef Typelist<NullType, NullType> RealParamsList0;
		typedef typename TL::Append<RealParamsList0, RealParamType1>::Result RealParamsList1;
		typedef typename TL::Append<RealParamsList1, RealParamType2>::Result RealParamsList2;
		typedef typename TL::Append<RealParamsList2, RealParamType3>::Result RealParamsList3;
		typedef typename TL::Append<RealParamsList3, RealParamType4>::Result RealParamsList4;
		typedef typename TL::Append<RealParamsList4, RealParamType5>::Result RealParamsList5;
		typedef typename RealParamsList5 RealParamsFullList;

		typedef typename Function<ReturnType,
			typename TL::TypeAtNonStrict<RealParamsFullList, 1>::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, 2>::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, 3>::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, 4>::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, 5>::Result> OutgoingFunctionType;
	};

	// Bound class
	template<typename IncomingFunctionType, typename P1>
 	class Bound_1 : public BoundBase
 	{
 	public:
		typedef typename BoundHelper<IncomingFunctionType, P1> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_1(const IncomingFunctionType &_fn, const P1 &_p1) 
			: fn(_fn), p1(_p1)
 		{ }
 
		ReturnType invoke() { return fn(p1.GetParameter()); }

		template<typename U1>
		ReturnType invoke(U1 u1) { return fn(p1.GetParameter(u1)); }

 	protected:
		IncomingFunctionType fn;
 		ParamHolder<P1> p1;
 	};
 
	template<typename IncomingFunctionType, typename P1, typename P2>
	class Bound_2 : public BoundBase
	{
	public:
		typedef typename BoundHelper<IncomingFunctionType, P1, P2> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_2(const IncomingFunctionType &_fn, const P1 &_p1, const P2 &_p2)
			: fn(_fn), p1(_p1), p2(_p2)
		{ }

		ReturnType invoke() { return fn(p1.GetParameter(), p2.GetParameter()); }

		template<typename U1>
		ReturnType invoke(U1 u1) { return fn(p1.GetParameter(u1), p2.GetParameter(u1)); }

		template<typename U1, typename U2>
		ReturnType invoke(U1 u1, U2 u2) { return fn(p1.GetParameter(u1, u2), p2.GetParameter(u1, u2)); }

	protected:
		IncomingFunctionType fn;
		ParamHolder<P1> p1;
		ParamHolder<P2> p2;
	};
	

	// Bind functions
	template<typename F, typename P1>
	typename Bound_1<F, P1>::OutgoingFunctionType Bind(const F &fn, const P1 &p1)
 	{
		typedef Bound_1<F, P1> BoundType;

		BoundType *b = new BoundType(fn, p1);
		BoundType::OutgoingFunctionType ret(b, &BoundType::invoke);

		return ret;
 	}

	template<typename F, typename P1, typename P2>
	typename Bound_2<F, P1, P2>::OutgoingFunctionType Bind(const F &fn, const P1 &p1, const P2 &p2)
	{
		typedef Bound_2<F, P1, P2> BoundType;

		BoundType *b = new BoundType(fn, p1, p2);
		BoundType::OutgoingFunctionType ret(b, &BoundType::invoke);

        return ret;
	}
};

namespace
{
	static const DAVA::Placeholder<1> _1;
	static const DAVA::Placeholder<2> _2;
	static const DAVA::Placeholder<3> _3;
	static const DAVA::Placeholder<4> _4;
	static const DAVA::Placeholder<5> _5;
};

#endif // __DAVA_BIND_H__
