#pragma once

#include "TArcCore/ClientModule.h"
#include "DataProcessing/DataWrapper.h"

#include "Base/BaseTypes.h"

class LibraryModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void PostInit() override;

    friend class FileSystemData;
};