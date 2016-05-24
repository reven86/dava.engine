#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class JobManager;

class AppContext
{
public:
    AppContext();
    ~AppContext();

    JobManager* jobManager = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
