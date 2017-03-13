#pragma once

#include <TArc/Core/ClientModule.h>

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

    void OnContextWillBeChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne) override;
    void OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne) override;

    DAVA_VIRTUAL_REFLECTION(PackageListenerModule, DAVA::TArc::ClientModule);
};
