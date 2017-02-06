#pragma once

#include <TArc/Core/ClientModule.h>

#include <TArc/Utils/QtConnections.h>

class EditModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;

    void CreateActions();

    void OnUndo();
    void OnRedo();

    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(EditModule, DAVA::TArc::ClientModule);
};
