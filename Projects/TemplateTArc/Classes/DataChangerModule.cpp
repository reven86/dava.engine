#include "DataChangerModule.h"
#include "SharedData.h"

#include "TArcCore/ContextAccessor.h"

#include "Base/Type.h"

void DataChangerModule::OnContextCreated(tarc::DataContext& context)
{
}

void DataChangerModule::OnContextDeleted(tarc::DataContext& context)
{
}

void DataChangerModule::PostInit()
{
    wrapper = GetAccessor().CreateWrapper(DAVA::Type::Instance<SharedData>());
    wrapper.AddListener(this);
}

void DataChangerModule::OnDataChanged(const tarc::DataWrapper& wrapper_)
{
    if (wrapper.HasData())
    {
        tarc::DataWrapper::Editor<SharedData> editor = wrapper.CreateEditor<SharedData>();
        DAVA::Logger::Info("Changer %d", editor->GetValue());
        editor->SetValue(editor->GetValue() + 1);
    }
    else
    {
        DAVA::Logger::Info("Changer empty");
    }
}
