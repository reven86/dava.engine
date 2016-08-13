#pragma once

#include "TArcCore/ClientModule.h"
#include "DataProcessing/DataWrapper.h"

#include "Base/BaseTypes.h"

class DataChangerModule : public tarc::ClientModule, private tarc::DataListener
{
protected:
    void OnContextCreated(tarc::DataContext& context) override;
    void OnContextDeleted(tarc::DataContext& context) override;
    void PostInit(tarc::UI& ui) override;
    void OnDataChanged(const tarc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;

private:
    tarc::DataWrapper wrapper;
};