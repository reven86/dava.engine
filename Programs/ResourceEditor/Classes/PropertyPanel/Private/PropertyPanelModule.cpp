#include "Classes/PropertyPanel/PropertyPanelModule.h"
#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/PropertyPanel/QualitySettingsComponentExt.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Application/REGlobal.h"

#include <TArc/Controls/PropertyPanel/PropertiesView.h>
#include <TArc/Controls/PropertyPanel/TimerUpdater.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/DataProcessing/DataNode.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Core/FieldBinder.h>

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

    std::shared_ptr<DAVA::TArc::TimerUpdater> updater;
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
    data->updater.reset(new TimerUpdater(1000, 100));

    DockPanelInfo panelInfo;
    panelInfo.title = QStringLiteral("New Property Panel");
    panelInfo.actionPlacementInfo = ActionPlacementInfo(CreateMenuPoint(QList<QString>() << "View"
                                                                                         << "Dock"));
    PropertiesView::Params params(REGlobal::MainWindowKey);
    params.accessor = accessor;
    params.invoker = GetInvoker();
    params.ui = ui;
    params.objectsField.type = DAVA::ReflectedTypeDB::Get<PropertyPanelData>();
    params.objectsField.fieldName = DAVA::FastName(PropertyPanelData::selectedEntitiesProperty);
    params.settingsNodeName = "PropertyPanel";
    params.updater = std::weak_ptr<PropertiesView::Updater>(data->updater);

    PropertiesView* view = new PropertiesView(params);

    view->RegisterExtension(std::make_shared<REModifyPropertyExtension>(accessor));
    view->RegisterExtension(std::make_shared<EntityChildCreator>());
    view->RegisterExtension(std::make_shared<EntityEditorCreator>());
    view->RegisterExtension(std::make_shared<QualitySettingsChildCreator>());
    view->RegisterExtension(std::make_shared<QualitySettingsEditorCreator>());
    ui->AddView(REGlobal::MainWindowKey, PanelKey(panelInfo.title, panelInfo), view);
}

DAVA_VIRTUAL_REFLECTION_IMPL(PropertyPanelModule)
{
    DAVA::ReflectionRegistrator<PropertyPanelModule>::Begin()
    .ConstructorByPointer()
    .End();
}

#if !defined(DEPLOY_BUILD)
DECL_GUI_MODULE(PropertyPanelModule);
#endif
