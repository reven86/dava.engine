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


#ifndef __DAVAENGINE_NATIVE_VIEW_POOL_H__
#define __DAVAENGINE_NATIVE_VIEW_POOL_H__


#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"

#import <UIKit/UIKit.h>

namespace DAVA
{

template<class VIEW_TYPE>
class NativeViewPool
{
public:
    
    NativeViewPool();
    ~NativeViewPool();
    
    VIEW_TYPE * GetOrCreateView();
    
    void ReleaseView(const VIEW_TYPE *view);
    
private:
    
    uint32 usedViewsCount;
    Vector<VIEW_TYPE *> allocatedViews;
    
    Mutex mutex;
};
    

template <class VIEW_TYPE>
NativeViewPool<VIEW_TYPE>::NativeViewPool()
: usedViewsCount(0)
{
    
}

    
template <class VIEW_TYPE>
NativeViewPool<VIEW_TYPE>::~NativeViewPool()
{
    LockGuard<Mutex> locker(mutex);
    
    const uint32 count = (uint32)allocatedViews.size();
    for(uint32 i = 0; i < count; ++i)
    {
        VIEW_TYPE *view = allocatedViews[i];
        [view release];
    }
    
    allocatedViews.clear();
    usedViewsCount = 0;
}

    
template <class VIEW_TYPE>
void NativeViewPool<VIEW_TYPE>::ReleaseView(const VIEW_TYPE *view)
{
    DVASSERT(usedViewsCount != 0);
    
    LockGuard<Mutex> locker(mutex);
    
    const uint32 count = (uint32)allocatedViews.size();
    const uint32 lastViewIndex = usedViewsCount - 1;
    for(uint32 releasedViewIndex = 0; releasedViewIndex < count; ++releasedViewIndex)
    {
        VIEW_TYPE *v = allocatedViews[releasedViewIndex];
        
        if(view == v)
        {
            if(releasedViewIndex < lastViewIndex)
            {
                allocatedViews[releasedViewIndex] = allocatedViews[lastViewIndex];
                allocatedViews[lastViewIndex] = v;
            }
            
            --usedViewsCount;
            
            break;
        }
    }
}
    

template <class VIEW_TYPE>
VIEW_TYPE * NativeViewPool<VIEW_TYPE>::GetOrCreateView()
{
    LockGuard<Mutex> locker(mutex);
    
    VIEW_TYPE *view = nil;
    if(usedViewsCount < allocatedViews.size())
    {
        view = allocatedViews[usedViewsCount];
    }
    else
    {
        view = [[VIEW_TYPE alloc] init];
        allocatedViews.push_back(view);
    }

    ++usedViewsCount;
    return view;
}
    
    
};
#endif // #if defined(__DAVAENGINE_IPHONE__)

#endif //__DAVAENGINE_NATIVE_VIEW_POOL_H__


 