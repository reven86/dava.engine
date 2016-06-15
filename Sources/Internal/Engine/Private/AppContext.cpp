#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/AppContext.h"
#include "Job/JobManager.h"

namespace DAVA
{
AppContext::AppContext() = default;

AppContext::~AppContext()
{
    SafeDelete(jobManager);
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
