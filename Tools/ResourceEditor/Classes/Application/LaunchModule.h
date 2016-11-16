#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/DataProcessing/DataContext.h"
#include "TArc/DataProcessing/DataListener.h"

class LaunchModule : public DAVA::TArc::ClientModule, private DAVA::TArc::DataListener
{
protected:
    void PostInit() override;

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

private:
    DAVA::TArc::DataWrapper projectDataWrapper;
};