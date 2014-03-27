//
//  STLPoolAllocatorFactory.cpp
//  Framework
//
//  Created by Sergey Bogdanov on 3/24/14.
//
//

#include "STLPoolAllocatorFactory.h"


namespace DAVA
{
    
    STLPoolAllocatorFactory::STLPoolAllocatorFactory():LastAlloc(NULL)
    {
        
    }
    
    STLPoolAllocatorFactory::~STLPoolAllocatorFactory()
    {
#ifdef __DAVAENGINE_DEBUG__
        Dump();
#endif
        std::vector<Pool*>::iterator itEnd = allocators.end();
        for (std::vector<Pool*>::iterator it = allocators.begin(); it != itEnd; ++it)
        {
            delete (*it);
        }
        allocators.clear();
    }
    
    void STLPoolAllocatorFactory::Dump()
    {
        //#ifdef __DAVAENGINE_DEBUG__
        std::cout<<std::endl<<"STLPoolAllocatorFactory::Dump ================"<<std::endl<<"Allocated:";
        std::vector<Pool*>::iterator itEnd = allocators.end();
        unsigned int allover = 0;
        for(std::vector<Pool*>::iterator it = allocators.begin(); it != itEnd; ++it)
        {
            std::cout<<std::endl<<(*it)->getSize()<<" * "<<(*it)->growCount<<" = "<<(*it)->getSize()*(*it)->growCount;
            (*it)->dump();
        }
        std::cout<<std::endl<<"End of STLPoolAllocatorFactory::Dump ========================== allover= %ubyte"<<allover;
        //#endif //__DAVAENGINE_DEBUG__
    }
    
    void * STLPoolAllocatorFactory::Allocate(unsigned int classSize,unsigned int countObject)
    {
     /*   if (LastAlloc)
        {
           // if (LastAlloc->blockSize == classSize)
            {
                return LastAlloc;
            }
        }
        unsigned int initSize=1024*1024;
        Pool * alloc = allocators[classSize];
        if ( 0 == alloc )
        {
           // alloc = new FixedSizePoolAllocator(classSize, 1);
            
            if (classSize*countObject > initSize) {
                initSize = classSize*countObject*2;
            }
            alloc = new Pool(initSize);
            allocators[classSize] = alloc;
            //LastAlloc = alloc;
        } else
        {
            if (alloc->getSize()<classSize*countObject)
            {
                alloc = new Pool(classSize*countObject*2);
                allocators[classSize] = alloc;
            }
        }*/
        Pool * alloc = NULL;
        unsigned int size =classSize*countObject + sizeof(Pool::Block);
        for (unsigned int i=0;i<allocators.size();i++)
        {
            if (allocators[i]->getSize()>size) {
                alloc=allocators[i];
                break;
            }
        }
        if (0 == alloc) {
            unsigned int minSize = 4096*300;/* 1024*1024*/;
            alloc = new Pool(size<minSize?minSize:size);
            allocators.push_back(alloc);
        }
        unsigned int allocateSize=classSize*countObject;
        if(allocateSize%16){
            allocateSize+=(16-allocateSize%16);
        }
//        void * p = alloc->allocate(allocateSize);
        //std::cout<<"stl alloc pointer"<<p;
        return alloc->allocate(allocateSize);;
    }
    void STLPoolAllocatorFactory::Deallocate(void *p)
    {
        //Pool * alloc = NULL;
        //Pool::deallocate(p);
        for (unsigned int i=0;i<allocators.size();i++)
        {
            if ((p>allocators[i]->Blocks)&&(p<reinterpret_cast<void *>(reinterpret_cast<char*>(allocators[i]->Blocks) + allocators[i]->poolSize)))
            {
                //alloc=allocators[i];
                allocators[i]->deallocate(p);
                return;
            }
        }
        std::cout<<"\n Not Deallocate Pool\n";
    }
    
}