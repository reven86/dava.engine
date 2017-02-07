#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>

#include <TArc/Utils/QtConnections.h>

class FileSystemCacheModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;

    void CreateActions();
    void OnFindFile();

    void OnUIPathChanged(const DAVA::Any& path);

    std::unique_ptr<DAVA::TArc::FieldBinder> projectUiPathFieldBinder;
    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(FileSystemCacheModule, DAVA::TArc::ClientModule);
};
