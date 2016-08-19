#pragma once

#include "TArcCore/ClientModule.h"
#include "DataProcessing/DataWrapper.h"

#include "Base/BaseTypes.h"

class LibraryModule : public tarc::ClientModule
{
protected:
    void OnContextCreated(tarc::DataContext& context) override;
    void OnContextDeleted(tarc::DataContext& context) override;
    void PostInit() override;
};