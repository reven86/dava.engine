#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/CheckBox.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QtTest>
#include <QStyle>
#include <QStyleOption>

namespace CheckBoxTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("");

class CheckBoxTestModule : public DAVA::TArc::ClientModule
{
public:
    CheckBoxTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        model.emplace("bool", true);
        model.emplace("checkState", Qt::PartiallyChecked);

        DAVA::Reflection reflectedModel = DAVA::Reflection::Create(&model);

        DAVA::TArc::CheckBox::FieldsDescriptor descrBool;
        descrBool.valueFieldName = DAVA::FastName("bool");
        DAVA::TArc::CheckBox* checkBool = new DAVA::TArc::CheckBox(descrBool, GetAccessor(), reflectedModel);
        DAVA::TArc::PanelKey keyBool("CheckBox_bool", DAVA::TArc::CentralPanelInfo());
        GetUI()->AddView(wndKey, keyBool, checkBool->ToWidgetCast());

        DAVA::TArc::CheckBox::FieldsDescriptor descrState;
        descrState.valueFieldName = DAVA::FastName("checkState");
        DAVA::TArc::CheckBox* checkState = new DAVA::TArc::CheckBox(descrState, GetAccessor(), reflectedModel);
        DAVA::TArc::PanelKey keyState("CheckBox_state", DAVA::TArc::CentralPanelInfo());
        GetUI()->AddView(wndKey, keyState, checkState->ToWidgetCast());
    }

    DAVA::Map<DAVA::String, DAVA::Any> model;

    static CheckBoxTestModule* instance;

    DAVA_VIRTUAL_REFLECTION(CheckBoxTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<CheckBoxTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

CheckBoxTestModule* CheckBoxTestModule::instance = nullptr;
}

DAVA_TARC_TESTCLASS(CheckBoxTest)
{
    DAVA_TEST (BoolTest)
    {
        using namespace CheckBoxTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("CheckBox_bool"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QStyle* style = w->style();
        QStyleOptionButton option;
        option.initFrom(w);
        QRect r = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option, w);

        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), r.center());

        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        TEST_VERIFY(checkBox != nullptr);
        CheckBoxTestModule* inst = CheckBoxTestModule::instance;
        TEST_VERIFY(inst->model.find("bool") != inst->model.end());

        eventList.simulate(w);
        TEST_VERIFY(checkBox->isChecked() == false);
        TEST_VERIFY(inst->model["bool"].Cast<bool>() == false);

        eventList.simulate(w);
        TEST_VERIFY(checkBox->isChecked() == true);
        TEST_VERIFY(inst->model["bool"].Cast<bool>() == true);
    }

    DAVA_TEST (CheckStateTest)
    {
        using namespace CheckBoxTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("CheckBox_state"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QStyle* style = w->style();
        QStyleOptionButton option;
        option.initFrom(w);
        QRect r = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option, w);
        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), r.center());

        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        TEST_VERIFY(checkBox != nullptr);
        CheckBoxTestModule* inst = CheckBoxTestModule::instance;
        TEST_VERIFY(inst->model.find("checkState") != inst->model.end());

        eventList.simulate(w);
        TEST_VERIFY(checkBox->checkState() == Qt::Checked);
        TEST_VERIFY(inst->model["checkState"].Cast<Qt::CheckState>() == Qt::Checked);

        eventList.simulate(w);
        TEST_VERIFY(checkBox->checkState() == Qt::Unchecked);
        TEST_VERIFY(inst->model["checkState"].Cast<Qt::CheckState>() == Qt::Unchecked);

        eventList.simulate(w);
        TEST_VERIFY(checkBox->checkState() == Qt::Checked);
        TEST_VERIFY(inst->model["checkState"].Cast<Qt::CheckState>() == Qt::Checked);
    }

    DAVA_TEST (BoolModuleTest)
    {
        using namespace CheckBoxTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("CheckBox_bool"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        connections.AddConnection(checkBox, &QCheckBox::stateChanged, DAVA::MakeFunction(this, &CheckBoxTest::OnStateChanged));

        EXPECT_CALL(*this, OnStateChanged(Qt::Unchecked))
        .WillOnce(Return());

        CheckBoxTestModule::instance->model["bool"] = false;
    }

    DAVA_TEST (CheckStateModuleTest)
    {
        using namespace CheckBoxTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("CheckBox_state"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        connections.AddConnection(checkBox, &QCheckBox::stateChanged, DAVA::MakeFunction(this, &CheckBoxTest::OnStateChanged));

        EXPECT_CALL(*this, OnStateChanged(Qt::PartiallyChecked))
        .WillOnce(Return());

        CheckBoxTestModule::instance->model["checkState"] = Qt::PartiallyChecked;
    }

    MOCK_METHOD1_VIRTUAL(OnStateChanged, void(int newState));

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(CheckBoxTestDetails::CheckBoxTestModule);
    END_TESTED_MODULES()

    DAVA::TArc::QtConnections connections;
};
