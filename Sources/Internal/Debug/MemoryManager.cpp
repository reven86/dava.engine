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
#include "Platform/Thread.h"
#include "FileSystem/FileSystem.h"
#include "Base/FastName.h"
#include "Base/ObjectFactory.h"
#include <functional>
#include <sstream>
#include "Debug/Stats.h"

#if defined(__DAVAENGINE_WIN32__)
#include <malloc.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include <malloc.h>
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <malloc/malloc.h>
#endif

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
    Backtrace   * backtrace;        // 8
    uint32        size;             // 4
    uint32        mallocSize;
    uint32        flags;            // 4
    FastName      userInfo;         //
    eMemoryPool   poolIndex;
//#if defined(_WIN32)
	uint32		  align[1];
//#endif
    
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
    static const uint32 CHECK_SIZE = 32;
    static const uint32 HALF_CHECK_SIZE = CHECK_SIZE / 2;
    
    static const uint32 CHECK_CODE_HEAD = 0xCEECC0DE;
    static const uint32 CHECK_CODE_TAIL = 0xFEEDC0DE;
    static const uint32 FROM_PTR_TO_MEMORY_BLOCK = sizeof(MemoryBlock) + 16;
    
	MemoryManagerImpl();
	virtual ~MemoryManagerImpl();
	
	virtual void	*New(size_t size, eMemoryPool poolIndex, const char * userInfo);
	virtual void	Delete(void * pointer);
    void            DumpLeaks(File * leaksLogFile, BacktraceTree::BacktraceTreeNode * node, int32 depth);
	virtual void	DumpLog(uint32_t dumpFlags);
    
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
    
    // Statistics
    struct MemoryPoolStatistics
    {
        MemoryPoolStatistics()
        {
            currentAllocatedMemory = 0;
            realAllocatedMemory = 0;
            peakMemoryUsage = 0;
            maximumBlockSize = 0;
            managerOverheadSize = 0;
            peakManagerOverheadSize = 0;
        }
        
        int		peakMemoryUsage;
        uint32	maximumBlockSize;
        int		currentAllocatedMemory;
        int     realAllocatedMemory;
        int     managerOverheadSize;
        int     peakManagerOverheadSize;
    };
    
    // Tail of the memory block list / Circular list
    struct MemoryPoolInfo
    {
        MemoryPoolInfo()
        {
            headBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock*));
            headBlock->next = headBlock;
            headBlock->prev = headBlock;
            blockCount = 0;
        }
        
        ~MemoryPoolInfo()
        {
            free(headBlock);
        }
        
        MemoryBlock * headBlock;
        uint32 blockCount;
        MemoryPoolStatistics stats;
    };

    inline void PushMemoryBlock(MemoryBlock * item)
    {
        MemoryPoolInfo * info = &memoryPools[item->poolIndex];
        
        MemoryBlock * insertAfter = info->headBlock;
        item->next = insertAfter->next;
        item->prev = insertAfter;
        
        insertAfter->next->prev = item;
        insertAfter->next = item;
        info->blockCount++;
        
        NewUpdateStats(item, info->stats);
        NewUpdateStats(item, globalStats);
    }
    
    inline void EraseMemoryBlock(MemoryBlock * item)
    {
        MemoryPoolInfo * info = &memoryPools[item->poolIndex];
        item->prev->next = item->next;
        item->next->prev = item->prev;
        info->blockCount--;
        
        DeleteUpdateStats(item, info->stats);
        DeleteUpdateStats(item, globalStats);
    }
    
    inline void NewUpdateStats(MemoryBlock * block, MemoryPoolStatistics & stats)
    {
        if (block->size > stats.maximumBlockSize)
        {
            stats.maximumBlockSize = block->size;
        }
        stats.currentAllocatedMemory += block->size;
        stats.realAllocatedMemory += block->mallocSize;
        
        if (stats.currentAllocatedMemory > stats.peakMemoryUsage)
        {
            stats.peakMemoryUsage = stats.currentAllocatedMemory;
        }
        
        stats.managerOverheadSize += CHECK_SIZE + sizeof(MemoryBlock);
        if (stats.managerOverheadSize > stats.peakManagerOverheadSize)
            stats.peakManagerOverheadSize = stats.managerOverheadSize;
    }
    
    inline void DeleteUpdateStats(MemoryBlock * block, MemoryPoolStatistics & stats)
    {
        stats.realAllocatedMemory -= block->mallocSize;
        stats.currentAllocatedMemory -= block->size;
        stats.managerOverheadSize -= CHECK_SIZE + sizeof(MemoryBlock);
    }
    
    MemoryPoolInfo memoryPools[MEMORY_POOL_COUNT];
    MemoryPoolStatistics globalStats;
    
    
    typedef std::set<Backtrace*, BackTraceLessCompare, Mallocator<Backtrace*> > BacktraceSet;
    BacktraceSet backtraceSet;
    
    // Array for fast detection of block overwrites
    static const uint32 MAX_OVERWRITTEN_BLOCKS = 16;
    bool        overwrittenCount;
    MemoryBlock * overwrittenArray[MAX_OVERWRITTEN_BLOCKS];
                               
    
    Mutex   mutex;
	
	static int useClass;
	static MemoryManagerImpl * instance_new;
};
	
	int MemoryManagerImpl::useClass = 0;
	MemoryManagerImpl * MemoryManagerImpl::instance_new = 0;
}

//DAVA::FastName defaultUserInfo;

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
void * operator new(size_t _size) throw(std::bad_alloc)
#elif defined(__DAVAENGINE_WIN32__)
void * operator new(size_t _size)
#endif
{
	return DAVA::MemoryManagerImpl::Instance()->New(_size, MEMORY_POOL_DEFAULT, 0);
}

void * operator new(size_t _size, const std::nothrow_t &) throw()
{
	return DAVA::MemoryManagerImpl::Instance()->New(_size, MEMORY_POOL_DEFAULT, 0);
}

void   operator delete(void * _ptr) throw()
{
	DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
}

void   operator delete(void * _ptr, const std::nothrow_t &) throw()
{
	DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
}

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
void * operator new[](size_t _size) throw(std::bad_alloc)
#elif defined(__DAVAENGINE_WIN32__)
void * operator new[](size_t _size)
#endif
{
	return DAVA::MemoryManagerImpl::Instance()->New(_size, MEMORY_POOL_DEFAULT, 0);
}

void * operator new[](size_t _size, const std::nothrow_t &) throw()
{
	return DAVA::MemoryManagerImpl::Instance()->New(_size, MEMORY_POOL_DEFAULT, 0);
}

void   operator delete[](void * _ptr) throw()
{
	DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
}

void   operator delete[](void * _ptr, const std::nothrow_t &) throw()
{
	DAVA::MemoryManagerImpl::Instance()->Delete(_ptr);
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
    uint32 checkMemoryBlockSize = sizeof(MemoryBlock);
    DVASSERT((checkMemoryBlockSize % 16) == 0)
    
    
    overwrittenCount = 0;
    for (uint32 k = 0; k < MAX_OVERWRITTEN_BLOCKS; ++k)
        overwrittenArray[k] = 0;
    
    currentFlags = FLAG_LEAK_TRACKING_ENABLED;
}

MemoryManagerImpl::~MemoryManagerImpl()
{
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
    
#define MEMORY_BLOCK_TO_PTR(x) ((uint8*)x + sizeof(MemoryBlock) + HALF_CHECK_SIZE)
#define PTR_TO_MEMORY_BLOCK(x) (MemoryBlock*)((uint8*)x - sizeof(MemoryBlock) - HALF_CHECK_SIZE)

void	*MemoryManagerImpl::New(size_t size, eMemoryPool poolIndex, const char * userInfo)
{
    mutex.Lock();

    // Align to 4 byte boundary.
    if ((size & 15) != 0)
	{
		size += 16 - (size & 15);
	}
    
    MemoryBlock * block = (MemoryBlock *)malloc(sizeof(MemoryBlock) + CHECK_SIZE + size);
    

    //if (!block)throw std::bad_alloc();
    
    Backtrace bt;
    GetBacktrace(&bt, 2);
    
    BacktraceSet::iterator it = backtraceSet.find(&bt);
    if (it != backtraceSet.end())
    {
        block->backtrace = *it;
    }else
    {
        Backtrace * backtrace = CreateBacktrace();
        *backtrace = bt;
        block->backtrace = backtrace;
        backtraceSet.insert(backtrace);
    }

    block->size = size;
#if defined(__DAVAENGINE_WIN32__)
    block->mallocSize = static_cast<uint32>(_msize(block));
#elif defined(__DAVAENGINE_ANDROID__)
#error "cannot use malloc_usable_size on android"
    block->mallocSize = 0;
    //block->mallocSize = static_cast<uint32>(malloc_usable_size(block));
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    block->mallocSize = static_cast<uint32>(malloc_size(block));
#else
#error "Platform not supported"
#endif
    block->flags = currentFlags;
    block->poolIndex = poolIndex;
    
    PushMemoryBlock(block);

    uint8 * checkHead = (uint8*)block + sizeof(MemoryBlock);
    uint8 * checkTail = (uint8*)block + sizeof(MemoryBlock) + size + CHECK_SIZE - HALF_CHECK_SIZE;
    uint32 * checkHeadWrite = (uint32*)checkHead;
    uint32 * checkTailWrite = (uint32*)checkTail;
    *checkHeadWrite++ = CHECK_CODE_HEAD;
    *checkHeadWrite = CHECK_CODE_HEAD;
    *checkTailWrite++ = CHECK_CODE_TAIL;
    *checkTailWrite = CHECK_CODE_TAIL;
    
    uint8 * outmemBlock = (uint8*)block;
    outmemBlock += sizeof(MemoryBlock) + HALF_CHECK_SIZE;
    //Logger::FrameworkDebug("malloc: %p %p", block, outmemBlock);
    mutex.Unlock();
    return outmemBlock;
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
	if (ptr)
	{
        mutex.Lock();
        
        uint8 * uint8ptr = (uint8*)ptr; 
        uint8ptr -= (sizeof(MemoryBlock) + HALF_CHECK_SIZE);
        MemoryBlock * block = (MemoryBlock*)(uint8ptr);
        
        uint8 * checkHead = (uint8*)block + sizeof(MemoryBlock);
        uint8 * checkTail = (uint8*)block + sizeof(MemoryBlock) + block->size + CHECK_SIZE - HALF_CHECK_SIZE;
        uint32 * checkHeadRead = (uint32*)checkHead;
        uint32 * checkTailRead = (uint32*)checkTail;

        if ((checkHeadRead[0] != CHECK_CODE_HEAD) || 
            (checkHeadRead[1] != CHECK_CODE_HEAD) || 
            (checkTailRead[0] != CHECK_CODE_TAIL) || 
            (checkTailRead[1] != CHECK_CODE_TAIL))
        {
            Logger::Error("fatal: block: %p overwritten during usage", ptr);
        }
        //Logger::FrameworkDebug("free: %p %p", block, ptr);
        
        EraseMemoryBlock(block);
        free(block);
        
        mutex.Unlock();
	}
}
    
void MemoryManagerImpl::DumpLeaks(File * leaksLogFile, BacktraceTree::BacktraceTreeNode * node, int32 depth)
{
    uint32 nodeHeaderMarker = 0x00DE00DE;
    leaksLogFile->Write(&nodeHeaderMarker, 4);
    uint64 ptr = (uint64)node->pointer;
    leaksLogFile->Write(&ptr, 8);
    uint32 sizeOfChildren = node->SizeOfAllChildren();
    leaksLogFile->Write(&sizeOfChildren, 4);
    uint32 size =  node->children.size();
    leaksLogFile->Write(&size);
    
    if (size > 0)
    {
        for (uint32 k = 0; k < node->children.size(); ++k)
        {
            DumpLeaks(leaksLogFile, node->children[k], depth + 1);
        }
    }

}

void MemoryManagerImpl::DumpLog(uint32 dumpFlags)
{
    //mutex.Lock();
    DisableLeakTracking();
 
    
    //std::vector<char> buffer(50 * 1024 * 1024);
    std::ofstream memoryLog;
    //memoryLog.rdbuf()->pubsetbuf(&buffer.front(), buffer.size());
    
    FilePath documentsPath = FileSystem::GetUserDocumentsPath();
    String path = documentsPath.GetAbsolutePathname() + "memory.log";
    memoryLog.open(path.c_str());

    if (dumpFlags & DUMP_GLOBAL_STATS)
    {
        memoryLog << Format("    M E M O R Y     M A N A G E R     R E P O R T   ") << std::endl;
        memoryLog << Format("----------------------------------------------------") << std::endl;
        // 
        memoryLog << Format("* Currently Allocated Memory Size : %d", globalStats.currentAllocatedMemory) << std::endl;
        memoryLog << Format("* Real Allocated Memory Size : %d", globalStats.realAllocatedMemory) << std::endl;
        memoryLog << Format("* Peak Memory Usage : %d", globalStats.peakMemoryUsage) << std::endl;
        memoryLog << Format("* Max Allocated Block Size: %d", globalStats.maximumBlockSize) << std::endl;
        
        memoryLog << Format("* Peak Manager Overhead: %d", globalStats.peakManagerOverheadSize) << std::endl;
        memoryLog << Format("* Current Manager Overhead : %d", globalStats.managerOverheadSize) << std::endl;
        memoryLog << Format("* Overhead percentage: %0.3f %%", (float)globalStats.peakManagerOverheadSize / (float)globalStats.peakMemoryUsage * 100.0f) << std::endl;
        memoryLog << Format("* Total backtrace count: %d", backtraceSet.size()) << std::endl; 
    }
    
    if (dumpFlags & DUMP_POOL_STATS)
    {
        for (uint32 poolIndex = 0; poolIndex < MEMORY_POOL_COUNT; ++poolIndex)
        {
            memoryLog << Format(" P O O L # %d  ", poolIndex) << std::endl;
            memoryLog << Format("----------------------------------------------------") << std::endl;
            memoryLog << Format("* Currently Allocated Memory Size : %d", memoryPools[poolIndex].stats.currentAllocatedMemory) << std::endl;
            memoryLog << Format("* Real Allocated Memory Size : %d", memoryPools[poolIndex].stats.realAllocatedMemory) << std::endl;
            memoryLog << Format("* Peak Memory Usage : %d", memoryPools[poolIndex].stats.peakMemoryUsage) << std::endl;
            memoryLog << Format("* Max Allocated Block Size: %d", memoryPools[poolIndex].stats.maximumBlockSize) << std::endl;
            
            memoryLog << Format("* Peak Manager Overhead: %d", memoryPools[poolIndex].stats.peakManagerOverheadSize) << std::endl;
            memoryLog << Format("* Current Manager Overhead : %d", memoryPools[poolIndex].stats.managerOverheadSize) << std::endl;
            memoryLog << Format("* Overhead percentage: %0.3f %%", (float)memoryPools[poolIndex].stats.peakManagerOverheadSize / (float)memoryPools[poolIndex].stats.peakMemoryUsage * 100.0f) << std::endl;

            memoryLog << Format("* Leaks in PoolIndex: %d Count: %d", poolIndex, memoryPools[poolIndex].blockCount) << std::endl << std::endl;
        }
    }
    
    if (dumpFlags & DUMP_BASE_OBJECTS)
    {
        memoryLog << Format(" B A S E   O B J E C T S") << std::endl;
        memoryLog << Format("----------------------------------------------------") << std::endl;
        
        //MemoryPoolInfo & boPool = memoryPools[MEMORY_POOL_BASE_OBJECTS];
        Map<String, size_t> boMemoryMap;
        
        //int32 sizeofObj = 0;
        {
            MemoryBlock * headBlock = memoryPools[MEMORY_POOL_BASE_OBJECTS].headBlock;
            for (MemoryBlock * currentBlock = headBlock->next; currentBlock != headBlock; currentBlock = currentBlock->next)
            {
                if (currentBlock->flags & FLAG_LEAK_TRACKING_ENABLED)
                {
                    BaseObject * ptr = (BaseObject*)MEMORY_BLOCK_TO_PTR(currentBlock);
                    //BaseObject * baseObject = dynamic_cast<BaseObject*>(ptr);
                    if (ptr)
                    {
                        String name = ObjectFactory::Instance()->GetName(ptr);
                        boMemoryMap[name] += currentBlock->size;
                        //sizeofObj = currentBlock->size;
                    }
                }
            }
        }
        for (auto it = boMemoryMap.begin(); it != boMemoryMap.end(); ++it)
        {
            memoryLog << Format("- Type: %s Size: %d", it->first.c_str(), it->second) << std::endl;
        }
    }
    
//    if (dumpFlags & DUMP_LEAKS)
//    {
//        memoryLog << Format(" C U R R E N T   L E A K S") << std::endl;
//        memoryLog << Format("----------------------------------------------------") << std::endl;
//
//        uint32 index = 0;
//        for (uint32 poolIndex = 0; poolIndex < MEMORY_POOL_COUNT; ++poolIndex)
//        {
//            MemoryBlock * headBlock = memoryPools[poolIndex].headBlock;
//            for (MemoryBlock * currentBlock = headBlock->next; currentBlock != headBlock; currentBlock = currentBlock->next)
//            {
//                index++;
//                //MemoryBlock * block = currentBlock;
//                //CheckMemblockOverrun(block);
//                //if (block.filename)
//                if (currentBlock->flags & FLAG_LEAK_TRACKING_ENABLED)
//                {
//                    //Logger::FrameworkDebug("****** 	Memory Leak Found	******");
//                    memoryLog << "****** 	Memory Leak Found	******" << std::endl;
//                    //Logger::FrameworkDebug("* Source File : %s", block->filename);
//                    //Logger::FrameworkDebug("* Source Line : %d", block->line);
//                    //Logger::FrameworkDebug("* Allocation Size : %d bytes", currentBlock->size);
//                    memoryLog << Format("* Allocation Size : %d bytes", currentBlock->size) << std::endl;
//                    BacktraceLog log;
//                    CreateBacktraceLog(currentBlock->backtrace, &log);
//                    for (uint32 k = 0; k < currentBlock->backtrace->size; ++k) 
//                    {
//                        //Logger::FrameworkDebug("%s",log.strings[k]); 
//                        memoryLog << log.strings[k] << std::endl;
//                    }
//                    ReleaseBacktraceLog(&log);
//                    //Logger::FrameworkDebug("* Was placed : " << (block.isPlaced ? " true" : " false")<< std::endl;
//                    //Logger::FrameworkDebug("************************************");
//                }
//            }
//        }
//    }
    
    if (dumpFlags & DUMP_LEAKS)
    {
        BacktraceTree backtraceTree;
        {
            ImmediateTimeMeasure immTimeMeasure(FastName("Build Backtrace Tree"));
            
            uint32 validateSize = 0;
            for (uint32 poolIndex = 0; poolIndex < MEMORY_POOL_COUNT; ++poolIndex)
            {
                MemoryBlock * headBlock = memoryPools[poolIndex].headBlock;
                for (MemoryBlock * currentBlock = headBlock->next; currentBlock != headBlock; currentBlock = currentBlock->next)
                {
                    if (currentBlock->flags & FLAG_LEAK_TRACKING_ENABLED)
                    {
                        backtraceTree.Insert(currentBlock->backtrace, currentBlock->size);
                        validateSize += currentBlock->size;
                    }
                }
            }
            DVASSERT(validateSize == backtraceTree.head->SizeOfAllChildren());
        }
        {
            ImmediateTimeMeasure immTimeMeasure(FastName("Generate Symbols"));
            backtraceTree.GenerateSymbols();
        }

        FilePath documentsPath = FileSystem::GetUserDocumentsPath();
        String path = documentsPath.GetAbsolutePathname() + "leaks.log";
        File * leaksLogFile = File::PureCreate(FilePath(path.c_str()), File::CREATE | File::WRITE);
        if (leaksLogFile)
        {
            ImmediateTimeMeasure immTimeMeasure(FastName("Prepare to Write Backtrace Tree"));

            uint32 size = (uint64)backtraceTree.symbols.size();
            leaksLogFile->Write(&size, 4);
            for (auto it = backtraceTree.symbols.begin(); it != backtraceTree.symbols.end();++it)
            {
                uint64 ptr = (uint64)it->first;
                leaksLogFile->Write(&ptr, 8);
                leaksLogFile->WriteString(String(it->second));
            }
            
            BacktraceTree::BacktraceTreeNode * currentItem = backtraceTree.head;

            DumpLeaks(leaksLogFile, currentItem, 1);
        }
        SafeRelease(leaksLogFile);
    }
    
    memoryLog.close();
    
    EnableLeakTracking();
    //mutex.Unlock();
}

};

#endif // MEMORY_MANAGER

