#ifndef STEAM_H
#define STEAM_H

#if defined(__DAVAENGINE_STEAM__)

// wrapped for UNITY BUILD
namespace STEAM_SDK
{
#include "steam/steam_api.h"
}

namespace DAVA
{
class Steam final
{
public:
    using Storage = STEAM_SDK::ISteamRemoteStorage;

    Steam() = default;

    static void Init();
    static void Deinit();
    static bool IsInited();

    static Storage* CreateStorage();

private:
    static bool isInited;
};
}

#endif

#endif