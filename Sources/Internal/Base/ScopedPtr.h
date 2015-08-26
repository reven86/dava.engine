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


#ifndef __DAVAENGINE_SCOPED_PTR_H__
#define __DAVAENGINE_SCOPED_PTR_H__

#include "Base/BaseTypes.h"
#include "FileSystem/Logger.h"

namespace DAVA
{

template<typename BASE_OBJECT>
class ScopedPtr
{
public:
	explicit ScopedPtr(BASE_OBJECT * p);
	~ScopedPtr();
	BASE_OBJECT & operator*() const;
	BASE_OBJECT * operator->() const;
	operator BASE_OBJECT*() const;
    BASE_OBJECT* get() const;
    void reset(BASE_OBJECT* p = nullptr);
    explicit operator bool() const;
	
	//protection from 'delete ScopedObject'
	operator void*() const;
	
	ScopedPtr(const ScopedPtr&);
	const ScopedPtr& operator=(const ScopedPtr&);
    const ScopedPtr& operator=(BASE_OBJECT* p);

private:
	BASE_OBJECT * object;
};

//implementation
template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::ScopedPtr(BASE_OBJECT * p)
:	object(p)
{
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::ScopedPtr(const ScopedPtr& scopedPtr)
{
	object = scopedPtr.object;
	object->Retain();
}

template<typename BASE_OBJECT>
const ScopedPtr<BASE_OBJECT>& ScopedPtr<BASE_OBJECT>::operator=(const ScopedPtr& scopedPtr)
{
	if (this == &scopedPtr)
	{
		return *this;
	}

    SafeRelease(object);
	object = scopedPtr.object;
	object->Retain();

	return *this;
}

template<typename BASE_OBJECT>
const ScopedPtr<BASE_OBJECT>& ScopedPtr<BASE_OBJECT>::operator=(BASE_OBJECT* p)
{
    if (p != object)
    {
        SafeRelease(object);
        object = p;
    }

    return *this;
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::~ScopedPtr()
{
	SafeRelease(object);
}

template<typename BASE_OBJECT>
BASE_OBJECT & ScopedPtr<BASE_OBJECT>::operator*() const
{
	return *object;
}

template<typename BASE_OBJECT>
BASE_OBJECT * ScopedPtr<BASE_OBJECT>::operator->() const
{
	return object;
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::operator BASE_OBJECT*() const
{
	return object;
}

template<typename BASE_OBJECT>
BASE_OBJECT* ScopedPtr<BASE_OBJECT>::get() const
{
    return object;
}

template<typename BASE_OBJECT>
void ScopedPtr<BASE_OBJECT>::reset(BASE_OBJECT* p)
{
    if (p != object)
    {
        SafeRelease(object);
        object = p;
    }
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::operator bool() const
{
    return object != nullptr;
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::operator void*() const
{
	return object;
}

};

#endif //__DAVAENGINE_SCOPED_PTR_H__