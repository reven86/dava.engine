#include "ControllerModule.h"

#include "SharedData.h"
#include "TArcCore/ContextAccessor.h"

void TemplateControllerModule::OnContextCreated(tarc::DataContext& context)
{
    context.CreateData(std::make_unique<SharedData>());
}

void TemplateControllerModule::OnContextDeleted(tarc::DataContext& context)
{
}

void TemplateControllerModule::PostInit()
{
    contextID = GetContextManager().CreateContext();
    GetContextManager().ActivateContext(contextID);
    wrapper = GetAccessor().CreateWrapper(DAVA::Type::Instance<SharedData>());
    wrapper.AddListener(this);
}

void TemplateControllerModule::OnDataChanged(const tarc::DataWrapper&)
{
    tarc::DataContext::ContextID newContext = tarc::DataContext::Empty;
    if (wrapper.HasData())
    {
        tarc::DataWrapper::Editor<SharedData> editor = wrapper.CreateEditor<SharedData>();
        DAVA::Logger::Info("Data changed. New value : %d", editor->GetValue());
    }
    else
    {
        DAVA::Logger::Info("Data changed. New value : empty");
        newContext = contextID;
    }
    //GetContextManager().ActivateContext(newContext);
}
