#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

namespace DAVA
{
class Scene;
class FilePath;
}

class MitsubaExporter : public DAVA::TArc::ClientModule
{
public:
    //bool Export(DAVA::Scene* scene, const DAVA::FilePath& toFile);

protected:
    void PostInit() override;
    void Export();

private:
    class Private;
    DAVA::TArc::QtConnections connections;
};
