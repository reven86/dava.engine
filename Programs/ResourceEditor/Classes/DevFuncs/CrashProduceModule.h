#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

class CrashProduceModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;

private:
    DAVA::TArc::QtConnections connections;
};