#pragma once

#include "TArcCore/ClientModule.h"
#include "DataProcessing/DataWrapper.h"

class DataChangerModule : public tarc::ClientModule, private tarc::DataListener
{
protected:
    void OnContextCreated(tarc::DataContext& context) override;
    void OnContextDeleted(tarc::DataContext& context) override;
    void PostInit(tarc::UI& ui) override;
    void OnDataChanged(const tarc::DataWrapper& wrapper) override;

private:
    tarc::DataWrapper wrapper;
};