#ifndef STEAM_H
#define STEAM_H

#if defined(__DAVAENGINE_STEAM__)

class ISteamRemoteStorage;

namespace DAVA
{
class Steam final
{
public:
    Steam() = default;

    static void Init();
    static void Deinit();
    static bool IsInited();

    static ISteamRemoteStorage* CreateStorage();

private:
    static bool isInited;
};
}
#endif
#endif