//
//  STLAllocatorFactory.h
//  Framework
//
//  Created by Sergey Bogdanov on 3/11/14.
//
//

#ifndef __Framework__STLAllocatorFactory__
#define __Framework__STLAllocatorFactory__


#include <istream>
#include <map>
#include "Base/Singleton.h"
#include "Base/FixedSizePoolAllocator.h"



namespace DAVA
{
    
    class STLAllocatorFactory : public Singleton<STLAllocatorFactory>
    {
    public:
        STLAllocatorFactory();
        virtual ~STLAllocatorFactory();
        
        FixedSizePoolAllocator * GetAllocator(unsigned int classSize);
        
        void Dump();
        
    private:
        std::map<unsigned int,FixedSizePoolAllocator*> allocators;
        FixedSizePoolAllocator * LastAlloc;
    };
    
    
};

#endif /* defined(__Framework__STLAllocatorFactory__) */
