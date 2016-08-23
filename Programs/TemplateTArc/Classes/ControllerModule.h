#pragma once

#include "TArcCore/ControllerModule.h"
#include "DataProcessing/DataContext.h"
#include "DataProcessing/DataWrapper.h"
#include "DataProcessing/DataListener.h"

#include "Base/BaseTypes.h"

class TemplateControllerModule : public DAVA::TArc::ControllerModule, public DAVA::TArc::DataListener
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void PostInit() override;

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;

private:
    DAVA::TArc::DataWrapper wrapper;
    DAVA::TArc::DataContext::ContextID contextID;
};