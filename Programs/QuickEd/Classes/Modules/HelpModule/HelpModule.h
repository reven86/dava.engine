#pragma once

#include <TArc/Core/ClientModule.h>

#include <TArc/Utils/QtConnections.h>

class HelpModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;

    void UnpackHelp();
    void CreateActions();
    void OnShowHelp();

    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(HelpModule, DAVA::TArc::ClientModule);
};
