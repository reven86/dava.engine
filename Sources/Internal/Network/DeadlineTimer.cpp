#include "DeadlineTimer.h"

namespace DAVA {

DeadlineTimer::DeadlineTimer (IOLoop* loop) : BaseClassType (loop) {}

void DeadlineTimer::HandleTimer ()
{
    waitHandler (this);
}

}   // namespace DAVA
