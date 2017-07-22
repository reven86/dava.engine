#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Tools/AssetCache/AssetCacheClient.h>

class ProjectData;

class SpritesPackerModule;
class SpritesPackerModuleSettings : public DAVA::InspBase
{
public:
    SpritesPackerModuleSettings(SpritesPackerModule* module);

    bool IsUsingAssetCache() const;
    void SetUsingAssetCacheEnabled(bool enabled);

    INTROSPECTION(SpritesPackerModuleSettings,
                  PROPERTY("isUsingAssetCache", "Asset cache/Use asset cache", IsUsingAssetCache, SetUsingAssetCacheEnabled, DAVA::I_PREFERENCE)
                  )
private:
    SpritesPackerModule* module = nullptr;
};

class SpritesPackerModule : public DAVA::TArc::ClientModule
{
    friend class SpritesPackerModuleSettings;

public:
    SpritesPackerModule();
    ~SpritesPackerModule() override;

private:
    void OnReloadFinished();
    void OnReloadSprites();

    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;

    void CreateActions();
    bool IsUsingAssetCache() const;
    void SetUsingAssetCacheEnabled(bool enabled);
    void EnableCacheClient();
    void DisableCacheClient();

    DAVA::TArc::QtConnections connections;

    std::unique_ptr<DAVA::AssetCacheClient> cacheClient;
    DAVA::AssetCacheClient::ConnectionParams connectionParams;

    DAVA_VIRTUAL_REFLECTION(SpritesPackerModule, DAVA::TArc::ClientModule);

private:
    std::unique_ptr<SpritesPackerModuleSettings> settings;
};
