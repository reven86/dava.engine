#include "ControllerModule.h"

#include "SharedData.h"

#include "WindowSubSystem/UI.h"
#include "WindowSubSystem/ActionUtils.h"
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

void TemplateControllerModule::PostInit()
{
    tarc::UI& ui = GetUI();
    tarc::ContextManager& manager = GetContextManager();
    contextID = manager.CreateContext();
    manager.ActivateContext(contextID);
    wrapper = GetAccessor().CreateWrapper(DAVA::ReflectedType::Get<SharedData>());
    wrapper.AddListener(this);

    tarc::WindowKey windowKey(DAVA::FastName("TemplateTArc"));

    tarc::CentralPanelInfo info;
    ui.AddView(windowKey, tarc::PanelKey("RenderWidget", info), manager.GetRenderWidget());

    QAction* action = new QAction(QString("DAE"), nullptr);
    QObject::connect(action, &QAction::triggered, []()
                     {
                         DAVA::Logger::Info("Action triggered");
                     });

    {
        tarc::ActionPlacementInfo openFilePlacement;
        openFilePlacement.AddPlacementPoint(tarc::CreateMenuPoint("File"));
        openFilePlacement.AddPlacementPoint(tarc::CreateToolbarPoint("FileToolBar"));
        openFilePlacement.AddPlacementPoint(tarc::CreateStatusbarPoint(true));

        QAction* action = new QAction(QString("Open"), nullptr);
        QObject::connect(action, &QAction::triggered, []()
                         {
                             DAVA::Logger::Info("Open triggered");
                         });

        ui.AddAction(windowKey, openFilePlacement, action);
    }
    {
        tarc::ActionPlacementInfo exportPlacement;
        exportPlacement.AddPlacementPoint(tarc::CreateMenuPoint("File/Export"));
        exportPlacement.AddPlacementPoint(tarc::CreateToolbarPoint("ExportToolBar"));
        ui.AddAction(windowKey, exportPlacement, action);
        ui.AddAction(windowKey, exportPlacement, new QAction(QString("SC2"), nullptr));
    }
}

void TemplateControllerModule::OnDataChanged(const tarc::DataWrapper&, const DAVA::Set<DAVA::String>& fields)
{
    if (wrapper.HasData())
    {
        tarc::DataEditor<SharedData> editor = wrapper.CreateEditor<SharedData>();
        QString msg = QString("Most longest message that i could imagine in my cruel life. Sorry for that! Data changed. New value : %1").arg(editor->GetValue());
        GetUI().ShowMessage(tarc::WindowKey(DAVA::FastName("TemplateTArc")), msg);
    }
    else
    {
        GetUI().ShowMessage(tarc::WindowKey(DAVA::FastName("TemplateTArc")), "Data changed. New value : empty", 1000);
    }
}
