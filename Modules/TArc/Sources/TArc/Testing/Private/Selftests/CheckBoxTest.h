#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/CheckBox.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QtTest>
#include <QStyle>
#include <QStyleOption>

namespace CheckBoxTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("CheckBoxTestWnd");

struct CheckBoxDataSource
{
    bool value = true;
    bool isReadOnly = false;

    bool GetValue() const
    {
        return value;
    }

    static DAVA::String GetDescription(const DAVA::Any& v)
    {
        return v.Cast<bool>() == true ? "True" : "False";
    }

    DAVA::String GetValueDescription() const
    {
        return value == true ? "Visible" : "Invisible";
    }

    DAVA_REFLECTION(CheckBoxDataSource)
    {
        DAVA::ReflectionRegistrator<CheckBoxDataSource>::Begin()
        .Field("value", &CheckBoxDataSource::value)[DAVA::M::ReadOnly(), DAVA::M::ValueDescription(&CheckBoxDataSource::GetDescription)]
        .Field("readOnlyValue", &CheckBoxDataSource::GetValue, nullptr)
        .Field("writableValue", &CheckBoxDataSource::value)
        .Field("isReadOnly", &CheckBoxDataSource::isReadOnly)
        .Field("writableDescription", &CheckBoxDataSource::GetValueDescription, nullptr)
        .End();
    }
};

class CheckBoxTestModule : public DAVA::TArc::ClientModule
{
public:
    CheckBoxTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;
        model.emplace("bool", true);
        model.emplace("checkState", Qt::PartiallyChecked);

        DAVA::Reflection reflectedModel = DAVA::Reflection::Create(&model);

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        {
            CheckBox::Params params(GetAccessor(), GetUI(), CheckBoxTestDetails::wndKey);
            params.fields[CheckBox::Fields::Checked] = "bool";
            CheckBox* checkBox = new CheckBox(params, GetAccessor(), reflectedModel);
            checkBox->SetObjectName("CheckBox_bool");
            layout->AddControl(checkBox);
        }

        {
            CheckBox::Params params(GetAccessor(), GetUI(), CheckBoxTestDetails::wndKey);
            params.fields[CheckBox::Fields::Checked] = "checkState";
            CheckBox* checkState = new CheckBox(params, GetAccessor(), reflectedModel);
            checkState->SetObjectName("CheckBox_state");
            layout->AddControl(checkState);
        }

        DAVA::Reflection refModel = DAVA::Reflection::Create(&dataSource);

        {
            CheckBox::Params params(GetAccessor(), GetUI(), CheckBoxTestDetails::wndKey);
            params.fields[CheckBox::Fields::Checked] = "value";
            CheckBox* checkBox = new CheckBox(params, GetAccessor(), refModel);
            checkBox->SetObjectName("CheckBox_ReadOnlyMeta");
            layout->AddControl(checkBox);
        }

        {
            CheckBox::Params params(GetAccessor(), GetUI(), CheckBoxTestDetails::wndKey);
            params.fields[CheckBox::Fields::Checked] = "readOnlyValue";
            CheckBox* checkBox = new CheckBox(params, GetAccessor(), refModel);
            checkBox->SetObjectName("CheckBox_ReadOnly");
            layout->AddControl(checkBox);
        }

        {
            CheckBox::Params params(GetAccessor(), GetUI(), CheckBoxTestDetails::wndKey);
            params.fields[CheckBox::Fields::Checked] = "writableValue";
            params.fields[CheckBox::Fields::IsReadOnly] = "isReadOnly";
            params.fields[CheckBox::Fields::TextHint] = "writableDescription";
            CheckBox* checkBox = new CheckBox(params, GetAccessor(), refModel);
            checkBox->SetObjectName("CheckBox_writable");
            layout->AddControl(checkBox);
        }

        GetUI()->AddView(wndKey, DAVA::TArc::PanelKey("CheckBoxSandbox", DAVA::TArc::CentralPanelInfo()), w);
    }

    DAVA::Map<DAVA::String, DAVA::Any> model;
    CheckBoxDataSource dataSource;

    static CheckBoxTestModule* instance;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(CheckBoxTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<CheckBoxTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

class EnableEventListener : public QObject
{
public:
    EnableEventListener(QCheckBox* box_, const DAVA::Function<void(bool)>& callback_)
        : box(box_)
        , callback(callback_)
    {
        TEST_VERIFY(box->isEnabled() == true);
        box->installEventFilter(this);
    }

    bool eventFilter(QObject* obj, QEvent* e) override
    {
        if (obj == box && e->type() == QEvent::EnabledChange && active == true)
        {
            callback(box->isEnabled());
            active = false;
        }

        return false;
    }

private:
    bool active = true;
    QCheckBox* box;
    DAVA::Function<void(bool)> callback;
};

QPoint GetCheckboxCenter(QWidget* w)
{
    QStyle* style = w->style();
    QStyleOptionButton option;
    option.initFrom(w);
    QRect r = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option, w);

    return r.center();
}

void SimulateClickOnCheckBox(QWidget* w)
{
    QTestEventList eventList;
    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), GetCheckboxCenter(w));
    eventList.simulate(w);
}

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

        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        TEST_VERIFY(checkBox != nullptr);
        CheckBoxTestModule* inst = CheckBoxTestModule::instance;
        TEST_VERIFY(inst->model.find("bool") != inst->model.end());

        SimulateClickOnCheckBox(w);
        TEST_VERIFY(checkBox->isChecked() == false);
        TEST_VERIFY(inst->model["bool"].Cast<bool>() == false);

        SimulateClickOnCheckBox(w);
        TEST_VERIFY(checkBox->isChecked() == true);
        TEST_VERIFY(inst->model["bool"].Cast<bool>() == true);
    }

    DAVA_TEST (CheckStateTest)
    {
        using namespace CheckBoxTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("CheckBox_state"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        TEST_VERIFY(checkBox != nullptr);
        CheckBoxTestModule* inst = CheckBoxTestModule::instance;
        TEST_VERIFY(inst->model.find("checkState") != inst->model.end());

        SimulateClickOnCheckBox(w);
        TEST_VERIFY(checkBox->checkState() == Qt::Checked);
        TEST_VERIFY(inst->model["checkState"].Cast<Qt::CheckState>() == Qt::Checked);

        SimulateClickOnCheckBox(w);
        TEST_VERIFY(checkBox->checkState() == Qt::Unchecked);
        TEST_VERIFY(inst->model["checkState"].Cast<Qt::CheckState>() == Qt::Unchecked);

        SimulateClickOnCheckBox(w);
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

    void ReadOnlyMetaValueChanged(int newState)
    {
        QList<QWidget*> widgets = LookupWidget(CheckBoxTestDetails::wndKey, QString("CheckBox_ReadOnlyMeta"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);

        TEST_VERIFY(checkBox->isEnabled() == false);
        TEST_VERIFY(checkBox->isChecked() == false);
        TEST_VERIFY(checkBox->text() == QStringLiteral("False"));
    }

    DAVA_TEST (ReadOnlyMetaTest)
    {
        using namespace CheckBoxTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("CheckBox_ReadOnlyMeta"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        TEST_VERIFY(checkBox->isEnabled() == false);
        TEST_VERIFY(checkBox->isChecked() == true);
        TEST_VERIFY(checkBox->text() == QStringLiteral("True"));

        SimulateClickOnCheckBox(w);
        TEST_VERIFY(checkBox->isEnabled() == false);
        TEST_VERIFY(checkBox->isChecked() == true);
        TEST_VERIFY(checkBox->text() == QStringLiteral("True"));

        connections.AddConnection(checkBox, &QCheckBox::stateChanged, DAVA::MakeFunction(this, &CheckBoxTest::OnStateChanged));
        EXPECT_CALL(*this, OnStateChanged(_))
        .WillOnce(Invoke(this, &CheckBoxTest::ReadOnlyMetaValueChanged));
        CheckBoxTestModule::instance->dataSource.value = false;
    }

    DAVA_TEST (ReadOnlyTest)
    {
        using namespace CheckBoxTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("CheckBox_ReadOnly"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        TEST_VERIFY(checkBox->isEnabled() == false);
        TEST_VERIFY(checkBox->isChecked() == false);
        TEST_VERIFY(checkBox->text() == QStringLiteral(""));

        SimulateClickOnCheckBox(w);
        TEST_VERIFY(checkBox->isEnabled() == false);
        TEST_VERIFY(checkBox->isChecked() == false);
        TEST_VERIFY(checkBox->text() == QStringLiteral(""));
    }

    DAVA_TEST (WritableTest)
    {
        using namespace CheckBoxTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("CheckBox_writable"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
        TEST_VERIFY(checkBox->isEnabled() == true);
        TEST_VERIFY(checkBox->isChecked() == false);
        TEST_VERIFY(checkBox->text() == QStringLiteral("Invisible"));

        SimulateClickOnCheckBox(w);
        TEST_VERIFY(checkBox->isEnabled() == true);
        TEST_VERIFY(checkBox->isChecked() == true);

        eventListener.reset(new CheckBoxTestDetails::EnableEventListener(checkBox, DAVA::MakeFunction(this, &CheckBoxTest::OnEnableChange)));
        CheckBoxTestModule::instance->dataSource.isReadOnly = true;

        EXPECT_CALL(*this, OnEnableChange(false))
        .WillOnce(Return());
    }

    void Update(DAVA::float32 timeElapsed, const DAVA::String& testName) override
    {
        if (testName == "WritableTest")
        {
            writableTestUpdateCount++;
            TEST_VERIFY_WITH_MESSAGE(writableTestUpdateCount < 10, "WritableTest failed");
        }

        TestClass::Update(timeElapsed, testName);
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        bool testCompleted = true;
        if (testName == "WritableTest")
        {
            QList<QWidget*> widgets = LookupWidget(CheckBoxTestDetails::wndKey, QString("CheckBox_writable"));
            TEST_VERIFY(widgets.size() == 1);
            QWidget* w = widgets.front();
            QCheckBox* checkBox = qobject_cast<QCheckBox*>(w);
            testCompleted = (checkBox->text() == QStringLiteral("Visible"));
        }

        return testCompleted;
    }

    DAVA::int32 writableTestUpdateCount = 0;
    std::unique_ptr<CheckBoxTestDetails::EnableEventListener> eventListener;

    MOCK_METHOD1_VIRTUAL(OnStateChanged, void(int newState));
    MOCK_METHOD1_VIRTUAL(OnEnableChange, void(bool));

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(CheckBoxTestDetails::CheckBoxTestModule);
    END_TESTED_MODULES()

    DAVA::TArc::QtConnections connections;
};
