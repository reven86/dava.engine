#ifndef __DAVAENGINE_NATIVE_VIEW_POOL_H__
#define __DAVAENGINE_NATIVE_VIEW_POOL_H__


#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"

#import <UIKit/UIKit.h>

namespace DAVA
{
template <class VIEW_TYPE>
class NativeViewPool
{
public:
    NativeViewPool();
    ~NativeViewPool();

    VIEW_TYPE* GetOrCreateView();

    void ReleaseView(const VIEW_TYPE* view);

private:
    uint32 usedViewsCount;
    Vector<VIEW_TYPE*> allocatedViews;

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
    for (uint32 i = 0; i < count; ++i)
    {
        VIEW_TYPE* view = allocatedViews[i];
        [view release];
    }

    allocatedViews.clear();
    usedViewsCount = 0;
}

template <class VIEW_TYPE>
void NativeViewPool<VIEW_TYPE>::ReleaseView(const VIEW_TYPE* view)
{
    DVASSERT(usedViewsCount != 0);

    LockGuard<Mutex> locker(mutex);

    const uint32 count = (uint32)allocatedViews.size();
    const uint32 lastViewIndex = usedViewsCount - 1;
    for (uint32 releasedViewIndex = 0; releasedViewIndex < count; ++releasedViewIndex)
    {
        VIEW_TYPE* v = allocatedViews[releasedViewIndex];

        if (view == v)
        {
            if (releasedViewIndex < lastViewIndex)
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
VIEW_TYPE* NativeViewPool<VIEW_TYPE>::GetOrCreateView()
{
    LockGuard<Mutex> locker(mutex);

    VIEW_TYPE* view = nil;
    if (usedViewsCount < allocatedViews.size())
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
