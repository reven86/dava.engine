#pragma once

#include "TArcCore/ControllerModule.h"
#include "DataProcessing/DataContext.h"
#include "DataProcessing/DataWrapper.h"

#include "Base/BaseTypes.h"

class TemplateControllerModule : public tarc::ControllerModule, public tarc::DataListener
{
protected:
    void OnContextCreated(tarc::DataContext& context) override;
    void OnContextDeleted(tarc::DataContext& context) override;
    void PostInit(tarc::UI& ui) override;

    void OnDataChanged(const tarc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;

private:
    tarc::DataWrapper wrapper;
    tarc::DataContext::ContextID contextID;
};