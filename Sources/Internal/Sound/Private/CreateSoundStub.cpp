#include "Sound/SoundSystem.h"

namespace DAVA
{

#if defined(__DAVAENGINE_COREV2__)

SoundSystem* CreateSoundSystem(Engine* e)
{
    static SoundSystem* instSoundSystem = nullptr;
    if (nullptr == instSoundSystem)
    {
        instSoundSystem = new SoundSystem(e);
    }
    return instSoundSystem;
}

#else

SoundSystem* CreateSoundSystem()
{
    static SoundSystem* instSoundSystem = nullptr;
    if (nullptr == instSoundSystem)
    {
        instSoundSystem = new SoundSystem();
    }
    return instSoundSystem;
}

#endif
}