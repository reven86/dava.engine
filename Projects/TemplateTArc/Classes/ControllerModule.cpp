#include "ControllerModule.h"

#include "SharedData.h"

#include "WindowSubSystem/UI.h"
#include "TArcCore/ContextAccessor.h"

#include <QAction>
#include <QUrl>

void TemplateControllerModule::OnContextCreated(tarc::DataContext& context)
{
    context.CreateData(std::make_unique<SharedData>());
}

void TemplateControllerModule::OnContextDeleted(tarc::DataContext& context)
{
}

void TemplateControllerModule::PostInit(tarc::UI& ui)
{
    tarc::ContextManager& manager = GetContextManager();
    contextID = manager.CreateContext();
    manager.ActivateContext(contextID);
    wrapper = GetAccessor().CreateWrapper(DAVA::Type::Instance<SharedData>());
    wrapper.AddListener(this);

    tarc::WindowKey windowKey(DAVA::FastName("TemplateTArc"));

    tarc::CentralPanelInfo info;
    ui.AddView(windowKey, tarc::PanelKey("RenderWidget", info), manager.GetRenderWidget());
    manager.GetRenderWidget()->show();

    QAction* action = new QAction(QString("DAE"), nullptr);
    QObject::connect(action, &QAction::triggered, []()
                     {
                         DAVA::Logger::Info("Action triggered");
                     });

    ui.AddAction(windowKey, tarc::ActionPlacementInfo(QUrl("menu:File")), new QAction(QString("Open"), nullptr));
    ui.AddAction(windowKey, tarc::ActionPlacementInfo(QUrl("menu:File/Export")), action);
    ui.AddAction(windowKey, tarc::ActionPlacementInfo(QUrl("menu:File/Export")), new QAction(QString("SC2"), nullptr));
}

void TemplateControllerModule::OnDataChanged(const tarc::DataWrapper&, const DAVA::Set<DAVA::String>& fields)
{
    tarc::DataContext::ContextID newContext = tarc::DataContext::Empty;
    if (wrapper.HasData())
    {
        tarc::DataEditor<SharedData> editor = wrapper.CreateEditor<SharedData>();
        DAVA::Logger::Info("Data changed. New value : %d", editor->GetValue());
    }
    else
    {
        DAVA::Logger::Info("Data changed. New value : empty");
        newContext = contextID;
    }
    //GetContextManager().ActivateContext(newContext);
}
