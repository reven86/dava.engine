#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrapper.h"

class InitModule : public DAVA::TArc::ClientModule, private DAVA::TArc::DataListener
{
protected:
    ~InitModule();

    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void PostInit() override;

private:
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;

private:
    DAVA::TArc::DataWrapper projectDataWrapper;
};