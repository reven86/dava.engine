#ifndef __DAVAENGINE_DEADLINETIMERBASE_H__
#define __DAVAENGINE_DEADLINETIMERBASE_H__

#include <libuv/uv.h>

#include "Detail/Noncopyable.h"

namespace DAVA {

class IOLoop;

/*
 Class DeadlineTimerBase - base class for timers
*/
class DeadlineTimerBase : private Noncopyable
{
public:
    explicit DeadlineTimerBase (IOLoop* ioLoop);

    IOLoop* Loop () { return loop; }

          uv_timer_t* Handle ()       { return &handle; }
    const uv_timer_t* Handle () const { return &handle; }

          uv_handle_t* HandleAsHandle ()       { return reinterpret_cast<uv_handle_t*> (&handle); }
    const uv_handle_t* HandleAsHandle () const { return reinterpret_cast<const uv_handle_t*> (&handle); }

    bool IsClosed () const;

protected:
    void InternalClose (uv_close_cb callback);

    // Protected destructor to prevent deletion through this type
    ~DeadlineTimerBase () {}

protected:
    IOLoop*    loop;
    uv_timer_t handle;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_DEADLINETIMERBASE_H__
