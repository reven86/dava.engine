#include <Debug/DVAssert.h>

#include "IOLoop.h"
#include "DeadlineTimerBase.h"

namespace DAVA {

DeadlineTimerBase::DeadlineTimerBase (IOLoop* ioLoop) : loop (ioLoop), handle ()
{
    DVASSERT (ioLoop);

    uv_timer_init (loop->Handle (), &handle);
}

bool DeadlineTimerBase::IsClosed () const
{
    return uv_is_closing (HandleAsHandle ()) ? true : false;
}

void DeadlineTimerBase::InternalClose (uv_close_cb callback)
{
    if (!IsClosed ())
        uv_close (HandleAsHandle (), callback);
}

}   // namespace DAVA
