#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

#include "Reflection/Reflection.h"

class CrashProduceModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;

private:
    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(CrashProduceModule, DAVA::TArc::ClientModule);
};