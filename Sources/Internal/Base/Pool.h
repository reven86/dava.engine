//
//  pool.h
//  Framework
//
//  Created by Sergey Bogdanov on 3/17/14.
//
//

#ifndef Framework_pool_h
#define Framework_pool_h


#ifdef _WIN32

#pragma warning(push)

#pragma warning(disable:4018) //signed/unsigned mismatch

#pragma warning(disable:4290) // exception spec ignored

#endif


#include <exception>
#include <list>
#include <algorithm>
#include <iostream>  //for dump()

class Pool
{

public:
    Pool(size_t size = 0):poolSize(size),growCount(1)
    {
 //       void * p = ::malloc(size_);
//        memset(p, 0, size_);
        pool_mem_.push_back(new char[poolSize]);
//        pool_mem_.push_back(reinterpret_cast<char*>(p));
        Blocks = reinterpret_cast<Block*>(*(pool_mem_.begin()));
        Blocks->prev= 0;
        Blocks->next= 0;
        Blocks->free= 1;
        Blocks->blockSize = poolSize-sizeof(Block);
        std::cout<<std::endl<<"New pool"<<size;
    };
    ~Pool()
    
    {
        
        std::for_each(pool_mem_.begin(), pool_mem_.end(), killer());
        
    }
    
    
    void* allocate(size_t size)
    {
        if(size>poolSize- sizeof(Block)) {
            std::cout<<std::endl<<"size="<<size;
           /* size_=size+sizeof(Block);
            Blocks->free=0;
            Blocks->blockSize = size_;*/
            throw std::bad_alloc();
        }
        Block *b = Blocks;

        while ((b->free==0)||(b->blockSize<size)) {
            if(b->next==NULL) grow(b);
                b=b->next;

        }
        if(b->blockSize - size < 2*sizeof(Block)){
            b->free=0;
            return reinterpret_cast<char *>(b) + sizeof(Block);
        }
        else{
            int offset =size + sizeof(Block);
            Block * newBlock = (reinterpret_cast<Block *>(reinterpret_cast<char *>(b) + offset));
            if(b->next!=NULL) b->next->prev = newBlock;
            newBlock->next = b->next;
            b->next = newBlock;
            newBlock->prev = b;
            b->free = 0;
            //int dbg =sizeof(Block);
            newBlock->blockSize = b->blockSize - size - sizeof(Block);
            
            b->blockSize = size;
            newBlock->free = 1;
            return reinterpret_cast<char *>(b) + sizeof(Block);
        }
        
    }
    
    void static deallocate(void *p, size_t = 0)
    {
        if(!p) return;
        Block *b = reinterpret_cast<Block *>(static_cast<char*>(p) - sizeof(Block));
        if(b->prev && b->next){
            if(b->prev->free && b->next->free){
                b->prev->blockSize += b->blockSize + b->next->blockSize + 2*sizeof(Block);
                b->prev->next = b->next->next;
                if(b->next->next)b->next->next->prev = b->prev;
                return;
            }
        }
        if(b->prev){
            if(b->prev->free){
                b->prev->blockSize += b->blockSize + sizeof(Block);
                b->prev->next=b->next;
                if(b->next) b->next->prev = b->prev;
                b->free= 1;
                return;
            }
        }
        if(b->next){
            if(b->next->free){
                b->blockSize += b->next->blockSize + sizeof(Block);
                b->next = b->next->next;
                if(b->next) b->next->prev = b;
                b->free= 1;
                return;
            }
        }
        b->free = 1;
    }
    void dump()
    {
        using namespace std;
        Block *b = Blocks;
        while(1){
            cout<<std::endl<<"size block="<<b->blockSize<<", free="<<b->free<<
            
            ", prev="<<b->prev<<", next="<<b->next<<endl;
            if(b->next) b = b->next;
            else break;
        }
    }
    unsigned int getSize()
    {
        return poolSize;
    }
    struct Block{
        Block *prev;
        Block *next;
        size_t blockSize;
        int free;
        Block(Block *prev, Block *next, size_t size, int free):
        
        prev(prev), next(next), blockSize(size), free(free){}
        ~Block(){}
    };
//private:
    size_t poolSize;
    std::list<char *> pool_mem_;
    unsigned int growCount;
    

    Block * Blocks;
    Pool(const Pool &);
    Pool& operator=(const Pool&);
    
    struct killer
    
    {
        void operator()(char *p){delete [] p;}
        
    };
    
    static void kill(char *p){delete [] p;}
    void grow(Block *b)
    {
        growCount++;
        Block *newBlock;
        char *p = new char[poolSize];
        pool_mem_.push_back(p);
        newBlock = reinterpret_cast<Block*>(p);
        newBlock->prev= b;
        newBlock->next= 0;
        newBlock->free= 1;
        newBlock->blockSize= poolSize-sizeof(Block);
        b->next = newBlock;
    }
    
};

#ifdef _WIN32

#pragma warning(pop)

#endif





#endif
