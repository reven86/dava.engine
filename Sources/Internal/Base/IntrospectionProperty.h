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



#ifndef __DAVAENGINE_INTROSPECTION_PROPERTY_H__
#define __DAVAENGINE_INTROSPECTION_PROPERTY_H__

#include "Base/IntrospectionBase.h"

namespace DAVA
{
	// Класс представляет расширение базового класса IntrospectionMember и описывает члена интроспекции, как свойство
	// Свойство в отличии от базового члена интроспекции позволяет указать методы класса, 
	// которые будут использоваться при получении/установки значений в данный член интроспекции.
	// Свойства требуются в том случае, если логика работы класса такова, что при изменение одной переменной
	// может поменять другая.
	// 
	// В классе переопределены виртуальные функции базового класса - IntrospectionMember: Value(), SetValue()
	// 
	// Пример:
	// class A
	// {
	// public:
	//		int sum;
	//		
	//		int GetA() {return a;};
	//		int GetB() {return b;};
	//		
	//		void SetA(int _a) {a = _a; sum = a + b;};
	//		void SetB(int _b) {b = _b; sum = a + b;};
	//		
	// protected:
	//		int a;
	//		int b;
	//		
	// public:
	//		INTROSPECTION(A, 
	//			MEMBER(sum, "Sum a+b", 0)
	//			PROPERTY(a, "a value", GetA, SetA, 0)
	//			PROPERTY(b, "b value", GetB, SetB, 0)
	//		);
	// };

	template<typename T, typename V>
	class InspProp : public InspMember
	{
	public:
		typedef V	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(const V &);

		InspProp(const char *_name, const InspDesc &_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: InspMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

		virtual void* Pointer(void *object) const { return NULL; };
		virtual void* Data(void *object) const { return NULL; };

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	// Этот класс по сути специализация IntrospectionProperty, с той лишь разницой, что
	// возвращаемое Get функцией значение является ссылкой.
	template<typename T, typename V>
	class InspPropReturnRef : public InspMember
	{
	public:
		typedef V&	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(const V &);

		InspPropReturnRef(const char *_name, const InspDesc &_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: InspMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V& realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

		virtual void* Pointer(void *object) const { return NULL; };
		virtual void* Data(void *object) const { return NULL; };

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	// Этот класс по сути специализация IntrospectionProperty, с той лишь разницой, что
	// аргумертом Set/Get функций является указатель.
	template<typename T, typename V>
	class InspPropParamRef : public InspMember
	{
	public:
		typedef V*	 (T::*GetterPtr)();
		typedef void (T::*SetterPtr)(V*);

		InspPropParamRef(const char *_name, const InspDesc &_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: InspMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V* realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V* realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

		virtual void* Pointer(void *object) const { return NULL; };
		virtual void* Data(void *object) const 
		{ 
			if(type->IsPointer())
			{
				T* realObj = (T *) object;
				return (realObj->*getter)();
			}

			return NULL;
		};

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	// Этот класс по сути специализация IntrospectionProperty, с той лишь разницой, что
	// аргумертом Set/Get функций является обычный тип.
	template<typename T, typename V>
	class InspPropParamSimple : public InspMember
	{
	public:
		typedef V	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(V);

		InspPropParamSimple(const char *_name, const InspDesc &_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: InspMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

		virtual void* Pointer(void *object) const { return NULL; };
		virtual void* Data(void *object) const { return NULL; };

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

    // Этот класс по сути специализация IntrospectionProperty, с той лишь разницой, что
    // аргумертом Set/Get функций является перечисление.
    template<typename T, typename V>
    class InspPropParamEnum : public InspMember
    {
    public:
        typedef V	 (T::*GetterPtr)() const;
        typedef void (T::*SetterPtr)(V);

        InspPropParamEnum(const char *_name, const InspDesc &_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
            : InspMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
        { }

        virtual VariantType Value(void *object) const
        {
            T* realObj = (T *) object;
            DAVA::int32 storeValue = static_cast<DAVA::int32>((realObj->*getter)());
            return VariantType::LoadData(&storeValue, DAVA::MetaInfo::Instance<DAVA::int32>());
        }

        virtual void SetValue(void *object, const VariantType &val) const
        {
            T* realObj = (T *) object;
            DAVA::int32 storeValue = 0;
            VariantType::SaveData(&storeValue, DAVA::MetaInfo::Instance<DAVA::int32>(), val);
            (realObj->*setter)(static_cast<V>(storeValue));
        }

        virtual void* Pointer(void *object) const { return NULL; };
        virtual void* Data(void *object) const { return NULL; };

    protected:
        const GetterPtr getter;
        const SetterPtr setter;
    };
    

	// Набор функций для автоматического вывода параметро и создания IntrospectionProperty или IntrospectionPropertyRef
	// в зависимости от входных типов
	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new InspProp<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new InspProp<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, const VV (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new InspProp<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, const VV (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new InspProp<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), (VV (TT::*)() const) _g, _s, _flags);
	}

	// ret ref
	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV& (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new InspPropReturnRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV& (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new InspPropReturnRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, const VV& (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new InspPropReturnRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, const VV& (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new InspPropReturnRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), (VV& (TT::*)() const) _g, _s, _flags);
	}


	// param pointer
	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV* (TT::*_g)(), void (TT::*_s)(const VV*), int _flags)
	{
		return new InspPropParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV* (TT::*_g)() const, void (TT::*_s)(const VV*), int _flags)
	{
		return new InspPropParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, const VV* (TT::*_g)(), void (TT::*_s)(const VV*), int _flags)
	{
		return new InspPropParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, const VV* (TT::*_g)() const, void (TT::*_s)(const VV*), int _flags)
	{
		return new InspPropParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV* (TT::*_g)(), void (TT::*_s)(VV*), int _flags)
	{
		return new InspPropParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV* (TT::*_g)() const, void (TT::*_s)(VV*), int _flags)
	{
		return new InspPropParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, const VV* (TT::*_g)(), void (TT::*_s)(VV*), int _flags)
	{
		return new InspPropParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, const VV* (TT::*_g)() const, void (TT::*_s)(VV*), int _flags)
	{
		return new InspPropParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	// param simple
	template<typename TT, typename VV>
	DAVA::InspMember* CreateIspProp(const char *_name, const InspDesc &_desc, VV (TT::*_g)() const, void (TT::*_s)(VV), int _flags)
	{
		return new InspPropParamSimple<TT, VV>(_name, _desc, DAVA::MetaInfo::Instance<VV>(), _g, _s, _flags);
	}

    // param enum
    template<typename TT, typename VV>
    DAVA::InspMember* CreateInspEnum(const char *_name, const char *_desc, VV (TT::*_g)() const, void (TT::*_s)(VV), int _flags)
    {
        return new InspPropParamEnum<TT, VV>(_name, InspDesc(_desc, GlobalEnumMap<VV>::Instance()), DAVA::MetaInfo::Instance<VV>(), _g, _s, _flags);
    }
};

#endif // __DAVAENGINE_INTROSPECTION_PROPERTY_H__
