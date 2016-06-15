#include "FileSystem/FileSystem.h"
#include "AssetCache/AssetCacheClient.h"

#include "Platform/DeviceInfo.h"
#include "Platform/DateTime.h"

#include "SpriteResourcesPacker.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "Settings/SettingsManager.h"

SpriteResourcesPacker::~SpriteResourcesPacker()
{
}

void SpriteResourcesPacker::SetInputDir(const DAVA::FilePath& _inputDir)
{
    DVASSERT(_inputDir.IsDirectoryPathname());
    inputDir = _inputDir;
}

void SpriteResourcesPacker::SetOutputDir(const DAVA::FilePath& _outputDir)
{
    DVASSERT(_outputDir.IsDirectoryPathname());
    outputDir = _outputDir;
}

void SpriteResourcesPacker::PackLightmaps(DAVA::eGPUFamily gpu)
{
    return PerformPack(true, gpu);
}

void SpriteResourcesPacker::PackTextures(DAVA::eGPUFamily gpu)
{
    return PerformPack(false, gpu);
}

void SpriteResourcesPacker::PerformPack(bool isLightmapPacking, DAVA::eGPUFamily gpu)
{
    DAVA::FileSystem::Instance()->CreateDirectory(outputDir, true);

    DAVA::AssetCacheClient cacheClient(true);
    DAVA::ResourcePacker2D resourcePacker;

    resourcePacker.forceRepack = true;
    resourcePacker.InitFolders(inputDir, outputDir);
    resourcePacker.isLightmapsPacking = isLightmapPacking;

    bool shouldDisconnectClient = false;
    if (SettingsManager::GetValue(Settings::General_AssetCache_UseCache).AsBool())
    {
        DAVA::String ipStr = SettingsManager::GetValue(Settings::General_AssetCache_Ip).AsString();
        DAVA::uint16 port = static_cast<DAVA::uint16>(SettingsManager::GetValue(Settings::General_AssetCache_Port).AsUInt32());
        DAVA::uint64 timeoutSec = SettingsManager::GetValue(Settings::General_AssetCache_Timeout).AsUInt32();

        DAVA::AssetCacheClient::ConnectionParams params;
        params.ip = (ipStr.empty() ? DAVA::AssetCache::GetLocalHost() : ipStr);
        params.port = port;
        params.timeoutms = timeoutSec * 1000; //in ms

        DAVA::AssetCache::Error connected = cacheClient.ConnectSynchronously(params);
        if (connected == DAVA::AssetCache::Error::NO_ERRORS)
        {
            resourcePacker.SetCacheClient(&cacheClient, "Resource Editor.Repack Sprites");
            shouldDisconnectClient = true;
        }
    }

    resourcePacker.PackResources(gpu);
    if (shouldDisconnectClient)
    {
        cacheClient.Disconnect();
    }
}
