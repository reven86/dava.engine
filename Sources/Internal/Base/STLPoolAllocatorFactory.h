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
#include "Base/Singleton.h"
#include "Base/Pool.h"



namespace DAVA
{
    
    class STLPoolAllocatorFactory : public Singleton<STLPoolAllocatorFactory>
    {
    public:
        STLPoolAllocatorFactory();
        virtual ~STLPoolAllocatorFactory();
        
        void * Allocate(unsigned int classSize ,unsigned int countObject);
        void Deallocate(void *p);
        void Dump();
        
    private:
        std::vector<Pool*> allocators;
        Pool * LastAlloc;
    };
    
    
};

#endif /* defined(__Framework__STLPoolAllocatorFactory__) */
