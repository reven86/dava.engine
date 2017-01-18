#include "Classes/DevFuncs/TestUIModule.h"
#include "Classes/Application/REGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/CheckBox.h>

#include <Reflection/ReflectionRegistrator.h>

#include <Logger/Logger.h>

#include <QAction>
#include <QDialog>
#include <QVBoxLayout>

void TestUIModule::PostInit()
{
    using namespace DAVA::TArc;
    UI* ui = GetUI();
    ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "DebugFunctions"
                                                                       << "Test UI"));

    QAction* assertAction = new QAction("CheckBox", nullptr);
    connections.AddConnection(assertAction, &QAction::triggered, DAVA::MakeFunction(this, &TestUIModule::TestCheckBox));
    ui->AddAction(REGlobal::MainWindowKey, placementInfo, assertAction);
}

struct TestData
{
    bool GetInvertedValue() const
    {
        return !value;
    }
    void SetInvertedValue(bool v)
    {
        DAVA::Logger::Info("++++ newValue: %d", !v);
        value = !v;
    }
    bool value = false;

    DAVA_REFLECTION(TestData);
};

DAVA_REFLECTION_IMPL(TestData)
{
    DAVA::ReflectionRegistrator<TestData>::Begin()
    .Field("value", &TestData::value)
    .Field("invertedValue", &TestData::GetInvertedValue, &TestData::SetInvertedValue)
    .End();
}

void TestUIModule::TestCheckBox()
{
    using namespace DAVA::TArc;

    TestData testData;
    DAVA::Reflection reflection = DAVA::Reflection::Create(&testData);

    QDialog* dlg = new QDialog();
    QVBoxLayout* layout = new QVBoxLayout();
    dlg->setLayout(layout);

    CheckBox::FieldsDescriptor valueDescr;
    valueDescr.valueFieldName = FastName("value");
    CheckBox* valueBox = new CheckBox(valueDescr, GetAccessor(), reflection, dlg); //->ToWidgetCast();
    layout->addWidget(valueBox->ToWidgetCast());

    CheckBox::FieldsDescriptor valueInvertedDescr;
    valueInvertedDescr.valueFieldName = FastName("invertedValue");
    CheckBox* valueInvertedBox = new CheckBox(valueInvertedDescr, GetAccessor(), reflection, dlg); //->ToWidgetCast();
    layout->addWidget(valueInvertedBox->ToWidgetCast());

    dlg->exec();
}

DAVA_REFLECTION_IMPL(TestUIModule)
{
    DAVA::ReflectionRegistrator<TestUIModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(TestUIModule);
