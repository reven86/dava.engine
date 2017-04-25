#include "Classes/PropertyPanel/PropertyPanelModule.h"
#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/PropertyPanel/QualitySettingsComponentExt.h"
#include "Classes/PropertyPanel/KeyedArchiveExtensions.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Qt/Scene/SceneSignals.h"

#include <TArc/Controls/PropertyPanel/PropertiesView.h>
#include <TArc/Controls/PropertyPanel/TimerUpdater.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/DataProcessing/DataNode.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Core/FieldBinder.h>

#include <QtTools/Utils/QtDelayedExecutor.h>

#include <Scene3D/Entity.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedObject.h>
#include <Base/FastName.h>
#include <Base/BaseTypes.h>

#include <QPointer>
#include <QList>
#include <QString>
#include <QTimer>

namespace PropertyPanelModuleDetail
{
class PropertyPanelUpdater : public DAVA::TArc::PropertiesView::Updater
{
public:
    PropertyPanelUpdater()
        : timerUpdater(1000, 100)
    {
        SceneSignals* sceneSignals = SceneSignals::Instance();
        connections.AddConnection(sceneSignals, &SceneSignals::CommandExecuted, [this](SceneEditor2* scene, const RECommandNotificationObject& commandNotification)
                                  {
                                      QueueFullUpdate();
                                  });

        timerUpdater.update.Connect(this, &PropertyPanelUpdater::EmitUpdate);
    }

private:
    void EmitUpdate(DAVA::TArc::PropertiesView::UpdatePolicy policy)
    {
        update.Emit(policy);
    }

    void QueueFullUpdate()
    {
        executor.DelayedExecute(DAVA::Bind(&PropertyPanelUpdater::EmitUpdate, this, DAVA::TArc::PropertiesView::FullUpdate));
    }

private:
    DAVA::TArc::TimerUpdater timerUpdater;
    DAVA::TArc::QtConnections connections;
    QtDelayedExecutor executor;
};

class PropertyPanelData : public DAVA::TArc::DataNode
{
public:
    PropertyPanelData(DAVA::TArc::ContextAccessor* accessor_)
        : accessor(accessor_)
    {
    }

    DAVA::Vector<DAVA::Reflection> GetSelectedEntities() const
    {
        using namespace DAVA::TArc;

        DAVA::Vector<DAVA::Reflection> result;
        const DataContext* ctx = accessor->GetActiveContext();
        if (ctx == nullptr)
        {
            return result;
        }

        SelectionData* data = ctx->GetData<SelectionData>();
        const SelectableGroup& group = data->GetSelection();
        result.reserve(group.GetSize());
        for (auto entity : group.ObjectsOfType<DAVA::Entity>())
        {
            result.push_back(DAVA::Reflection::Create(DAVA::ReflectedObject(entity)));
        }

        return result;
    }

    static const char* selectedEntitiesProperty;

    std::shared_ptr<DAVA::TArc::PropertiesView::Updater> updater;
    DAVA::TArc::ContextAccessor* accessor = nullptr;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(PropertyPanelData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<PropertyPanelData>::Begin()
        .Field(selectedEntitiesProperty, &PropertyPanelData::GetSelectedEntities, nullptr)
        .End();
    }
};

const char* PropertyPanelData::selectedEntitiesProperty = "selectedEntities";
}

void PropertyPanelModule::PostInit()
{
    using namespace DAVA::TArc;
    using namespace PropertyPanelModuleDetail;
    UI* ui = GetUI();

    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    ctx->CreateData(std::make_unique<PropertyPanelData>(accessor));

    PropertyPanelData* data = ctx->GetData<PropertyPanelData>();
    data->updater.reset(new PropertyPanelUpdater());

    DockPanelInfo panelInfo;
    panelInfo.title = QStringLiteral("New Property Panel");
    panelInfo.actionPlacementInfo = ActionPlacementInfo(CreateMenuPoint(QList<QString>() << MenuItems::menuView
                                                                                         << "Dock"));
    PropertiesView::Params params(DAVA::TArc::mainWindowKey);
    params.accessor = accessor;
    params.invoker = GetInvoker();
    params.ui = ui;
    params.objectsField.type = DAVA::ReflectedTypeDB::Get<PropertyPanelData>();
    params.objectsField.fieldName = DAVA::FastName(PropertyPanelData::selectedEntitiesProperty);
    params.settingsNodeName = "PropertyPanel";
    params.updater = std::weak_ptr<PropertiesView::Updater>(data->updater);
#if !defined(DEPLOY_BUILD)
    params.isInDevMode = true;
#endif

    PropertiesView* view = new PropertiesView(params);

    view->RegisterExtension(std::make_shared<REModifyPropertyExtension>(accessor));
    view->RegisterExtension(std::make_shared<EntityChildCreator>());
    view->RegisterExtension(std::make_shared<EntityEditorCreator>());
    view->RegisterExtension(std::make_shared<QualitySettingsChildCreator>());
    view->RegisterExtension(std::make_shared<QualitySettingsEditorCreator>());
    view->RegisterExtension(std::make_shared<KeyedArchiveChildCreator>());
    view->RegisterExtension(std::make_shared<KeyedArchiveEditorCreator>(accessor));
    ui->AddView(DAVA::TArc::mainWindowKey, PanelKey(panelInfo.title, panelInfo), view);
}

DAVA_VIRTUAL_REFLECTION_IMPL(PropertyPanelModule)
{
    DAVA::ReflectionRegistrator<PropertyPanelModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(PropertyPanelModule);
