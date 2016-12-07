#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

class MitsubaExporter : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;
    void Export();

private:
    class Private;
    DAVA::TArc::QtConnections connections;
};
