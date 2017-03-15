#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>

namespace DAVA
{
namespace TArc
{
class DataContext;
}
}

class PackageListenerModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key);

    void OnPackageChanged(const DAVA::Any& package);

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;

    DAVA_VIRTUAL_REFLECTION(PackageListenerModule, DAVA::TArc::ClientModule);
};
