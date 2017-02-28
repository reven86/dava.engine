#include "Classes/PropertyPanel/PropertyPanelModule.h"
#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Application/REGlobal.h"

#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/DataProcessing/DataNode.h"
#include "TArc/Utils/ModuleCollection.h"
#include "TArc/Core/FieldBinder.h"

#include <Scene3D/Entity.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>
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
    DAVA::Vector<DAVA::Entity*> selectedEntities;
    DAVA::Vector<DAVA::Reflection> propertyPanelObjects;

    static const char* selectedEntitiesProperty;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(PropertyPanelData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<PropertyPanelData>::Begin()
        .Field(selectedEntitiesProperty, &PropertyPanelData::propertyPanelObjects)
        .End();
    }
};

const char* PropertyPanelData::selectedEntitiesProperty = "selectedEntities";
}

void PropertyPanelModule::PostInit()
{
    using namespace DAVA::TArc;
    using PropertyPanelData = PropertyPanelModuleDetail::PropertyPanelData;
    UI* ui = GetUI();

    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    ctx->CreateData(std::make_unique<PropertyPanelData>());

    DockPanelInfo panelInfo;
    panelInfo.title = QStringLiteral("New Property Panel");
    panelInfo.actionPlacementInfo = ActionPlacementInfo(CreateMenuPoint(QList<QString>() << "View"
                                                                                         << "Dock"));

    FieldDescriptor propertiesDataSourceField;
    propertiesDataSourceField.type = DAVA::ReflectedTypeDB::Get<PropertyPanelModuleDetail::PropertyPanelData>();
    propertiesDataSourceField.fieldName = DAVA::FastName(PropertyPanelModuleDetail::PropertyPanelData::selectedEntitiesProperty);

    PropertiesView* view = new PropertiesView(accessor, propertiesDataSourceField, "PropertyPanel");
    view->RegisterExtension(std::make_shared<REModifyPropertyExtension>(accessor));
    view->RegisterExtension(std::make_shared<EntityChildCreator>());
    ui->AddView(REGlobal::MainWindowKey, PanelKey(panelInfo.title, panelInfo), view);

    // Bind to current selection changed
    binder.reset(new FieldBinder(accessor));
    FieldDescriptor fieldDescr;
    fieldDescr.fieldName = DAVA::FastName(SelectionData::selectionPropertyName);
    fieldDescr.type = DAVA::ReflectedTypeDB::Get<SelectionData>();
    binder->BindField(fieldDescr, DAVA::MakeFunction(this, &PropertyPanelModule::SceneSelectionChanged));
}

void PropertyPanelModule::SceneSelectionChanged(const DAVA::Any& newSelection)
{
    using namespace DAVA::TArc;

    DataContext* ctx = GetAccessor()->GetGlobalContext();
    PropertyPanelModuleDetail::PropertyPanelData* data = ctx->GetData<PropertyPanelModuleDetail::PropertyPanelData>();
    data->selectedEntities.clear();
    data->selectedEntities.shrink_to_fit();
    data->propertyPanelObjects.clear();

    if (newSelection.CanGet<SelectableGroup>())
    {
        const SelectableGroup& group = newSelection.Get<SelectableGroup>();
        for (auto entity : group.ObjectsOfType<DAVA::Entity>())
        {
            data->selectedEntities.push_back(entity);
        }

        for (size_t i = 0; i < data->selectedEntities.size(); ++i)
        {
            data->propertyPanelObjects.push_back(DAVA::Reflection::Create(&data->selectedEntities[i]));
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(PropertyPanelModule)
{
    DAVA::ReflectionRegistrator<PropertyPanelModule>::Begin()
    .ConstructorByPointer()
    .End();
}

#if !defined(DEPLOY_BUILD)
//DECL_GUI_MODULE(PropertyPanelModule);
#endif
