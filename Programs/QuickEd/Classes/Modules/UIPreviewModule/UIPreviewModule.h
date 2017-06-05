#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/DataProcessing/DataListener.h>
#include <TArc/Utils/QtConnections.h>

class UIPreviewModule : public DAVA::TArc::ClientModule
{
private:
    void PostInit() override;

    void RunUIViewer();
    void CollectAllFiles();
    void CollectUsedFiles();

    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(UIPreviewModuleModule, DAVA::TArc::ClientModule);
};
