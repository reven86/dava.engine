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

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(EditModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<EditModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
