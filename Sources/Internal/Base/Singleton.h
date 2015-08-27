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


#ifndef __DAVAENGINE_SINGLETON_H__
#define __DAVAENGINE_SINGLETON_H__

namespace DAVA
{

// Singleton	

template <typename T>
class Singleton
{
public:
	Singleton()
	{
		// TODO: Add assertion here DVASSERT(instance == 0 && "Singleton object should be initialized only once");
		if (instance == nullptr)
		{
            // we need here dynamic_cast but can't use because: 
            // http://en.cppreference.com/w/cpp/language/dynamic_cast
            // 6 - part
            // error can appiar if use write: class A: public B, public C, public Singleton<A>
            // better always use: class A: public Singleton<A>, public B, public C
            // so if I use static_cast all works
			instance = static_cast<T*>(this);
		}
	}
	virtual ~Singleton()
	{
		instance = nullptr;
	}
	
	static T * Instance() { return instance; }
    
	void Release()
	{
		if (this)
		{
			delete this;
			instance = nullptr;
		}else {
			// TODO: add DebugBreak();
			//DVASSERT(0 && "Attempt to delete singleton second time");
		}

		return;
	}
private:
	static T * instance;
};
template <typename T> 
T * Singleton<T>::instance = nullptr;
	
};
#endif //__DAVAENGINE_SINGLETON_H__


