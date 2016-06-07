#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class JobManager;
class InputSystem;
class UIControlSystem;
class VirtualCoordinatesSystem;

class AppContext
{
public:
    AppContext();
    ~AppContext();

    JobManager* jobManager = nullptr;
    InputSystem* inputSystem = nullptr;
    UIControlSystem* uiControlSystem = nullptr;
    VirtualCoordinatesSystem* virtualCoordSystem = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
