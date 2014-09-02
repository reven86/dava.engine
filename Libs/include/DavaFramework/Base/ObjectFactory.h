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


#ifndef __DAVAENGINE_OBJECT_FACTORY_H__
#define __DAVAENGINE_OBJECT_FACTORY_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/StaticSingleton.h"
#include "Base/ObjectCreator.h"
#include <typeinfo>


namespace DAVA 
{
	
/*#define REGISTER_CLASS_WITH_STRING_NAME(class_name, symbol_name) \
static BaseObject * Create##class_name()\
{\
return new class_name();\
};\
static ObjectRegistrator registrator##class_name(symbol_name, &Create##class_name);
*/	

/**
	\ingroup baseobjects
	\brief	This class allow to create classes by name
 
	For example you want to create bunch of classes using their names it can be done easily using framework
	In implementation of your class you should write REGISTER_CLASS(ClassName) and this class now can 
	be created by it's name using ObjectFactory singleton
	
	Class must be inherited from BaseObject and have default constructor.
 
	Example:
	
	Definition of class
	\code
	// UIButton.h
	class UIButton : public BaseObject
	{
		UIButton::UIButton()
 
	};
	\endcode
	
	Implementation of class
	\code
	// UIButton.cpp
	
	REGISTER_CLASS(UIButton);
	
	UIButton::UIButton()
	{
	}
	\endcode
		
	Usage of ObjectFactory:
	\code
		
	void MyFunction()
	{
		UIButton * button = dynamic_cast<UIButton*>(ObjectFactory::Instance()->New("UIButton"));
		
	}
 
	\endcode 
 */

class ObjectFactory : public StaticSingleton<ObjectFactory>
{
public:
    ObjectFactory();
    
	/**
		\brief creates a class with given name
		
		\param[in] name name of class you want to create
	 */
    template <class T>
    T* New(const String & name)
    {
        Map<String, CreateObjectFunc>::iterator it = creatorMap.find(name);
        if (it != creatorMap.end())
        {
            CreateObjectFunc newFunc = it->second;
            
            //VI: cannot use cast_if_equal since we need to cast to base types
            //VI: but cast_if_equal casts to the exact types only
            return static_cast<T*>((newFunc)());
        }
        return 0;
    }

	/**
        \brief creates a class with given name

        \param[in] name name of class you want to create
        \param[in] object you can pass as creation parameter
	 */
    template <class S, class T>
	T* New(const String & name, const S & object);
    
    //    /**
    //        \brief 
    //     */
    //    template <class T> 
    //    T * New(const String & name);

    template<class T>
    const String & GetName(T * t);
	
	/**
		\brief This function is supposed to RegisterObjectCreator
		
		It used internally by REGISTER_CLASS define and REGISTER_CLASS_WITH_ALIAS define.
        Both defines are in BaseObject.h
     
		\param[in] name this is name of class we want to register
		\param[in] func this is pointer to function that can create such class
        \param[in] alias this name can be used if you want to save object as his parent
	*/
	void RegisterObjectCreator(const String & name, CreateObjectFunc func, const std::type_info & typeinfo, uint32 size);
	void RegisterObjectCreator(const String & name, CreateObjectFunc func, const std::type_info & typeinfo, uint32 size, const String & alias);


    void Dump();

private:
	Map<String, CreateObjectFunc> creatorMap;
    Map<String, String> nameMap;
    Map<String, uint32> sizeMap;
    String unregisteredClassName;
};
    
    
template<class T>
const String & ObjectFactory::GetName(T * t)
{
    Map<String, String>::iterator it = nameMap.find(typeid(*t).name());
    if (it != nameMap.end())
    {
        return it->second;
    }
    return unregisteredClassName;
}

};

#endif // __DAVAENGINE_OBJECT_FACTORY_H__