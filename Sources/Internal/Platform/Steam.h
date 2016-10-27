#ifndef STEAM_H
#define STEAM_H

#if defined(__DAVAENGINE_STEAM__)
#include "Base/BaseTypes.h"

class ISteamRemoteStorage;
namespace DAVA
{
class Steam final
{
public:
    static const String appIdPropertyKey;

    Steam() = default;

    static void Init();
    static void Deinit();
    static bool IsInited();
    static void Update();

    // Get code of language, ex "ru" for "russian"
    static String GetSteamLanguage();

    static ISteamRemoteStorage* CreateStorage();

private:
    static bool isInited;
};
}
#endif
#endif