#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Tools/AssetCache/AssetCacheClient.h>

class ProjectData;

class SpritesPackerModule : public DAVA::TArc::ClientModule, public DAVA::InspBase
{
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

public:
    INTROSPECTION(SpritesPackerModule,
                  PROPERTY("isUsingAssetCache", "Asset cache/Use asset cache", IsUsingAssetCache, SetUsingAssetCacheEnabled, DAVA::I_PREFERENCE)
                  )
};
