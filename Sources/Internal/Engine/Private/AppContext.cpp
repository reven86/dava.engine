#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/AppContext.h"

namespace DAVA
{
AppContext::AppContext() = default;

AppContext::~AppContext()
{
    SafeDelete(jobManager);
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
