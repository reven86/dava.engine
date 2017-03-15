#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class SettingsModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;

private:
    void ShowSettings();

    DAVA::TArc::QtConnections connections;
    DAVA_VIRTUAL_REFLECTION(SettingsModule, DAVA::TArc::ClientModule);
};
