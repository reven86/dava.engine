#include "Classes/PropertyPanel/PropertyPanelModule.h"
#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Application/REGlobal.h"

#include "TArc/Controls/PropertyPanel/ReflectedPropertyModel.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/DataProcessing/DataNode.h"
#include "TArc/DataProcessing/QtReflectionBridge.h"
#include "TArc/Utils/ModuleCollection.h"
#include "TArc/Core/FieldBinder.h"

#include "Scene3D/Entity.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Base/FastName.h"

#include <QPointer>
#include <QQmlEngine>
#include <QList>
#include <QString>
#include <QTimer>

namespace PropertyPanelModuleDetail
{
class PropertyPanelData : public DAVA::TArc::DataNode
{
public:
    PropertyPanelData(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui)
    {
        model = new DAVA::TArc::ReflectedPropertyModel(ui->GetQmlEngine(), ui->GetReflectionBridge());
        model->RegisterExtension(std::make_shared<REModifyPropertyExtension>(accessor));

        QObject::connect(&timer, &QTimer::timeout, [this]()
                         {
                             model->Update();
                         });

        timer.setInterval(500);
        timer.setSingleShot(false);
        timer.start();
    }

    ~PropertyPanelData()
    {
        delete model;
    }

    DAVA::TArc::ReflectedPropertyModel* model = nullptr;
    QTimer timer;

    DAVA_VIRTUAL_REFLECTION(PropertyPanelData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<PropertyPanelData>::Begin()
        .Field("model", &PropertyPanelData::model)
        .End();
    }
};
}

PropertyPanelModule::~PropertyPanelModule() = default;

void PropertyPanelModule::PostInit()
{
    using namespace DAVA::TArc;
    using PropertyPanelData = PropertyPanelModuleDetail::PropertyPanelData;
    UI* ui = GetUI();

    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    ctx->CreateData(std::make_unique<PropertyPanelData>(accessor, ui));

    DataWrapper wrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<PropertyPanelData>());

    DockPanelInfo panelInfo;
    panelInfo.title = QStringLiteral("New Property Panel");
    panelInfo.actionPlacementInfo = ActionPlacementInfo(CreateMenuPoint(QList<QString>() << "View"
                                                                                         << "Dock"));

    PanelKey panelKey(panelInfo.title, panelInfo);
    ui->AddView(REGlobal::MainWindowKey, panelKey, QStringLiteral("qrc:/TArc/PropertyPanel/Component/PropertyPanel.qml"), std::move(wrapper));

    binder.reset(new FieldBinder(accessor));

    DAVA::TArc::FieldDescriptor fieldDescr;
    fieldDescr.fieldName = DAVA::FastName(SelectionData::selectionPropertyName);
    fieldDescr.type = ReflectedTypeDB::Get<SelectionData>();
    binder->BindField(fieldDescr, DAVA::MakeFunction(this, &PropertyPanelModule::SceneSelectionChanged));
}

void PropertyPanelModule::SceneSelectionChanged(const Any& newSelection)
{
    selection.clear();
    DAVA::Vector<DAVA::Reflection> objects;
    if (newSelection.CanGet<SelectableGroup>())
    {
        SelectableGroup group = newSelection.Get<SelectableGroup>();
        for (auto entity : group.ObjectsOfType<DAVA::Entity>())
        {
            selection.push_back(entity);
        }

        for (size_t i = 0; i < selection.size(); ++i)
        {
            objects.push_back(DAVA::Reflection::Create(&selection[i]));
        }
    }

    DAVA::TArc::DataContext* ctx = GetAccessor()->GetGlobalContext();
    using PropertyPanelData = PropertyPanelModuleDetail::PropertyPanelData;
    PropertyPanelData* data = ctx->GetData<PropertyPanelData>();
    data->model->SetObjects(objects);
}

DAVA_REFLECTION_IMPL(PropertyPanelModule)
{
    DAVA::ReflectionRegistrator<PropertyPanelModule>::Begin()
        .ConstructorByPointer()
        .End();
}

DECL_GUI_MODULE(PropertyPanelModule);
