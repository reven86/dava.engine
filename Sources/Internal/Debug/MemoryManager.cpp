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


#include "DAVAConfig.h"
#include "Debug/MemoryManager.h"
#include "Debug/List.h"
#include "FileSystem/Logger.h"
#include "Debug/Backtrace.h"
#include <assert.h>
#include "Base/Mallocator.h"
#include "Utils/StringFormat.h"
#include "Base/MemoryRoutines.h"
#include "Platform/Mutex.h"


#ifdef ENABLE_MEMORY_MANAGER

#ifdef new
#undef new
#endif

namespace DAVA 
{
    
struct MemoryBlock
{
    MemoryBlock * next;             // 8 or 4
    MemoryBlock * prev;             // 8 or 4
#if defined(ENABLE_MEMORY_BACKTRACE)
    Backtrace   * backtrace;        // 8
#endif
    uint32        size;             // 4
    uint32        flags;            // 4
#if defined(_WIN32) || ( defined(__DAVAENGINE_IPHONE__) && defined(ENABLE_MEMORY_BACKTRACE) )
	uint32		  align[3];
#endif
    
    // TOTAL: 32 bytes.
    
//	char			* filename;
//	int				line;
//	int				size;
//	bool			isPlaced;  // C++ PL 10.4.11
//	void			* ptr;
};

	
    
//struct BacktraceTreeNode
//{
//    void * ptr;
//    MallocList<BacktraceTreeNode*> nodes;
//    Backtrace * backtrace;
//    
//    
//    void Insert(BacktraceTreeNode * node, Backtrace * backtrace, uint32 depth)
//    {
//        if (node->ptr != backtrace->array[depth]) // try to insert to wrong node
//            return;
//        
//        if (node->ptr == backtrace->array[depth])
//        {
//            // same backtrace found
//            if (depth == backtrace->size)return;
//                
//            for (MallocList<BacktraceTreeNode*>::Iterator it = nodes->Begin(); it != nodes->End(); ++it)
//            {
//                BacktraceTreeNode * childNode = *it;
//                if (childNode->
//            }
//        }
//        
//    }
//    
//    Backtrace * Find(Backtrace * backtrace, BacktraceTreeNode * node, uint32 depth)
//    {
//        if ((backtrace->array[depth] == node->ptr) && (backtrace->size == depth))
//            return backtrace;
//        
//        for (MallocList<BacktraceTreeNode*>::Iterator it = nodes->Begin(); it != nodes->End(); ++it)
//        {
//            BacktraceTreeNode * childNode = *it;
//            if (childNode->ptr)
//            {
//                
//            }
//        }
//    };
//};

struct BackTraceLessCompare
{
    bool operator() (const Backtrace * bt1, const Backtrace * bt2)
    {
        uint32 minSize = Min(bt1->size, bt2->size);
        for (uint32 k = 0; k < minSize; ++k)
            if (bt1->array[k] < bt2->array[k])
            {
                return true;
            }else if (bt1->array[k] > bt2->array[k])
			{
				return false;
			}
        return false;
    };
};
	
class MemoryManagerImpl : public MemoryManager
{
public:
#if defined(ENABLE_HEAP_CHECK)
    static const uint32 CHECK_SIZE = 32;
#else
    static const uint32 CHECK_SIZE = 0;
#endif
    static const uint32 CHECK_CODE_HEAD = 0xCEECC0DE;
    static const uint32 CHECK_CODE_TAIL = 0xFEEDC0DE;
    static const uint32 FROM_PTR_TO_MEMORY_BLOCK = sizeof(MemoryBlock) + 16;
    
	MemoryManagerImpl();
	virtual ~MemoryManagerImpl();
	
	virtual void	*New(size_t size);
	//virtual void	*New(size_t size, void *pLoc);
	virtual void	Delete(void * pointer);
    
	
	virtual void	CheckMemoryLeaks();
	virtual void	FinalLog();
	void	CheckMemblockOverrun(MemoryBlock * memBlock);

	static MemoryManagerImpl * Instance() 
	{
		//static MemoryManagerImpl instance;
		//return &instance;
		if (instance_new == 0)
		{
			uint32 sizeofMemoryBlock = sizeof(MemoryBlock);
            assert(sizeofMemoryBlock % 16 == 0);
			void * addr = malloc(sizeof(MemoryManagerImpl));
			instance_new = new(addr) MemoryManagerImpl(); //(sizeof(MemoryManagerImpl), addr);
		}
		return instance_new;					 
	} 
    
    enum
    {
        FLAG_LEAK_TRACKING_ENABLED = 1,
        FLAG_BASE_OBJECT = 2,
    };
    
    uint32 currentFlags;
    inline void DisableLeakTracking()
    {
        currentFlags &= ~FLAG_LEAK_TRACKING_ENABLED;
    }
    
    inline void EnableLeakTracking()
    {
        currentFlags |= FLAG_LEAK_TRACKING_ENABLED;
    }

    inline void PushMemoryBlock(MemoryBlock * item)
    {
        mutex.Lock();
        MemoryBlock * insertAfter = headBlock;
        item->next = insertAfter->next;
        item->prev = insertAfter;
        
        insertAfter->next->prev = item;
        insertAfter->next = item;
        blockCount++;
        mutex.Unlock();
    }
    
    inline void EraseMemoryBlock(MemoryBlock * item)
    {
        mutex.Lock();
        item->prev->next = item->next;
        item->next->prev = item->prev;
        blockCount--;
        mutex.Unlock();
    }
    
    inline void NewUpdateStats(MemoryBlock * block)
    {
        mutex.Lock();
        MemoryInfo& info = taggedMemInfoMap[GetCurrentTag()];
        if (block->size > maximumBlockSize)
        {
            maximumBlockSize = block->size;
        }
        currentAllocatedMemory += block->size;
        if (currentAllocatedMemory > peakMemoryUsage)
        {
            peakMemoryUsage = currentAllocatedMemory;
        }
        managerOverheadSize += CHECK_SIZE + sizeof(MemoryBlock);
        if (managerOverheadSize > peakManagerOverheadSize)
            peakManagerOverheadSize = managerOverheadSize;
        info.allocated += block->size;
        mutex.Unlock();
    }
    
    inline void DeleteUpdateStats(MemoryBlock * block)
    {
        mutex.Lock();
        MemoryInfo& info = taggedMemInfoMap[GetCurrentTag()];
        currentAllocatedMemory -= block->size;
        managerOverheadSize -= CHECK_SIZE + sizeof(MemoryBlock);
        info.freed += block->size;
        mutex.Unlock();
    }
    
    // Tail of the memory block list / Circular list
    MemoryBlock * headBlock;
    uint32 blockCount;
    MallocList<Backtrace*> backtraceList;
    
    typedef std::set<Backtrace*, BackTraceLessCompare, Mallocator<Backtrace*> > BacktraceSet;
    BacktraceSet backtraceSet;
    
    // Array for fast detection of block overwrites
    static const uint32 MAX_OVERWRITTEN_BLOCKS = 16;
    bool        overwrittenCount;
    MemoryBlock * overwrittenArray[MAX_OVERWRITTEN_BLOCKS];
                               
    // Statistics
	int		peakMemoryUsage;
	uint32	maximumBlockSize;
	int		currentAllocatedMemory;
    int     managerOverheadSize;
    int     peakManagerOverheadSize;
	
	static int useClass;
	static MemoryManagerImpl * instance_new;
};
	
	int MemoryManagerImpl::useClass = 0;
	MemoryManagerImpl * MemoryManagerImpl::instance_new = 0;


}

void * operator new(size_t _size) throw(std::bad_alloc )
{
#if defined(__USE_OWN_ALLOCATORS__)
    void *p = NULL;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        p = pAllocStack->Allocate(_size);
    }
    if(!p)
    {
        p = DAVA::MemoryManagerImpl::Instance()->New(_size);
        if (p == NULL) {
            throw std::bad_alloc();
        }
    }
    return p;
#else
	return DAVA::MemoryManagerImpl::Instance()->New(_size);
#endif
}

void * operator new(size_t _size, const std::nothrow_t &) throw()
{
#if defined(__USE_OWN_ALLOCATORS__)
    void *p = NULL;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        p = pAllocStack->Allocate(_size);
    }
    if(!p)
    {
        p = DAVA::MemoryManagerImpl::Instance()->New(_size);
    }
    return p;
#else
	return DAVA::MemoryManagerImpl::Instance()->New(_size);
#endif
}

void   operator delete(void * _ptr) throw()
{
#if defined(__USE_OWN_ALLOCATORS__)
    bool bDealocated = false;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        bDealocated = pAllocStack->Deallocate(_ptr);
    }
    if(!bDealocated)
    {
        DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
    }
#else
	DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
#endif
}

void   operator delete(void * _ptr, const std::nothrow_t &) throw()
{
#if defined(__USE_OWN_ALLOCATORS__)
    bool bDealocated = false;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        bDealocated = pAllocStack->Deallocate(_ptr);
    }
    if(!bDealocated)
    {
        DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
    }
#else
	DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
#endif
}


void * operator new[](size_t _size) throw(std::bad_alloc)
{
#if defined(__USE_OWN_ALLOCATORS__)
    void *p = NULL;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        p = pAllocStack->Allocate(_size);
    }
    if(!p)
    {
        p = DAVA::MemoryManagerImpl::Instance()->New(_size);
        if (p == NULL) {
            throw std::bad_alloc();
        }
    }
    return p;
#else
	return DAVA::MemoryManagerImpl::Instance()->New(_size);
#endif
}

void * operator new[](size_t _size, const std::nothrow_t &) throw()
{
#if defined(__USE_OWN_ALLOCATORS__)
    void *p = NULL;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        p = pAllocStack->Allocate(_size);
    }
    if(!p)
    {
        p = DAVA::MemoryManagerImpl::Instance()->New(_size);
    }
    return p;
#else
	return DAVA::MemoryManagerImpl::Instance()->New(_size);
#endif
}

void   operator delete[](void * _ptr) throw()
{
#if defined(__USE_OWN_ALLOCATORS__)
    bool bDealocated = false;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        bDealocated = pAllocStack->Deallocate(_ptr);
    }
    if(!bDealocated)
    {
        DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
    }
#else
	DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
#endif
}

void   operator delete[](void * _ptr, const std::nothrow_t &) throw()
{
#if defined(__USE_OWN_ALLOCATORS__)
    bool bDealocated = false;
    DAVA::AllocatorsStack *pAllocStack = DAVA::AllocatorsStack::Instance();
    if(pAllocStack)
    {
        bDealocated = pAllocStack->Deallocate(_ptr);
    }
    if(!bDealocated)
    {
        DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
    }
#else
	DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
#endif
}

//void*	operator new(size_t _size, void* pLoc) throw()
//{
//	return DAVA::MemoryManagerImpl::Instance()->New(_size, pLoc);
//}

namespace DAVA 
{
	


MemoryManagerImpl::MemoryManagerImpl()
{
	//memoryLog.open("memory.log");
	//memoryLog << "          M E M O R Y   M A N A G E R   L O G " << std::endl;
	//memoryLog << "----------------------------------------------------------------------" << std::endl;
	// clean statistics
    headBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock*)); 
    headBlock->next = headBlock;
    headBlock->prev = headBlock;
    blockCount = 0;
    
    overwrittenCount = 0;
    for (uint32 k = 0; k < MAX_OVERWRITTEN_BLOCKS; ++k)
        overwrittenArray[k] = 0;
    
	currentAllocatedMemory = 0;
	peakMemoryUsage = 0;
	maximumBlockSize = 0;
    managerOverheadSize = 0;
    peakManagerOverheadSize = 0;
    currentFlags = FLAG_LEAK_TRACKING_ENABLED;
}

MemoryManagerImpl::~MemoryManagerImpl()
{
    free(headBlock);
}

//void *malloc16 (size_t s) {
//    unsigned char *p;
//    unsigned char *porig = (unsigned char*)malloc (s + 0x10);   // allocate extra
//    if (porig == NULL) return NULL;             // catch out of memory
//    p = (porig + 16) & (~0xf);                  // insert padding
//    *(p-1) = p - porig;                         // store padding size
//    return p;
//}
//
//void free16(void *p) {
//    unsigned char *porig = (unsigned char*)p;                   // work out original
//    porig = porig - *(porig-1);                 // by subtracting padding
//    free (porig);                               // then free that
//}

void	*MemoryManagerImpl::New(size_t size)
{
#if defined(ENABLE_MEMORY_INFO_TRACKING)
    // Align to 4 byte boundary.
    if ((size & 15) != 0)
	{
		size += 16 - (size & 15);
	}
    uint32 memBlockSize = sizeof(MemoryBlock);
//    uint32 allocSize = sizeof(MemoryBlock) + size + CHECK_SIZE;
    MemoryBlock * block = (MemoryBlock *)malloc(sizeof(MemoryBlock) + CHECK_SIZE + size);
    
    //if (!block)throw std::bad_alloc();
    
#if defined(ENABLE_MEMORY_BACKTRACE)
    mutex.Lock();
    Backtrace bt;
    GetBacktrace(&bt);
    
    BacktraceSet::iterator it = backtraceSet.find(&bt);
    if (it != backtraceSet.end())
    {
        block->backtrace = *it;
    }else
    {
        Backtrace * backtrace = CreateBacktrace();
        GetBacktrace(backtrace);
        block->backtrace = backtrace;
        backtraceSet.insert(backtrace);
    }
    mutex.Unlock();
#endif

    block->size = size;
    block->flags = currentFlags;
    
    PushMemoryBlock(block);
    NewUpdateStats(block);

#if defined(ENABLE_HEAP_CHECK)
    uint8 * checkHead = (uint8*)block + sizeof(MemoryBlock);
    uint8 * checkTail = (uint8*)block + sizeof(MemoryBlock) + size + CHECK_SIZE - 16;
    uint32 * checkHeadWrite = (uint32*)checkHead;
    uint32 * checkTailWrite = (uint32*)checkTail;
    *checkHeadWrite++ = CHECK_CODE_HEAD;
    *checkHeadWrite = CHECK_CODE_HEAD;
    *checkTailWrite++ = CHECK_CODE_TAIL;
    *checkTailWrite = CHECK_CODE_TAIL;
#endif
    
    uint8 * outmemBlock = (uint8*)block;
#if defined(ENABLE_HEAP_CHECK)
    outmemBlock += sizeof(MemoryBlock) + 16;
#else
    outmemBlock += sizeof(MemoryBlock);
#endif
    //Logger::FrameworkDebug("malloc: %p %p", block, outmemBlock);
    return outmemBlock;
#else
    return malloc(size);
#endif
}

//void	*MemoryManagerImpl::New(size_t size, void *pLoc, const char * _file, int _line)
//{
//}
	
void	MemoryManagerImpl::CheckMemblockOverrun(MemoryBlock * memBlock)
{
//	uint32 * fill = (uint32*)memBlock.ptr;
//	uint32 * fillEnd = (uint32*)memBlock.ptr + CHECK_MEMORY_OVERRUNS * 2 + (memBlock.size / 4) - 1;
//	for (int k = 0; k < CHECK_MEMORY_OVERRUNS; ++k)
//	{
//		if (*fill != CHECK_CODE)
//		{
//			Logger::Error("* Find head overrun for block allocated: %s, line:%d", memBlock.filename, memBlock.line);
//			*fill++; 
//		}
//		if (*fillEnd != CHECK_CODE)
//		{
//			Logger::Error("* Find tail overrun for block allocated: %s, line:%d", memBlock.filename, memBlock.line);
//			*fillEnd--;
//		}
//	}
}
            
void	MemoryManagerImpl::Delete(void * ptr)
{
#if defined(ENABLE_MEMORY_INFO_TRACKING)
	if (ptr)
	{
//        bool found = false;
//        for (MallocList<void*>::Iterator it = allocatedPointers.Begin(); it != allocatedPointers.End(); ++it)
//        {
//            if (*it == ptr)
//            {
//                allocatedPointers.Erase(it);
//                found = true;
//                break;
//            }
//        }
//        
//        if (!found)
//        {
//            found = false;
//        }
        
        uint8 * uint8ptr = (uint8*)ptr;
#if defined(ENABLE_HEAP_CHECK)
        uint8ptr -= sizeof(MemoryBlock) + 16;
#else
        uint8ptr -= sizeof(MemoryBlock);
#endif
        MemoryBlock * block = (MemoryBlock*)(uint8ptr);
        
#if defined(ENABLE_HEAP_CHECK)
        uint8 * checkHead = (uint8*)block + sizeof(MemoryBlock);
        uint8 * checkTail = (uint8*)block + sizeof(MemoryBlock) + block->size + CHECK_SIZE - 16;
        uint32 * checkHeadRead = (uint32*)checkHead;
        uint32 * checkTailRead = (uint32*)checkTail;

        if ((checkHeadRead[0] != CHECK_CODE_HEAD) || 
            (checkHeadRead[1] != CHECK_CODE_HEAD) || 
            (checkTailRead[0] != CHECK_CODE_TAIL) || 
            (checkTailRead[1] != CHECK_CODE_TAIL))
        {
            Logger::Error("fatal: block: %p overwritten during usage", ptr);
        }
#endif
        //Logger::FrameworkDebug("free: %p %p", block, ptr);
        
        DeleteUpdateStats(block);
        EraseMemoryBlock(block);
        free(block);
	}
#else
    free(ptr);
#endif
}

    
// TODO: Add smart printing of sizes
void MemoryManagerImpl::FinalLog()
{
    DisableLeakTracking();
    
	Logger::FrameworkDebug("    M E M O R Y     M A N A G E R     R E P O R T   ");
	Logger::FrameworkDebug("----------------------------------------------------");
	// 
	Logger::FrameworkDebug("* Currently Allocated Memory Size : %d", currentAllocatedMemory);
	Logger::FrameworkDebug("* Memory Leaks Count: %d", blockCount);
	Logger::FrameworkDebug("* Peak Memory Usage : %d", peakMemoryUsage);
	Logger::FrameworkDebug("* Max Allocated Block Size: %d", maximumBlockSize);
    
	Logger::FrameworkDebug("* Peak Manager Overhead: %d", peakManagerOverheadSize);
	Logger::FrameworkDebug("* Current Manager Overhead : %d", managerOverheadSize);
    Logger::FrameworkDebug("* Overhead percentage: %0.3f %%", (float)peakManagerOverheadSize / (float)peakMemoryUsage * 100.0f);
    Logger::FrameworkDebug("* Total backtrace count: %d", backtraceSet.size()); 
	
    
    EnableLeakTracking();
    
    CheckMemoryLeaks();
}

void MemoryManagerImpl::CheckMemoryLeaks()
{
    DisableLeakTracking();
    
    std::ofstream memoryLog;
    memoryLog.open("/memory.log");

 	memoryLog << Format("    M E M O R Y     M A N A G E R     R E P O R T   ");
	memoryLog << Format("----------------------------------------------------");
	// 
	memoryLog << Format("* Currently Allocated Memory Size : %d", currentAllocatedMemory) << std::endl;
	memoryLog << Format("* Memory Leaks Count: %d", blockCount) << std::endl;
	memoryLog << Format("* Peak Memory Usage : %d", peakMemoryUsage) << std::endl;
	memoryLog << Format("* Max Allocated Block Size: %d", maximumBlockSize) << std::endl;
    
	memoryLog << Format("* Peak Manager Overhead: %d", peakManagerOverheadSize) << std::endl;
	memoryLog << Format("* Current Manager Overhead : %d", managerOverheadSize) << std::endl;
    memoryLog << Format("* Overhead percentage: %0.3f %%", (float)peakManagerOverheadSize / (float)peakMemoryUsage * 100.0f) << std::endl;
    memoryLog << Format("* Total backtrace count: %d", backtraceSet.size()) << std::endl; 

    uint32 index = 0;
	for (MemoryBlock * currentBlock = headBlock->next; currentBlock != headBlock; currentBlock = currentBlock->next)
	{
        index++;
		//MemoryBlock * block = currentBlock;
		//CheckMemblockOverrun(block);
		//if (block.filename)
        if (currentBlock->flags & FLAG_LEAK_TRACKING_ENABLED)
		{
			//Logger::FrameworkDebug("****** 	Memory Leak Found	******");
            memoryLog << "****** 	Memory Leak Found	******" << std::endl;
			//Logger::FrameworkDebug("* Source File : %s", block->filename);
			//Logger::FrameworkDebug("* Source Line : %d", block->line);
			//Logger::FrameworkDebug("* Allocation Size : %d bytes", currentBlock->size);
            memoryLog << Format("* Allocation Size : %d bytes", currentBlock->size) << std::endl;
#if defined(ENABLE_MEMORY_BACKTRACE)
            BacktraceLog log;
            CreateBacktraceLog(currentBlock->backtrace, &log);
            for (uint32 k = 0; k < currentBlock->backtrace->size; ++k) 
            {
                //Logger::FrameworkDebug("%s",log.strings[k]); 
                memoryLog << log.strings[k] << std::endl;
            }
            ReleaseBacktraceLog(&log);
#endif
			//Logger::FrameworkDebug("* Was placed : " << (block.isPlaced ? " true" : " false")<< std::endl;
			//Logger::FrameworkDebug("************************************");
		}
	}
    memoryLog.close();
    EnableLeakTracking();
}

};

#endif // MEMORY_MANAGER

