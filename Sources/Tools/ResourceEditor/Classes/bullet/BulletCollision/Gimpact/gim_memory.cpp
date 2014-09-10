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


/*
-----------------------------------------------------------------------------
This source file is part of GIMPACT Library.

For the latest info, see http://gimpact.sourceforge.net/

Copyright (c) 2006 Francisco Leon Najera. C.C. 80087371.
email: projectileman@yahoo.com

 This library is free software; you can redistribute it and/or
 modify it under the terms of EITHER:
   (1) The GNU Lesser General Public License as published by the Free
       Software Foundation; either version 2.1 of the License, or (at
       your option) any later version. The text of the GNU Lesser
       General Public License is included with this library in the
       file GIMPACT-LICENSE-LGPL.TXT.
   (2) The BSD-style license that is included with this library in
       the file GIMPACT-LICENSE-BSD.TXT.
   (3) The zlib/libpng license that is included with this library in
       the file GIMPACT-LICENSE-ZLIB.TXT.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 GIMPACT-LICENSE-LGPL.TXT, GIMPACT-LICENSE-ZLIB.TXT and GIMPACT-LICENSE-BSD.TXT for more details.

-----------------------------------------------------------------------------
*/


#include "gim_memory.h"
#include "stdlib.h"

#ifdef GIM_SIMD_MEMORY
#include "LinearMath/btAlignedAllocator.h"
#endif

static gim_alloc_function *g_allocfn = 0;
static gim_alloca_function *g_allocafn = 0;
static gim_realloc_function *g_reallocfn = 0;
static gim_free_function *g_freefn = 0;

void gim_set_alloc_handler (gim_alloc_function *fn)
{
  g_allocfn = fn;
}

void gim_set_alloca_handler (gim_alloca_function *fn)
{
  g_allocafn = fn;
}

void gim_set_realloc_handler (gim_realloc_function *fn)
{
  g_reallocfn = fn;
}

void gim_set_free_handler (gim_free_function *fn)
{
  g_freefn = fn;
}

gim_alloc_function *gim_get_alloc_handler()
{
  return g_allocfn;
}

gim_alloca_function *gim_get_alloca_handler()
{
  return g_allocafn;
}


gim_realloc_function *gim_get_realloc_handler ()
{
  return g_reallocfn;
}


gim_free_function  *gim_get_free_handler ()
{
  return g_freefn;
}


void * gim_alloc(size_t size)
{
	void * ptr;
	if (g_allocfn)
	{
		ptr = g_allocfn(size);
	}
	else
	{
#ifdef GIM_SIMD_MEMORY
		ptr = btAlignedAlloc(size,16);
#else
		ptr = malloc(size);
#endif
	}
  	return ptr;
}

void * gim_alloca(size_t size)
{
  if (g_allocafn) return g_allocafn(size); else return gim_alloc(size);
}


void * gim_realloc(void *ptr, size_t oldsize, size_t newsize)
{
 	void * newptr = gim_alloc(newsize);
    size_t copysize = oldsize<newsize?oldsize:newsize;
    gim_simd_memcpy(newptr,ptr,copysize);
    gim_free(ptr);
    return newptr;
}

void gim_free(void *ptr)
{
	if (!ptr) return;
	if (g_freefn)
	{
	   g_freefn(ptr);
	}
	else
	{
	#ifdef GIM_SIMD_MEMORY
		btAlignedFree(ptr);
	#else
		free(ptr);
	#endif
	}
}

