#pragma once

#include <TArc/Core/ClientModule.h>

class PreferencesModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(PreferencesModule, DAVA::TArc::ClientModule);
};
