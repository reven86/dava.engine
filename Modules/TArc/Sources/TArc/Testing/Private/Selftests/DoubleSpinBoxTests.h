#include "TArc/Testing/MockDefine.h"
#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"

#include "TArc/Controls/DoubleSpinBox.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/Testing/Private/TestModuleHolder.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QDoubleSpinBox>
#include <QtTest>

namespace DoubleSpinBoxTestDetails
{
using namespace DAVA;
using namespace DAVA::TArc;

WindowKey wndKey = FastName("DoubleSpinBoxWindow");

class DoubleSpinBoxData : public ReflectionBase
{
public:
    DoubleSpinBoxData()
    {
        rangeMeta.reset(new M::Range(3.0, 30.0, 0.2));
    }

    float64 value = 10.0;
    bool isEnabled = true;
    bool isReadOnly = false;

    float64 GetValue() const
    {
        return value;
    }

    void SetValue(float64 v)
    {
        value = v;
    }

    Any GetNoValue() const
    {
        if (value == 10.0)
            return Any();

        return value;
    }

    Any GetHintValue() const
    {
        if (value == 10.0)
            return String("no value");

        return value;
    }

    void SetAnyValue(const Any& v)
    {
        value = v.Cast<float64>();
    }

    const M::Range* GetRangeMeta() const
    {
        return rangeMeta.get();
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DoubleSpinBoxData)
    {
        ReflectionRegistrator<DoubleSpinBoxData>::Begin()
        .Field("value", &DoubleSpinBoxData::GetValue, &DoubleSpinBoxData::SetValue)[M::Range(3.0, 30.0, 0.2), M::FloatNumberAccuracy(5)]
        .Field("noRangeValue", &DoubleSpinBoxData::GetValue, &DoubleSpinBoxData::SetValue)
        .Field("readOnlyValue", &DoubleSpinBoxData::GetValue, nullptr)
        .Field("readOnlyMeta", &DoubleSpinBoxData::value)[M::ReadOnly()]
        .Field("noValue", &DoubleSpinBoxData::GetNoValue, &DoubleSpinBoxData::SetAnyValue)
        .Field("noValueHint", &DoubleSpinBoxData::GetHintValue, &DoubleSpinBoxData::SetAnyValue)
        .Field("rangeMeta", &DoubleSpinBoxData::GetRangeMeta, nullptr)
        .Field("isEnabled", &DoubleSpinBoxData::isEnabled)
        .Field("isReadOnly", &DoubleSpinBoxData::isReadOnly)
        .End();
    }

    std::shared_ptr<M::Range> rangeMeta;
};

class DoubleSpinBoxTestModule : public ClientModule
{
public:
    DoubleSpinBoxTestModule()
        : holder(this)
    {
    }

    void PostInit() override
    {
        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        Reflection ref = Reflection::Create(&model);

        {
            DoubleSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[DoubleSpinBox::Fields::Value] = "value";
            DoubleSpinBox* edit = new DoubleSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_value");
            layout->AddControl(edit);
        }

        {
            DoubleSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[DoubleSpinBox::Fields::Value] = "noRangeValue";
            params.fields[DoubleSpinBox::Fields::Range] = "rangeMeta";
            DoubleSpinBox* edit = new DoubleSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_metaRangeValue");
            layout->AddControl(edit);
        }

        {
            DoubleSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[DoubleSpinBox::Fields::Value] = "readOnlyValue";
            DoubleSpinBox* edit = new DoubleSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_readOnlyValue");
            layout->AddControl(edit);
        }

        {
            DoubleSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[DoubleSpinBox::Fields::Value] = "readOnlyMeta";
            DoubleSpinBox* edit = new DoubleSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_readOnlyMeta");
            layout->AddControl(edit);
        }

        {
            DoubleSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[DoubleSpinBox::Fields::Value] = "value";
            params.fields[DoubleSpinBox::Fields::IsReadOnly] = "isReadOnly";
            DoubleSpinBox* edit = new DoubleSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_readOnlyField");
            layout->AddControl(edit);
        }

        {
            DoubleSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[DoubleSpinBox::Fields::Value] = "value";
            params.fields[DoubleSpinBox::Fields::IsEnabled] = "isEnabled";
            DoubleSpinBox* edit = new DoubleSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_enable");
            layout->AddControl(edit);
        }

        {
            DoubleSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[DoubleSpinBox::Fields::Value] = "noValue";
            DoubleSpinBox* edit = new DoubleSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_noValue");
            layout->AddControl(edit);
        }

        {
            DoubleSpinBox::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[DoubleSpinBox::Fields::Value] = "noValueHint";
            DoubleSpinBox* edit = new DoubleSpinBox(params, GetAccessor(), ref);
            edit->SetObjectName("SpinBox_noValuehint");
            layout->AddControl(edit);
        }

        GetUI()->AddView(wndKey, PanelKey("DoubleSpinBoxView", CentralPanelInfo()), w);
    }

    DoubleSpinBoxData model;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DoubleSpinBoxTestModule, ClientModule)
    {
        ReflectionRegistrator<DoubleSpinBoxTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

private:
    TestModuleHolder<DoubleSpinBoxTestModule> holder;
};

using Holder = TestModuleHolder<DoubleSpinBoxTestModule>;
}

DAVA_TARC_TESTCLASS(DoubleSpinBoxTests)
{
    void SetFocus(QDoubleSpinBox * box)
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

    void ButtonClick(QTestEventList & list, bool isUpButton, QDoubleSpinBox* box)
    {
        QStyleOptionSpinBox opt;
        opt.initFrom(box);

        QStyle* style = box->style();
        QRect r = style->subControlRect(QStyle::CC_SpinBox, &opt, (isUpButton == true) ? QStyle::SC_SpinBoxUp : QStyle::SC_SpinBoxDown, box);
        list.addMouseClick(Qt::LeftButton, Qt::KeyboardModifier(), r.center());
    }

    void RangedValueChangeTest(const QString& name)
    {
        using namespace DoubleSpinBoxTestDetails;
        QDoubleSpinBox* box = LookupSingleWidget<QDoubleSpinBox>(wndKey, name);
        DoubleSpinBoxTestModule* module = Holder::moduleInstance;
        TEST_VERIFY(box->value() == module->model.value);
        TEST_VERIFY(box->text() == QString::number(module->model.value, 'f'));
        TEST_VERIFY(box->minimum() == 3.0);
        TEST_VERIFY(box->maximum() == 30.0);
        TEST_VERIFY(box->singleStep() == 0.2);

        // simulate spin value lower than minimum
        QTestEventList events;
        SetFocus(box);
        for (int i = 0; i < 40; ++i) // 10 (current value) - 40 * 0.2 (single step value) == 2
            ButtonClick(events, false, box);
        events.simulate(box);
        TEST_VERIFY(box->value() == 3.0); //  but lower bound is 3
        TEST_VERIFY(QString::number(box->value(), 'f') == QString::number(module->model.value, 'f'));

        events.clear();
        ButtonClick(events, true, box);
        events.simulate(box);

        TEST_VERIFY(box->value() == 3.2);
        TEST_VERIFY(QString::number(box->value(), 'f') == QString::number(module->model.value, 'f'));

        events.clear();
        events.addKeyClick(Qt::Key_Delete);
        events.addKeyClicks("-8"); // out of Range, so validator will not pass minus, but pass 8
        events.addKeyClick(Qt::Key_Return);
        events.simulate(box);
        TEST_VERIFY(box->value() == 8.0);
        TEST_VERIFY(QString::number(box->value(), 'f') == QString::number(module->model.value, 'f'));

        events.clear();
        events.addKeyClick(Qt::Key_Delete);
        events.addKeyClicks("24.5");
        events.addKeyClick(Qt::Key_Return);
        events.simulate(box);
        TEST_VERIFY(box->value() == 24.5);
        TEST_VERIFY(QString::number(box->value(), 'f') == QString::number(module->model.value, 'f'));

        module->model.value = 10.0;
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
        using namespace DoubleSpinBoxTestDetails;
        DoubleSpinBoxTestModule* module = Holder::moduleInstance;
        float64 readOnlyValue = module->model.value;

        QDoubleSpinBox* box = LookupSingleWidget<QDoubleSpinBox>(wndKey, spinName);

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
        DoubleSpinBoxTestDetails::Holder::moduleInstance->model.isReadOnly = true;
    }

    DAVA_TEST (ReadOnlyFieldTrueTest)
    {
        ReadOnlyTest("SpinBox_readOnlyField");
        DoubleSpinBoxTestDetails::Holder::moduleInstance->model.isReadOnly = false;
    }

    DAVA_TEST (EnabledSpinBoxTest)
    {
        RangedValueChangeTest("SpinBox_enable");
        DoubleSpinBoxTestDetails::Holder::moduleInstance->model.isEnabled = false;
    }

    DAVA_TEST (DisabledSpinBoxTest)
    {
        ReadOnlyTest("SpinBox_enable");
        DoubleSpinBoxTestDetails::Holder::moduleInstance->model.isEnabled = true;
    }

    void NoValueTest(const QString& name, const QString& focusSpinName, const QString& hintText)
    {
        using namespace DoubleSpinBoxTestDetails;
        QDoubleSpinBox* focusSpin = LookupSingleWidget<QDoubleSpinBox>(wndKey, focusSpinName);
        SetFocus(focusSpin);

        QDoubleSpinBox* box = LookupSingleWidget<QDoubleSpinBox>(wndKey, name);
        TEST_VERIFY(box->text() == hintText);

        DoubleSpinBoxTestModule* module = Holder::moduleInstance;
        float64 value = module->model.value;

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

        TEST_VERIFY(box->value() == 18.0);
        TEST_VERIFY(box->value() == module->model.value);
        TEST_VERIFY(box->text() == QString::number(module->model.value, 'f'));

        events.clear();
        SetFocus(box);
        ButtonClick(events, false, box);
        events.simulate(box);
        TEST_VERIFY(box->value() == 17.0);
        TEST_VERIFY(box->value() == module->model.value);
        TEST_VERIFY(box->text() == QString::number(module->model.value, 'f'));

        module->model.value = 10.0;
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
        using namespace DoubleSpinBoxTestDetails;
        QDoubleSpinBox* valueSpin = LookupSingleWidget<QDoubleSpinBox>(wndKey, "SpinBox_value");
        QDoubleSpinBox* metaRangeSpin = LookupSingleWidget<QDoubleSpinBox>(wndKey, "SpinBox_metaRangeValue");

        TEST_VERIFY(valueSpin->value() == 16.0);
        TEST_VERIFY(valueSpin->minimum() == 3.0);
        TEST_VERIFY(valueSpin->maximum() == 30.0);
        TEST_VERIFY(valueSpin->singleStep() == 0.2);

        TEST_VERIFY(valueSpin->value() == metaRangeSpin->value());
        TEST_VERIFY(metaRangeSpin->minimum() == 10.0);
        TEST_VERIFY(metaRangeSpin->maximum() == 100.0);
        TEST_VERIFY(metaRangeSpin->singleStep() == 0.4);

        QTestEventList events;
        SetFocus(metaRangeSpin);
        ButtonClick(events, true, metaRangeSpin);
        events.simulate(metaRangeSpin);
        TEST_VERIFY(metaRangeSpin->value() == 16.4);
        TEST_VERIFY(metaRangeSpin->value() == Holder::moduleInstance->model.value);
    }

    DAVA_TEST (ValueSyncTest)
    {
        using namespace ::testing;
        using namespace DoubleSpinBoxTestDetails;
        Holder::moduleInstance->model.value = 16.0;
        Holder::moduleInstance->model.rangeMeta.reset(new M::Range(10.0, 100.0, 0.4));

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &DoubleSpinBoxTests::AfterSyncTest));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(DoubleSpinBoxTestDetails::DoubleSpinBoxTestModule);
    END_TESTED_MODULES()
};