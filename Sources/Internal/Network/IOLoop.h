#ifndef __DAVAENGINE_IOLOOP_H__
#define __DAVAENGINE_IOLOOP_H__

#include <libuv/uv.h>

#include "Detail/Noncopyable.h"

namespace DAVA {

/*
 Class IOLoop is a thin wrapper on libuv's uv_loop_t object.
 IOLoop's responsibility is to gather event from operating system and call corresponding user callbacks.
 Run is a core method of IOLoop, must be called from at most one thread.
*/
class IOLoop : private Noncopyable
{
public:
    enum eRunMode
    {
        RUN_DEFAULT = UV_RUN_DEFAULT,
        RUN_ONCE    = UV_RUN_ONCE,
        RUN_NOWAIT  = UV_RUN_NOWAIT
    };

public:
    IOLoop (bool useDefaultIOLoop = true);

    ~IOLoop ();

    uv_loop_t* Handle () { return actualLoop; }

    int Run (eRunMode runMode = RUN_DEFAULT);

    void Stop ();

	bool IsAlive () const;

private:
    uv_loop_t  loop;
    uv_loop_t* actualLoop;
    bool       useDefaultLoop;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_IOLOOP_H__
