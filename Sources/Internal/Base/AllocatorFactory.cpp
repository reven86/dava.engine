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


#include "Base/AllocatorFactory.h"

namespace DAVA
{

AllocatorFactory::AllocatorFactory()
{

}

AllocatorFactory::~AllocatorFactory()
{
#ifdef __DAVAENGINE_DEBUG__
	Dump();
#endif 

	Map<String, FixedSizePoolAllocator*>::iterator itEnd = allocators.end();
	for(Map<String, FixedSizePoolAllocator*>::iterator it = allocators.begin(); it != itEnd; ++it)
	{
		delete((*it).second);
	}
	allocators.clear();
}

void AllocatorFactory::Dump()
{
#ifdef __DAVAENGINE_DEBUG__
	Logger::FrameworkDebug("AllocatorFactory::Dump (Max item count) ================");
	Map<String, FixedSizePoolAllocator*>::iterator itEnd = allocators.end();
	for(Map<String, FixedSizePoolAllocator*>::iterator it = allocators.begin(); it != itEnd; ++it)
	{
		FixedSizePoolAllocator * alloc = (*it).second;
		Logger::FrameworkDebug("  %s: %u", it->first.c_str(), alloc->maxItemCount);
	}

	Logger::FrameworkDebug("End of AllocatorFactory::Dump ==========================");
#endif //__DAVAENGINE_DEBUG__
}

FixedSizePoolAllocator * AllocatorFactory::GetAllocator(const DAVA::String& className, DAVA::uint32 classSize, int32 poolLength)
{
    FixedSizePoolAllocator * alloc = allocators[className];
    if(0 == alloc)
    {
        alloc = new FixedSizePoolAllocator(classSize, poolLength);
        allocators[className] = alloc;
    }

    return alloc;
}

}