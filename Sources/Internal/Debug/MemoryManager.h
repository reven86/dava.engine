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
#include "Platform/Mutex.h"

#ifdef ENABLE_MEMORY_MANAGER

#include <string>
#include <map>
#include <fstream>

#ifdef ENABLE_MEMORY_MANAGER
#define IMPLEMENT_TAGGED_CREATOR(tag) \
void * operator new(std::size_t size) \
{ \
    TAG_SWITCH(tag) \
    return ::operator new(size); \
} \
\
void operator delete(void * ptr) \
{ \
    TAG_SWITCH(tag) \
    ::operator delete(ptr); \
} \

#define TAG_SWITCH(tag) \
AutoTagSwitcher tagSwitcher(tag);\

#else
#define IMPLEMENT_TAGGED_CREATOR(tag)
#define TAG_SWITCH(tag)
#endif


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

struct MemoryInfo
{
    MemoryInfo() : allocated(0), freed(0) {}
    uint32 allocated;
    uint32 freed;
};
    
class MemoryManager : public Singleton<MemoryManager>
{
public:
	MemoryManager() : mutex(TRUE), mapMutex(TRUE) {};
	virtual ~MemoryManager() {};
		
//	virtual void	*New(size_t size, const char * _file, int _line) { return 0; };
//	virtual void	*New(size_t size, void *pLoc, const char * _file, int _line) { return 0; };
//	virtual void	Delete(void * pointer) {};
	
	virtual void	CheckMemoryLeaks() {};
	virtual void	FinalLog() {};

    enum AllocTagType
    {
        TAG_UNTAGGED = 0,
        TAG_TEXTURE,
        TAG_POOL_ALLOCATOR,
        TAG_SHADER,
        TAG_SCENE,
        TAG_SPRITE,
        
        TAG_COUNT
    };
    
    const char* GetTagTypeAsString(AllocTagType tag) const
    {
        static const char* TagStrings[] = {"Untagged", "Textures", "Pool allocators", "Shaders", "Scene", "Sprite", ""};
        return TagStrings[tag];
    }

    void SetCurrentTag(AllocTagType newTag)
    {
        mapMutex.Lock();
        pthread_t threadID = pthread_self();
        tagsMap[threadID] = newTag;
        mapMutex.Unlock();
    }
    
    AllocTagType GetCurrentTag()
    {
        mapMutex.Lock();
        AllocTagType res = TAG_UNTAGGED;
        pthread_t threadID = pthread_self();
        Map<pthread_t, AllocTagType>::iterator it = tagsMap.find(threadID);
        if(it != tagsMap.end())
        {
            res = it->second;
        }
        mapMutex.Unlock();
        return res;
    }
    
    void DumpTaggedMemory()
    {
        Logger::FrameworkDebug("    M E M O R Y     M A N A G E R  T A G S  R E P O R T   ");
        Logger::FrameworkDebug("----------------------------------------------------");
        
        uint32 totalAlloc = 0;
        uint32 totalFreed = 0;
        for(int i = 0; i < TAG_COUNT; ++i)
        {
            totalAlloc += taggedMemInfoMap[i].allocated;
            totalFreed += taggedMemInfoMap[i].freed;
            Logger::FrameworkDebug("* Tag named %s allocated %.3f MB (%u bytes), freed %.3f MB (%u bytes), live %.3f MB (%u bytes)",
                                    GetTagTypeAsString((AllocTagType)i),
                                    taggedMemInfoMap[i].allocated/(1024.f*1024.f), taggedMemInfoMap[i].allocated,
                                    taggedMemInfoMap[i].freed/(1024.f*1024.f), taggedMemInfoMap[i].freed,
                                    (taggedMemInfoMap[i].allocated - taggedMemInfoMap[i].freed)/(1024.f*1024.f),
                                    taggedMemInfoMap[i].allocated - taggedMemInfoMap[i].freed);
        }
        Logger::FrameworkDebug("* Total allocated %.3f MB (%u bytes), freed %.3f MB (%u bytes), live %.3f MB (%u bytes)",
                               totalAlloc/(1024.f*1024.f), totalAlloc,
                               totalFreed/(1024.f*1024.f), totalFreed,
                               (totalAlloc - totalFreed)/(1024.f*1024.f), totalAlloc - totalFreed);
        Logger::FrameworkDebug("   F I N I S H E D   M E M O R Y  T A G S  R E P O R T   ");
    }

protected:
    Map<pthread_t, AllocTagType> tagsMap;
    MemoryInfo taggedMemInfoMap[TAG_COUNT];
    Mutex mutex;
    Mutex mapMutex;
};
    
class AutoTagSwitcher
{
public:
    AutoTagSwitcher(MemoryManager::AllocTagType newTag) : currTag(newTag)
    {
        prevTag = MemoryManager::Instance()->GetCurrentTag();
        MemoryManager::Instance()->SetCurrentTag(currTag);
    }
    
    ~AutoTagSwitcher()
    {
        MemoryManager::Instance()->SetCurrentTag(prevTag);
    }
    
private:
    MemoryManager::AllocTagType prevTag;
    MemoryManager::AllocTagType currTag;
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

