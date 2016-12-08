#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"
#include "Reflection/ReflectionRegistrator.h"

class MitsubaExporter : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;
    void Export();

private:
    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(MitsubaExporter, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<MitsubaExporter>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
