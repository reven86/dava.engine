#include "ControllerModule.h"

#include "SharedData.h"

#include "WindowSubSystem/UI.h"
#include "WindowSubSystem/ActionUtils.h"
#include "TArcCore/ContextAccessor.h"

#include <QAction>
#include <QUrl>

void TemplateControllerModule::OnContextCreated(DAVA::TArc::DataContext& context)
{
    context.CreateData(std::make_unique<SharedData>());
}

void TemplateControllerModule::OnContextDeleted(DAVA::TArc::DataContext& context)
{
}

void TemplateControllerModule::PostInit()
{
    DAVA::TArc::UI& ui = GetUI();
    DAVA::TArc::ContextManager& manager = GetContextManager();
    contextID = manager.CreateContext();
    manager.ActivateContext(contextID);
    wrapper = GetAccessor().CreateWrapper(DAVA::ReflectedType::Get<SharedData>());
    wrapper.AddListener(this);

    DAVA::TArc::WindowKey windowKey(DAVA::FastName("TemplateTArc"));

    DAVA::TArc::CentralPanelInfo info;
    ui.AddView(windowKey, DAVA::TArc::PanelKey("RenderWidget", info), manager.GetRenderWidget());

    QAction* action = new QAction(QString("DAE"), nullptr);
    QObject::connect(action, &QAction::triggered, []()
                     {
                         DAVA::Logger::Info("Action triggered");
                     });

    {
        DAVA::TArc::ActionPlacementInfo openFilePlacement;
        openFilePlacement.AddPlacementPoint(DAVA::TArc::CreateMenuPoint("File"));
        openFilePlacement.AddPlacementPoint(DAVA::TArc::CreateToolbarPoint("FileToolBar"));
        openFilePlacement.AddPlacementPoint(DAVA::TArc::CreateStatusbarPoint(true));

        QAction* action = new QAction(QString("Open"), nullptr);
        QObject::connect(action, &QAction::triggered, []()
                         {
                             DAVA::Logger::Info("Open triggered");
                         });

        ui.AddAction(windowKey, openFilePlacement, action);
    }
    {
        DAVA::TArc::ActionPlacementInfo exportPlacement;
        exportPlacement.AddPlacementPoint(DAVA::TArc::CreateMenuPoint("File/Export"));
        exportPlacement.AddPlacementPoint(DAVA::TArc::CreateToolbarPoint("ExportToolBar"));
        ui.AddAction(windowKey, exportPlacement, action);
        ui.AddAction(windowKey, exportPlacement, new QAction(QString("SC2"), nullptr));
    }
}

void TemplateControllerModule::OnDataChanged(const DAVA::TArc::DataWrapper&, const DAVA::Set<DAVA::String>& fields)
{
    if (wrapper.HasData())
    {
        DAVA::TArc::DataEditor<SharedData> editor = wrapper.CreateEditor<SharedData>();
        QString msg = QString("Most longest message that i could imagine in my cruel life. Sorry for that! Data changed. New value : %1").arg(editor->GetValue());
        GetUI().ShowMessage(DAVA::TArc::WindowKey(DAVA::FastName("TemplateTArc")), msg);
    }
    else
    {
        GetUI().ShowMessage(DAVA::TArc::WindowKey(DAVA::FastName("TemplateTArc")), "Data changed. New value : empty", 1000);
    }
}
