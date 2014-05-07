#include "Base/Atomic.h"

#if defined(__DAVAENGINE_HTML5__)

namespace DAVA 
{

int32 AtomicIncrement( int32 &value )
{
    return ++value;
}

int32 AtomicDecrement( int32 &value )
{
    return --value;
}

};

#endif //#if defined(__DAVAENGINE_HTML5__)

