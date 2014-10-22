#ifndef __DAVAENGINE_DEADLINETIMERTEMPLATE_H__
#define __DAVAENGINE_DEADLINETIMERTEMPLATE_H__

#include <libuv/uv.h>

#include "DeadlineTimerBase.h"

namespace DAVA {

class IOLoop;

/*
 Template class DeadlineTimerTemplate provides basic waitable timer functionality
 Template parameter T specifies type that inherits DeadlineTimerTemplate (CRTP idiom)
 Bool template parameter autoRepeat specifies timer behaviour:
    when autoRepeat is true libuv automatically issues next wait operations until AsyncStop is called
    when autoRepeat is false user should explicitly issue next wait operation

 Type specified by T should implement methods:
    void HandleTimer ()
        This method is called when timeout has been expired
    void HandleClose ()
        This method is called after underlying socket has been closed by libuv

 Summary of methods that should be implemented by T:
    void HandleTimer ();
    void HandleClose ();
*/
template<typename T, bool autoRepeat = false>
class DeadlineTimerTemplate : public DeadlineTimerBase
{
public:
    typedef DeadlineTimerBase                    BaseClassType;
    typedef DeadlineTimerTemplate<T, autoRepeat> ThisClassType;
    typedef T                                    DerivedClassType;

    static const bool autoRepeatFlag = autoRepeat;

public:
    DeadlineTimerTemplate (IOLoop* loop) : BaseClassType (loop)
    {
        handle.data = static_cast<DerivedClassType*> (this);
    }

    ~DeadlineTimerTemplate () {}

    void Close ()
    {
        BaseClassType::InternalClose (&HandleCloseThunk);
    }

    void AsyncStopWait ()
    {
        uv_timer_stop (Handle ());
    }

protected:
    void InternalAsyncStartWait (unsigned int timeout, unsigned int repeat)
    {
        uv_timer_start (Handle (), &HandleTimerThunk, timeout, repeat);
    }

private:
    void HandleClose () {}

    void HandleTimer () {}

    static void HandleCloseThunk (uv_handle_t* handle)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleClose ();
    }

    static void HandleTimerThunk (uv_timer_t* handle)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleTimer ();
        if (!autoRepeatFlag)
            pthis->AsyncStop ();
    }
};

}   // namespace DAVA

#endif  // __DAVAENGINE_DEADLINETIMERTEMPLATE_H__
