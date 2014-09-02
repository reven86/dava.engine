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


#ifndef __DAVAENGINE_MEMORYMANAGER_H__
#define __DAVAENGINE_MEMORYMANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/Singleton.h"
#include "DAVAConfig.h"

#ifdef ENABLE_MEMORY_MANAGER

#include <string>
#include <map>
#include <fstream>


namespace DAVA
{

/*
	Class for finding memory leaks / overruns and so on
	
 */

//class	MemoryManager : public Singleton<MemoryManager>
//{
//public:
//	void BeginAllocationTracking(const String & trackingId);
//	void EndAllocationTracking(const String & trackingId);
//	void CheckIfAllObjectsReleased(const String & trackingId);
//
//	void RegisterNewBaseObject(BaseObject * baseObject);
//	void UnregistedDeletedBaseObject(BaseObject * baseObject);
//private:
//};
	
class MemoryManager : public Singleton<MemoryManager>
{
public:
	MemoryManager(){};
	virtual ~MemoryManager() {};
		
//	virtual void	*New(size_t size, const char * _file, int _line) { return 0; };
//	virtual void	*New(size_t size, void *pLoc, const char * _file, int _line) { return 0; };
//	virtual void	Delete(void * pointer) {};
	
	virtual void	CheckMemoryLeaks() {};
	virtual void	FinalLog() {};
};
	
}; 


//void*	operator new(size_t _size, void* pLoc, const char *_file, int _line);
//void*	operator new(size_t _size, const char *_file, int _line);
//void*	operator new[](size_t _size, const char *_file, int _line);
////void	operator delete[](void * ptr);

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
void * operator new(size_t _size) throw(std::bad_alloc);
void * operator new(size_t _size, const std::nothrow_t &) throw();

void   operator delete(void * ptr) throw();
void   operator delete(void * ptr, const std::nothrow_t &) throw();

void * operator new[](size_t _size) throw(std::bad_alloc);
void * operator new[](size_t _size, const std::nothrow_t &) throw();

void   operator delete[](void * ptr) throw();
void   operator delete[](void * ptr, const std::nothrow_t &) throw();
#else defined(__DAVAENGINE_WIN32__)
void * operator new(size_t _size) throw();
void * operator new(size_t _size, const std::nothrow_t &) throw();

void   operator delete(void * ptr) throw();
void   operator delete(void * ptr, const std::nothrow_t &) throw();

void * operator new[](size_t _size) throw();
void * operator new[](size_t _size, const std::nothrow_t &) throw();

void   operator delete[](void * ptr) throw();
void   operator delete[](void * ptr, const std::nothrow_t &) throw();
#endif


//// Default placement versions of operator new.
//inline void* operator new(std::size_t, void* __p) throw() { return __p; }
//inline void* operator new[](std::size_t, void* __p) throw() { return __p; }
//
//// Default placement versions of operator delete.
//inline void  operator delete  (void*, void*) throw() { }
//inline void  operator delete[](void*, void*) throw() { }

//#define new new (__FILE__, __LINE__)

#endif // ENABLE_MEMORY_MANAGER
#endif // __DAVAENGINE_MEMORYMANAGER_H__

