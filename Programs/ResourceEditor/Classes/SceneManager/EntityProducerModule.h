#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

class EntityProducerModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;

    // actions
    void InstantiateCurrentCamera();

private:
    DAVA::TArc::QtConnections connections;
};
