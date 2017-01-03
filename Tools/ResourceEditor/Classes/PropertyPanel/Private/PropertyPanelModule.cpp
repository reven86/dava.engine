#include "Classes/PropertyPanel/PropertyPanelModule.h"
#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/Qt/Scene/SceneSignals.h"
#include "Classes/Qt/Scene/SelectableGroup.h"
#include "Classes/Application/REGlobal.h"

#include "TArc/Controls/PropertyPanel/ReflectedPropertyModel.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/DataProcessing/DataNode.h"
#include "TArc/DataProcessing/QtReflectionBridge.h"

#include "Scene3D/Entity.h"
#include "Reflection/ReflectionRegistrator.h"

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

    SceneSignals* sceneSignals = SceneSignals::Instance();
    connections.AddConnection(sceneSignals, &SceneSignals::SelectionChanged, DAVA::MakeFunction(this, &PropertyPanelModule::SceneSelectionChanged));
}

void PropertyPanelModule::SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected)
{
    selection.clear();
    for (auto entity : selected->ObjectsOfType<DAVA::Entity>())
    {
        selection.push_back(entity);
    }

    DAVA::Vector<DAVA::Reflection> objects;
    for (size_t i = 0; i < selection.size(); ++i)
    {
        objects.push_back(DAVA::Reflection::Create(&selection[i]));
    }

    DAVA::TArc::DataContext* ctx = GetAccessor()->GetGlobalContext();
    using PropertyPanelData = PropertyPanelModuleDetail::PropertyPanelData;
    PropertyPanelData* data = ctx->GetData<PropertyPanelData>();
    data->model->SetObjects(objects);
}

//DAVA_REFLECTION(PropertyPanelModule)
//{
//    DAVA::ReflectionRegistrator<PropertyPanelModule>::Begin()
//        .ConstructorByPointer()
//        .End();
//}
