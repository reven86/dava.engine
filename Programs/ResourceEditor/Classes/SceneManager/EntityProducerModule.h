#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"
#include "Reflection/Reflection.h"

class EntityProducerModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;

    // actions
    void InstantiateCurrentCamera();

private:
    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(EntityProducerModule, DAVA::TArc::ClientModule);
};
