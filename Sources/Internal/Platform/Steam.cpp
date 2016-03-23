/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Steam.h"

#if defined(__DAVAENGINE_STEAM__)

#include "Logger/Logger.h"

using namespace STEAM_SDK;

namespace DAVA
{
bool Steam::isInited = false;

void Steam::Init()
{
    if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid))
    {
        // if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the
        // local Steam client and also launches this game again.

        // Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
        // removed steam_appid.txt from the game depot.
        Logger::Error("Error SteamAPI Restart.");
        return;
    }

    // Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
    // You don't necessarily have to though if you write your code to check whether all the Steam
    // interfaces are NULL before using them and provide alternate paths when they are unavailable.
    //
    // This will also load the in-game steam overlay dll into your process.  That dll is normally
    // injected by steam when it launches games, but by calling this you cause it to always load,
    // even when not launched via steam.
    if (!SteamAPI_Init())
    {
        Logger::Error("SteamAPI_Init() failed\n Fatal Error: Steam must be running to play this game(SteamAPI_Init() failed).");
        return;
    }

    if (!SteamController()->Init())
    {
        Logger::Error("SteamController()->Init failed.\n Fatal Error: SteamController()->Init failed.");
        return;
    }

    isInited = true;
}

void Steam::Deinit()
{
    // Shutdown the SteamAPI
    SteamAPI_Shutdown();
    isInited = false;
}

bool Steam::IsInited()
{
    return isInited;
}

Steam::Storage* Steam::CreateStorage()
{
    return SteamRemoteStorage();
}
}

#endif