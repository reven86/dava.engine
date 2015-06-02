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


#ifndef __DAVAENGINE_INPUT_CALLBACK_H__
#define __DAVAENGINE_INPUT_CALLBACK_H__

#include "Base/BaseObject.h"

namespace DAVA 
{

class UIEvent;
class InputSystem;
class InputCallbackBase : public BaseObject
{
protected:
	virtual ~InputCallbackBase() {};
public:
	virtual void operator () (UIEvent *) = 0;
	virtual InputCallbackBase * Clone() const = 0;
	virtual bool IsEqual(const InputCallbackBase * event) const = 0;
};

template<class T>
class InputCallbackBaseClassFunctionImpl : public InputCallbackBase
{
	T * targetObject;
	void (T::*targetFunction)(UIEvent *);
public:
	InputCallbackBaseClassFunctionImpl(T *_object, void (T::*_function)(UIEvent *)) 
	{
		targetObject = _object;
		targetFunction = _function;
	}
	
	virtual void operator () (UIEvent * input)
	{
		(targetObject->*targetFunction)(input);
	}
	
	virtual InputCallbackBaseClassFunctionImpl * Clone() const
	{
		return new InputCallbackBaseClassFunctionImpl(targetObject, targetFunction);
	}
	
	virtual bool IsEqual(const InputCallbackBase * inputCallback) const
	{
		const InputCallbackBaseClassFunctionImpl<T> * t = dynamic_cast<const InputCallbackBaseClassFunctionImpl<T>*> (inputCallback);
		if (t != 0)
		{
			if (targetObject == t->targetObject && targetFunction == t->targetFunction)return true;
		}
		return false;
	}
};
	
class InputCallbackStaticFunctionImpl : public InputCallbackBase
{
	void (*targetFunction)(UIEvent *);
public:
	InputCallbackStaticFunctionImpl(void (*_function)(UIEvent *)) 
	{
		targetFunction = _function;
	}
	
	virtual void operator () (UIEvent * input)
	{
		(*targetFunction)(input);
	}
	
	virtual InputCallbackStaticFunctionImpl * Clone() const
	{
		return new InputCallbackStaticFunctionImpl(targetFunction);
	}
	
	virtual bool IsEqual(const InputCallbackBase * inputCallback) const
	{
		const InputCallbackStaticFunctionImpl * t = dynamic_cast<const InputCallbackStaticFunctionImpl*> (inputCallback);
		if (t != 0)
		{
			if (targetFunction == t->targetFunction)return true;
		}
		return false;
	}
};
	
class InputCallback
{
	InputCallbackBase * inputCallbackBase;
	uint32 devices;
public:
	
	template<class C>
	InputCallback(C * targetObject, void (C::*targetFunction)(UIEvent *), uint32 inputDevices)
	{
		inputCallbackBase = new InputCallbackBaseClassFunctionImpl<C>(targetObject, targetFunction);
		devices = inputDevices;
	}	

	InputCallback(void (*targetFunction)(UIEvent *), uint32 inputDevices)
	{
		inputCallbackBase = new InputCallbackStaticFunctionImpl(targetFunction);
		devices = inputDevices;
	}	
	
	InputCallback()
	:	inputCallbackBase(0),
	devices(0)
	{
	}
	
	~InputCallback()
	{
		SafeRelease(inputCallbackBase);
	}
	
	void operator() (UIEvent * input)
	{	
		if (inputCallbackBase) 
		{
			(*inputCallbackBase)(input);
		}
	}
	
	bool operator == (const InputCallback &inputCallback) const
	{
		return (inputCallbackBase->IsEqual(inputCallback.inputCallbackBase) && devices == inputCallback.devices);
	}

	InputCallback(const InputCallback & inputCallback)
	{
		inputCallbackBase = SafeRetain(inputCallback.inputCallbackBase);
		devices = inputCallback.devices;
	}
	
	InputCallback & operator =(const InputCallback & inputCallback)
	{
		SafeRelease(inputCallbackBase);
		inputCallbackBase = SafeRetain(inputCallback.inputCallbackBase);
		devices = inputCallback.devices;
		return *this;
	}
    
    bool IsEmpty() const
    {
        return (inputCallbackBase == 0);
    }

	friend class InputSystem;
};
};



#endif
