#ifndef __DAVAENGINE_DEADLINETIMER_H__
#define __DAVAENGINE_DEADLINETIMER_H__

#include <libuv/uv.h>

#include <Base/Function.h>
#include <Debug/DVAssert.h>

#include "DeadlineTimerTemplate.h"

namespace DAVA {

class IOLoop;

/*
 Class DeadlineTimer - fully functional waitable timer implementation which can be used in most cases.
 User can provide functional object which is called on timer expiration.
 Functional objects prototypes:
    WaitHandlerType - called on timer expiration
        void (DeadlineTimer* timer)
*/
class DeadlineTimer : public DeadlineTimerTemplate<DeadlineTimer, false>
{
public:
    typedef DeadlineTimerTemplate<DeadlineTimer, false> BaseClassType;
    typedef DeadlineTimer                               ThisClassType;

    typedef DAVA::Function<void (ThisClassType* timer)> WaitHandlerType;

public:
    explicit DeadlineTimer (IOLoop* loop);

    ~DeadlineTimer () {}

    template <typename Handler>
    void AsyncStartWait (unsigned int timeout, unsigned int repeat, Handler handler)
    {
        DVASSERT (!(handler == 0));

        waitHandler = handler;
        BaseClassType::InternalAsyncStartWait (timeout, repeat);
    }

    void HandleTimer ();

private:
    WaitHandlerType waitHandler;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_DEADLINETIMER_H__
