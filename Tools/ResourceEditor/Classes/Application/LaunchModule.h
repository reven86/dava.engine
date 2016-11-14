#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/DataProcessing/DataContext.h"
#include "TArc/DataProcessing/DataListener.h"

class LaunchModule : public DAVA::TArc::ClientModule, private DAVA::TArc::DataListener
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void PostInit() override;

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;

private:
    DAVA::TArc::DataWrapper projectDataWrapper;
};