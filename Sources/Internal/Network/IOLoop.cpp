#include <cstring>

#include <Debug/DVAssert.h>

#include "IOLoop.h"

namespace DAVA {

IOLoop::IOLoop (bool useDefaultIOLoop) : actualLoop (NULL), useDefaultLoop (useDefaultIOLoop)
{
    memset (&loop, 0, sizeof (loop));
    if (!useDefaultIOLoop)
    {
        uv_loop_init (&loop);
        actualLoop = &loop;
    }
    else
        actualLoop = uv_default_loop ();
    actualLoop->data = this;
}

IOLoop::~IOLoop ()
{
    if (!useDefaultLoop)
        DVVERIFY (0 == uv_loop_close (actualLoop));
}

int IOLoop::Run (eRunMode runMode)
{
    uv_run_mode mode = runMode == RUN_DEFAULT ? UV_RUN_DEFAULT :
                      (runMode == RUN_ONCE    ? UV_RUN_ONCE    :
                      (runMode == RUN_NOWAIT  ? UV_RUN_NOWAIT  : UV_RUN_DEFAULT));
    return uv_run (actualLoop, mode);
}

void IOLoop::Stop ()
{
    uv_stop (actualLoop);
}

bool IOLoop::IsAlive () const
{
    return uv_loop_alive (actualLoop) != 0;
}

}	// namespace DAVA
