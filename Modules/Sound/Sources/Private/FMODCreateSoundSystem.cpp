#include "FMODSoundSystem.h"

namespace DAVA
{
SoundSystem* CreateSoundSystem(Engine* e)
{
    static SoundSystem* instSoundSystem = nullptr;
    if (nullptr == instSoundSystem)
    {
        instSoundSystem = new FMODSoundSystem(e);
    }
    return instSoundSystem;
}
}
