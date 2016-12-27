#include "Classes/PropertyPanel/PropertyPanelModule.h"

#include "TArc/Controls/PropertyPanel/ReflectedPropertyModel.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/DataProcessing/DataNode.h"
#include "TArc/DataProcessing/QtReflectionBridge.h"

#include "Reflection/ReflectionRegistrator.h"

#include <QPointer>
#include <QQmlEngine>
#include <QList>
#include <QString>

namespace PropertyPanelModuleDetail
{
class PropertyPanelData : public DAVA::TArc::DataNode
{
public:
    PropertyPanelData(QPointer<QQmlEngine> engine, QPointer<DAVA::TArc::QtReflectionBridge> reflectionBridge)
    {
        model = new DAVA::TArc::ReflectedPropertyModel(engine, reflectionBridge);
    }

    ~PropertyPanelData()
    {
        delete model;
    }

private:
    DAVA::TArc::ReflectedPropertyModel* model = nullptr;

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
    using namespace PropertyPanelModuleDetail;
    UI* ui = GetUI();

    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    ctx->CreateData(std::make_unique<PropertyPanelData>(ui->GetQmlEngine(), ui->GetReflectionBridge()));

    DataWrapper wrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<PropertyPanelData>());

    DockPanelInfo panelInfo;
    panelInfo.title = QStringLiteral("New Property Panel");
    panelInfo.actionPlacementInfo = ActionPlacementInfo(CreateMenuPoint(QList<QString>() << "View"
                                                                                         << "menuDock"));

    PanelKey panelKey(panelInfo.title, panelInfo);
    ui->AddView(REGlobal::MainWindowKey, panelKey, QStringLiteral("qrc:/TArc/PropertyPanel/Component/PropertyPanel.qml"), std::move(wrapper));
}

//DAVA_REFLECTION(PropertyPanelModule)
//{
//    DAVA::ReflectionRegistrator<PropertyPanelModule>::Begin()
//        .ConstructorByPointer()
//        .End();
//}
