#pragma once

#include "TArc/Core/ClientModule.h"

#include "QtTools/Utils/QtDelayedExecutor.h"

class LaunchModule : public DAVA::TArc::ClientModule
{
public:
    ~LaunchModule();

protected:
    void PostInit() override;
    void UnpackHelpDoc();

private:
    class FirstSceneCreator;
    QtDelayedExecutor delayedExecutor;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LaunchModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<LaunchModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};