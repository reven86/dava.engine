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


#ifndef __DAVAENGINE_BASEOBJECT_H__
#define __DAVAENGINE_BASEOBJECT_H__

#include "Concurrency/Atomic.h"
#include "Base/BaseTypes.h"
#include "Base/BaseObjectChecker.h"
#include "Base/Introspection.h"
#include "Debug/DVAssert.h"
#include "DAVAConfig.h"
#include "Base/RefPtr.h"
#include "Base/ScopedPtr.h"

#include "MemoryManager/MemoryProfiler.h"

#include <typeinfo>
#include <memory>

namespace DAVA
{

	
/**
	\defgroup baseobjects Framework Base Objects
	This group contain all framework classes which defines the basics of our system. 
	All architecture approaches & patterns we've selected in our framework is well-known and described. 
 */
/** 
	\ingroup baseobjects
	\brief class to implement object reference counting
 
	This class if base object for most of hierarchies in our SDK. It's main purpose to help you avoid issues with memory, and provide you 
    with many high-level mechanisms like serialization, messaging and so on. In most cases if you create own class it will be good idea 
    to derive it from BaseObject. 
  */

class InspInfo;
class KeyedArchive;
	
class BaseObject: public InspBase
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_BASEOBJECT);

protected:
	//! Destructor
	virtual ~BaseObject()
	{
		DVASSERT( referenceCount == 0 );
#ifdef ENABLE_BASE_OBJECT_CHECKS
		BaseObjectChecker::UnregisterBaseObject(this);
#endif 
	}

public:
	//! Constructor
	BaseObject()
		: referenceCount(1)
	{
#ifdef ENABLE_BASE_OBJECT_CHECKS
		BaseObjectChecker::RegisterBaseObject(this);
#endif 
	}

	/**
		\brief Increment reference counter in this object.
	 */
	virtual void Retain()
	{
        referenceCount++;
	}
	
	/** 
		\brief Decrement object reference counter and delete it if reference counter equal to 0.
		\returns referenceCounter value after decrement
	 */
	virtual int32 Release()
	{
#ifdef ENABLE_BASE_OBJECT_CHECKS
		if (!BaseObjectChecker::IsAvailable(this))
		{
			DVASSERT(0 && "Attempt to delete unavailable BaseObject");
		}	
#endif		

        int32 refCounter = --referenceCount;
		if (!refCounter)
		{
			delete this;
		}
		return refCounter;
	}

	/** 
		\brief return current number of references for this object
		\returns referenceCounter value 
	 */
	int32 GetRetainCount() const
	{
		return referenceCount.Get();
	}
    
    /**
        \brief return class name if it's registered with REGISTER_CLASS macro of our ObjectFactory class.
        This function is mostly intended for serialization, but can be used for other purposes as well.
        \returns name of the class you've passed to REGISTER_CLASS function. For example if you register class UIButton with the following line:
        REGISTER_CLASS(UIButton); you'll get "UIButton" as result.
     */
    const String & GetClassName() const;
    
    virtual void SaveObject(KeyedArchive * archive);
	virtual void LoadObject(KeyedArchive * archive);
    
    static BaseObject * LoadFromArchive(KeyedArchive * archive);
    
    static BaseObject * DummyGet() { return 0; };
protected:
    /*
    void SaveIntrospection(const String &key, KeyedArchive * archive, const IntrospectionInfo *info, void * object);
    void SaveCollection(const String &key, KeyedArchive * archive, const IntrospectionMember *member, void * object);
    
    void LoadIntrospection(const String &key, KeyedArchive * archive, const IntrospectionInfo *info, void * object);
    void LoadCollection(const String &key, KeyedArchive * archive, const IntrospectionMember *member, void * object);

    void * GetMemberObject(const IntrospectionMember *member, void * object) const;
	*/
    
	
	BaseObject(const BaseObject & /*b*/)
	{ }

	BaseObject & operator = (const BaseObject & /*b*/)
	{
		return *this;
	}
	
	Atomic<int32> referenceCount;

public:
	INTROSPECTION(BaseObject,
		MEMBER(referenceCount, "referenceCount", I_SAVE)
	);
};

template<typename T>
auto MakeSharedObject(T *obj) -> typename std::enable_if<std::is_base_of<BaseObject, T>::value, std::shared_ptr<T>>::type
{
    DVASSERT(nullptr != obj);
    obj->Retain();
    return std::shared_ptr<T>(obj, [](T *obj) { obj->Release(); });
}

/** 
	\ingroup baseobjects
	\brief	function to perform release safely. It checks if given object not equal to zero, in debug mode it also checks if such object
			haven't deallocated before and only if both checks is positive it call Release. After release it set value of variable to 0 to avoid 
			possible errors with usage of this variable
 */
template<class C>
void SafeRelease(C * &c) 
{ 
	if (c) 
	{
#ifdef ENABLE_BASE_OBJECT_CHECKS
		if (!BaseObjectChecker::IsAvailable(c))
		{
			DVASSERT(0 &&"SafeRelease Attempt to access unavailable BaseObject");
		}
#endif
		c->Release();
		c = 0;
	}
}
    

// /*#if defined(__DAVAENGINE_DIRECTX9__)*/
//template<>
//inline void SafeRelease<IUnknown>(IUnknown * &c) 
//{ 
//	if (c) 
//	{
//		c->Release();
//		c = 0;
//	}
//}
// // #endif 
/** 
	\ingroup baseobjects
	\brief	function to perform retain safely. Only if object exists it perform retain.
	\return same object with incremented reference count
*/
template<class C>
C * SafeRetain(C * c) 
{ 
	if (c) 
	{
#ifdef ENABLE_BASE_OBJECT_CHECKS
		BaseObject * c2 = dynamic_cast<BaseObject*>(c);
		if(c2)
		{
			if (!BaseObjectChecker::IsAvailable(c))
			{
				DVASSERT(0 &&"RetainedObject Attempt to access unavailable BaseObject");
			}
		}
#endif
		c->Retain();
	}
	return c;
}
    
    
    
using CreateObjectFunc = void* (*)();

class ObjectRegistrator
{
public:
    ObjectRegistrator(const String & name, CreateObjectFunc func, const std::type_info & typeinfo, uint32 size);
    ObjectRegistrator(const String & name, CreateObjectFunc func, const std::type_info & typeinfo, uint32 size, const String & alias);
};
	
#define REGISTER_CLASS(class_name) \
static void * Create##class_name()\
{\
return new class_name();\
};\
static ObjectRegistrator registrator##class_name(#class_name, &Create##class_name, typeid(class_name), sizeof(class_name));

#define REGISTER_CLASS_WITH_ALIAS(class_name, alias) \
static void * Create##class_name()\
{\
return new class_name();\
};\
static ObjectRegistrator registrator##class_name(#class_name, &Create##class_name, typeid(class_name), sizeof(class_name), alias);

    /*
     // tried to register every class that was marked by REGISTER_CLASS function;
     class_name * create_class##_class_name();\
     static size_t class_size = sizeof(create_class##_class_name());\

     */
	
/*template<class C>
C * SafeClone(C * object)
{
	if (object)
		return object->Clone();
	return 0;
}
	
// Boroda: Do not work when it's here, only works when it embedded into cpp file. WTF?
template<typename C>
RefPtr<C> SafeClone(const RefPtr<C> & object)
{
	if (object)
		return RefPtr<C>(object->Clone());
	return RefPtr<C>(0);
}
*/
}; 

/*
 For BaseObject* code is wrong / saved for potential use in other class
 
 #ifdef ENABLE_BASE_OBJECT_CHECKS
 virtual BaseObject& operator*() const throw() 
 {
 if (!BaseObjectChecker::IsAvailable((BaseObject*)this))
 {
 DVASSERT(0 &&"Attempt to access unavailable BaseObject");
 }	
 return *(BaseObject*)this; 
 }
 virtual BaseObject* operator->() const throw() 
 {
 if (!BaseObjectChecker::IsAvailable((BaseObject*)this))
 {
 DVASSERT(0 &&"Attempt to access unavailable BaseObject");
 }	
 return (BaseObject*)this; 
 }
 #endif 
 */


/*	
 Мысли на тему умного учета объектов и сериализации
 
 
 #define IMPLEMENT_CLASS(name)
 String GetName()
 {
 return String(#name);
 }	
 
 int		GetID()
 {
 return GlobalIdCounter++;
 }

 class A : public BaseObject
 {
 IMPLEMENT_CLASS(A);
 
 
 };
 
 */



#endif // __DAVAENGINE_BASEOBJECT_H__

