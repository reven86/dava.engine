//
//  STLPoolAllocatorFactory.h
//  Framework
//
//  Created by Sergey Bogdanov on 3/11/14.
//
//

#ifndef __Framework__STLPoolAllocatorFactory__
#define __Framework__STLPoolAllocatorFactory__


#include <istream>
#include <vector>
#include <map>
#include "Base/StaticSingleton.h"
#include "Base/Pool.h"


std::map< pthread_t , std::vector<Pool*> > allocatorThread;
std::vector<Pool*> baseAllocator;

namespace DAVA
{
    
    class STLPoolAllocatorFactory : public StaticSingleton<STLPoolAllocatorFactory>
    {
    public:
        STLPoolAllocatorFactory();
        virtual ~STLPoolAllocatorFactory();
        
        void * Allocate(unsigned int classSize ,unsigned int countObject, bool inBase);
        void Deallocate(void *p, bool inBase);
        void Dump();
        //std::map< pthread_t , std::vector<Pool*> > allocatorThread;
    private:
        //std::vector<Pool*> allocators;
        
        Pool * LastAlloc;
    };
    
    
};

#endif /* defined(__Framework__STLPoolAllocatorFactory__) */
