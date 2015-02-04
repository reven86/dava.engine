//
//  malloc.h
//  Framework
//
//  Created by Leonid Sinyakov on 2/4/15.
//
//

#ifndef Framework_malloc_h
#define Framework_malloc_h
//functions to use inside MemoryManager related code ONLY!
extern void* mem_malloc(size_t t);
extern void mem_free(void*p);

#endif
