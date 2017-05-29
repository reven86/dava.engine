#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Testing/Private/TestModuleHolder.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>

#include <QSpinBox>
#include <QtTest>

namespace IntSpinBoxTestDetails
{
using namespace DAVA;
using namespace DAVA::TArc;

WindowKey wndKey = FastName("IntSpinBoxWindow");

class IntSpinBoxData : public ReflectionBase
{
public:
    IntSpinBoxData()
    {
        rangeMeta.reset(new M::Range(3, 30, 2));
    }

    int32 value = 10;
    bool isEnabled = true;
    bool isReadOnly = false;

    int32 GetValue() const
    {
        return value;
    }

    void SetValue(int32 v)
    {
        value = v;
    }

    Any GetNoValue() const
    {
        if (value == 10)
            return Any();

        return value;
    }

    Any GetHintValue() const
    {
        if (value == 10)
            return String("no value");

        return value;
    }

    void SetAnyValue(const Any& v)
    {
        value = v.Cast<int32>();
    }

    const M::Range* GetRangeMeta() const
    {
        return rangeMeta.get();
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(IntSpinBoxData)
    {
        ReflectionRegistrator<IntSpinBoxData>::Begin()
        .Field("value", &IntSpinBoxData::GetValue, &IntSpinBoxData::SetValue)[M::Range(3, 30, 2)]
        .Field("noRangeValue", &IntSpinBoxData::GetValue, &IntSpinBoxData::SetValue)
        .Field("readOnlyValue", &IntSpinBoxData::GetValue, nullptr)
        .Field("readOnlyMeta", &IntSpinBoxData::value)[M::ReadOnly()]
        .Field("noValue", &IntSpinBoxData::GetNoValue, &IntSpinBoxData::SetAnyValue)
        .Field("noValueHint", &IntSpinBoxData::GetHintValue, &IntSpinBoxData::SetAnyValue)
        .Field("rangeMeta", &IntSpinBoxData::GetRangeMeta, nullptr)
        .Field("isEnabled", &IntSpinBoxData::isEnabled)
        .Field("isReadOnly", &IntSpinBoxData::isReadOnly)
        .End();
    }

    std::shared_ptr<M::Range> rangeMeta;
};

class IntSpinBoxTestModule : public ClientModule
{
public:
    IntSpinBoxTestModule()
        : holder(this)
    {
    }

    void PostInit() override
    {
        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        Reflection ref = Reflection::Create(&model);

        {
            IntSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[IntSpinBox::Fields::Value] = "value";
            IntSpinBox* edit = new IntSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_value");
            layout->AddControl(edit);
        }

        {
            IntSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[IntSpinBox::Fields::Value] = "noRangeValue";
            params.fields[IntSpinBox::Fields::Range] = "rangeMeta";
            IntSpinBox* edit = new IntSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_metaRangeValue");
            layout->AddControl(edit);
        }

        {
            IntSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[IntSpinBox::Fields::Value] = "readOnlyValue";
            IntSpinBox* edit = new IntSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_readOnlyValue");
            layout->AddControl(edit);
        }

        {
            IntSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[IntSpinBox::Fields::Value] = "readOnlyMeta";
            IntSpinBox* edit = new IntSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_readOnlyMeta");
            layout->AddControl(edit);
        }

        {
            IntSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[IntSpinBox::Fields::Value] = "value";
            params.fields[IntSpinBox::Fields::IsReadOnly] = "isReadOnly";
            IntSpinBox* edit = new IntSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_readOnlyField");
            layout->AddControl(edit);
        }

        {
            IntSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[IntSpinBox::Fields::Value] = "value";
            params.fields[IntSpinBox::Fields::IsEnabled] = "isEnabled";
            IntSpinBox* edit = new IntSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_enable");
            layout->AddControl(edit);
        }

        {
            IntSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[IntSpinBox::Fields::Value] = "noValue";
            IntSpinBox* edit = new IntSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_noValue");
            layout->AddControl(edit);
        }

        {
            IntSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[IntSpinBox::Fields::Value] = "noValueHint";
            IntSpinBox* edit = new IntSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_noValuehint");
            layout->AddControl(edit);
        }

        GetUI()->AddView(wndKey, PanelKey("IntSpinBoxView", CentralPanelInfo()), w);
    }

    IntSpinBoxData model;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(IntSpinBoxTestModule, ClientModule)
    {
        ReflectionRegistrator<IntSpinBoxTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

private:
    TestModuleHolder<IntSpinBoxTestModule> holder;
};

using Holder = TestModuleHolder<IntSpinBoxTestModule>;
}

DAVA_TARC_TESTCLASS(IntSpinBoxTests)
{
    void SetFocus(QSpinBox * box)
    {
        box->clearFocus();
        if (box->isActiveWindow() == false)
        {
            QWidget* topLevel = box;
            while (topLevel->parent() != nullptr)
                topLevel = topLevel->parentWidget();

            qApp->setActiveWindow(topLevel);
        }
        box->setFocus(Qt::MouseFocusReason);
    }

    void ButtonClick(QTestEventList & list, bool isUpButton, QSpinBox* box)
    {
        QStyleOptionSpinBox opt;
        opt.initFrom(box);

        QStyle* style = box->style();
        QRect r = style->subControlRect(QStyle::CC_SpinBox, &opt, (isUpButton == true) ? QStyle::SC_SpinBoxUp : QStyle::SC_SpinBoxDown, box);
        list.addMouseClick(Qt::LeftButton, Qt::KeyboardModifier(), r.center());
    }

    void RangedValueChangeTest(const QString& name)
    {
        using namespace IntSpinBoxTestDetails;
        QSpinBox* box = LookupSingleWidget<QSpinBox>(wndKey, name);
        IntSpinBoxTestModule* module = Holder::moduleInstance;
        TEST_VERIFY(box->value() == module->model.value);
        TEST_VERIFY(box->text() == QString::number(module->model.value));
        TEST_VERIFY(box->minimum() == 3);
        TEST_VERIFY(box->maximum() == 30);
        TEST_VERIFY(box->singleStep() == 2);

        // simulate spin value lower than minimum
        QTestEventList events;
        SetFocus(box);
        for (int i = 0; i < 4; ++i) // 10 (current value) - 4 * 2 (single step value) == 2
            ButtonClick(events, false, box);
        events.simulate(box);
        TEST_VERIFY(box->value() == 3); //  but lower bound is 3
        TEST_VERIFY(box->value() == module->model.value);
        TEST_VERIFY(box->text() == QString::number(module->model.value));

        events.clear();
        ButtonClick(events, true, box);
        events.simulate(box);

        TEST_VERIFY(box->value() == 5); // we made one click on ButtonUp and single step == 2, so value should be 5
        TEST_VERIFY(box->value() == module->model.value);
        TEST_VERIFY(box->text() == QString::number(module->model.value));

        events.clear();
        events.addKeyClick(Qt::Key_Delete);
        events.addKeyClicks("-8"); // out of Range, so validator will not pass minus, but pass 8
        events.addKeyClick(Qt::Key_Return);
        events.simulate(box);
        TEST_VERIFY(box->value() == 8);
        TEST_VERIFY(box->value() == module->model.value);

        events.clear();
        events.addKeyClick(Qt::Key_Delete);
        events.addKeyClicks("24");
        events.addKeyClick(Qt::Key_Return);
        events.simulate(box);
        TEST_VERIFY(box->value() == 24);
        TEST_VERIFY(box->value() == module->model.value);

        module->model.value = 10;
    }

    DAVA_TEST (RangedValueFieldTest)
    {
        RangedValueChangeTest("SpinBox_value");
    }

    DAVA_TEST (RangedMetaValueFieldTest)
    {
        RangedValueChangeTest("SpinBox_metaRangeValue");
    }

    void ReadOnlyTest(const QString& spinName)
    {
        using namespace IntSpinBoxTestDetails;
        IntSpinBoxTestModule* module = Holder::moduleInstance;
        int32 readOnlyValue = module->model.value;

        QSpinBox* box = LookupSingleWidget<QSpinBox>(wndKey, spinName);

        QTestEventList events;
        SetFocus(box);
        ButtonClick(events, false, box);
        events.simulate(box);
        TEST_VERIFY(box->value() == readOnlyValue);
        TEST_VERIFY(box->value() == module->model.value);

        events.clear();
        SetFocus(box);
        ButtonClick(events, true, box);
        events.simulate(box);
        TEST_VERIFY(box->value() == readOnlyValue);
        TEST_VERIFY(box->value() == module->model.value);

        events.clear();
        SetFocus(box);
        events.addKeyClick(Qt::Key_Delete);
        events.addKeyClicks("24");
        events.addKeyClick(Qt::Key_Return);
        events.simulate(box);
        TEST_VERIFY(box->value() == readOnlyValue);
        TEST_VERIFY(box->value() == module->model.value);
    }

    DAVA_TEST (ReadOnlySetterTest)
    {
        ReadOnlyTest("SpinBox_readOnlyValue");
    }

    DAVA_TEST (ReadOnlyMetaTest)
    {
        ReadOnlyTest("SpinBox_readOnlyMeta");
    }

    DAVA_TEST (ReadOnlyFieldFalseTest)
    {
        RangedValueChangeTest("SpinBox_readOnlyField");
        IntSpinBoxTestDetails::Holder::moduleInstance->model.isReadOnly = true;
    }

    DAVA_TEST (ReadOnlyFieldTrueTest)
    {
        ReadOnlyTest("SpinBox_readOnlyField");
        IntSpinBoxTestDetails::Holder::moduleInstance->model.isReadOnly = false;
    }

    DAVA_TEST (EnabledSpinBoxTest)
    {
        RangedValueChangeTest("SpinBox_enable");
        IntSpinBoxTestDetails::Holder::moduleInstance->model.isEnabled = false;
    }

    DAVA_TEST (DisabledSpinBoxTest)
    {
        ReadOnlyTest("SpinBox_enable");
        IntSpinBoxTestDetails::Holder::moduleInstance->model.isEnabled = true;
    }

    void NoValueTest(const QString& name, const QString& focusSpinName, const QString& hintText)
    {
        using namespace IntSpinBoxTestDetails;
        QSpinBox* focusSpin = LookupSingleWidget<QSpinBox>(wndKey, focusSpinName);
        SetFocus(focusSpin);

        QSpinBox* box = LookupSingleWidget<QSpinBox>(wndKey, name);
        TEST_VERIFY(box->text() == hintText);

        IntSpinBoxTestModule* module = Holder::moduleInstance;
        uint32 value = module->model.value;

        SetFocus(box);
        TEST_VERIFY(box->text() == "");
        TEST_VERIFY(value == module->model.value);

        SetFocus(focusSpin);
        TEST_VERIFY(box->text() == hintText);
        TEST_VERIFY(value == module->model.value);

        QTestEventList events;
        SetFocus(box);
        ButtonClick(events, true, box);
        events.simulate(box);
        TEST_VERIFY(box->text() == "");
        TEST_VERIFY(value == module->model.value);

        events.clear();
        SetFocus(box);
        events.addKeyClicks("18");
        events.addKeyClick(Qt::Key_Enter);
        events.simulate(box);

        TEST_VERIFY(box->value() == 18);
        TEST_VERIFY(box->value() == module->model.value);
        TEST_VERIFY(box->text() == QString::number(module->model.value));

        events.clear();
        SetFocus(box);
        ButtonClick(events, false, box);
        events.simulate(box);
        TEST_VERIFY(box->value() == 17);
        TEST_VERIFY(box->value() == module->model.value);
        TEST_VERIFY(box->text() == QString::number(module->model.value));

        module->model.value = 10;
    }

    DAVA_TEST (NoValueTest)
    {
        NoValueTest("SpinBox_noValue", "SpinBox_value", QString(DAVA::TArc::MultipleValuesString));
    }

    DAVA_TEST (NoValueHintTest)
    {
        NoValueTest("SpinBox_noValuehint", "SpinBox_value", "no value");
    }

    void AfterSyncTest()
    {
        using namespace IntSpinBoxTestDetails;
        QSpinBox* valueSpin = LookupSingleWidget<QSpinBox>(wndKey, "SpinBox_value");
        QSpinBox* metaRangeSpin = LookupSingleWidget<QSpinBox>(wndKey, "SpinBox_metaRangeValue");

        TEST_VERIFY(valueSpin->value() == 16);
        TEST_VERIFY(valueSpin->minimum() == 3);
        TEST_VERIFY(valueSpin->maximum() == 30);
        TEST_VERIFY(valueSpin->singleStep() == 2);

        TEST_VERIFY(valueSpin->value() == metaRangeSpin->value());
        TEST_VERIFY(metaRangeSpin->minimum() == 10);
        TEST_VERIFY(metaRangeSpin->maximum() == 100);
        TEST_VERIFY(metaRangeSpin->singleStep() == 4);

        QTestEventList events;
        SetFocus(metaRangeSpin);
        ButtonClick(events, true, metaRangeSpin);
        events.simulate(metaRangeSpin);
        TEST_VERIFY(metaRangeSpin->value() == 20);
        TEST_VERIFY(metaRangeSpin->value() == Holder::moduleInstance->model.value);
    }

    DAVA_TEST (ValueSyncTest)
    {
        using namespace ::testing;
        using namespace IntSpinBoxTestDetails;
        Holder::moduleInstance->model.value = 16;
        Holder::moduleInstance->model.rangeMeta.reset(new M::Range(10, 100, 4));

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &IntSpinBoxTests::AfterSyncTest));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(IntSpinBoxTestDetails::IntSpinBoxTestModule);
    END_TESTED_MODULES()
};