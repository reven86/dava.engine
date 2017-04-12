#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/ColorPicker/ColorPickerButton.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"

#include <QtTools/Utils/QtDelayedExecutor.h>

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include <Math/Color.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QtTest>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QApplication>

#include <memory>

namespace ColorPickerButtonTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("ColorPickerButtonTestWnd");

struct ColorPickerButtonDataSource
{
    ColorPickerButtonDataSource()
    {
        using namespace DAVA;
        colorRange.reset(new M::Range(Color(0.2f, 0.2f, 0.2f, 0.2f), Color(0.4f, 0.4f, 0.4f, 0.4f), Color(0.1f, 0.1f, 0.1f, 0.1f)));
    }

    DAVA::Color value;
    bool isReadOnly = false;

    const DAVA::Color& GetValue() const
    {
        return value;
    }

    void SetValue(const DAVA::Color& v)
    {
        value = v;
    }

    const DAVA::M::Range* GetRangeMeta() const
    {
        return colorRange.get();
    }
    std::shared_ptr<DAVA::M::Range> colorRange;

    DAVA_REFLECTION(ColorPickerButtonDataSource)
    {
        DAVA::ReflectionRegistrator<ColorPickerButtonDataSource>::Begin()
        .Field("value", &ColorPickerButtonDataSource::value)
        .Field("valueWithRange", &ColorPickerButtonDataSource::value)[DAVA::M::Range(DAVA::Color(0.2f, 0.2f, 0.2f, 0.2f), DAVA::Color(0.4f, 0.4f, 0.4f, 0.4f), DAVA::Color(0.1f, 0.1f, 0.1f, 0.1f))]
        .Field("range", &ColorPickerButtonDataSource::GetRangeMeta, nullptr)
        .Field("readOnlyValue", &ColorPickerButtonDataSource::value)[DAVA::M::ReadOnly()]
        .Field("writableValue", &ColorPickerButtonDataSource::GetValue, &ColorPickerButtonDataSource::SetValue)
        .Field("isReadOnly", &ColorPickerButtonDataSource::isReadOnly)
        .End();
    }
};

class ColorPickerButtonTestModule : public DAVA::TArc::ClientModule
{
public:
    ColorPickerButtonTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        DAVA::Reflection reflectedModel = DAVA::Reflection::Create(&dataSource);

        {
            ColorPickerButton::Params params;
            params.ui = GetUI();
            params.wndKey = ColorPickerButtonTestDetails::wndKey;
            params.accessor = GetAccessor();
            params.fields[ColorPickerButton::Fields::Color] = "value";
            params.fields[ColorPickerButton::Fields::IsReadOnly] = "isReadOnly";

            ColorPickerButton* button = new DAVA::TArc::ColorPickerButton(params, GetAccessor(), reflectedModel);
            button->SetObjectName("ColorPickerButton_value_readonly");
            layout->AddControl(button);
        }

        {
            ColorPickerButton::Params params;
            params.ui = GetUI();
            params.wndKey = ColorPickerButtonTestDetails::wndKey;
            params.accessor = GetAccessor();
            params.fields[ColorPickerButton::Fields::Color] = "readOnlyValue";

            ColorPickerButton* button = new DAVA::TArc::ColorPickerButton(params, GetAccessor(), reflectedModel);
            button->SetObjectName("ColorPickerButton_readOnlyValue");
            layout->AddControl(button);
        }

        {
            ColorPickerButton::Params params;
            params.ui = GetUI();
            params.wndKey = ColorPickerButtonTestDetails::wndKey;
            params.accessor = GetAccessor();
            params.fields[ColorPickerButton::Fields::Color] = "writableValue";

            ColorPickerButton* button = new DAVA::TArc::ColorPickerButton(params, GetAccessor(), reflectedModel);
            button->SetObjectName("ColorPickerButton_writableValue");
            layout->AddControl(button);
        }

        {
            ColorPickerButton::Params params;
            params.ui = GetUI();
            params.wndKey = ColorPickerButtonTestDetails::wndKey;
            params.accessor = GetAccessor();
            params.fields[ColorPickerButton::Fields::Color] = "valueWithRange";

            ColorPickerButton* button = new DAVA::TArc::ColorPickerButton(params, GetAccessor(), reflectedModel);
            button->SetObjectName("ColorPickerButton_valueWithRange");
            layout->AddControl(button);
        }

        {
            ColorPickerButton::Params params;
            params.ui = GetUI();
            params.wndKey = ColorPickerButtonTestDetails::wndKey;
            params.accessor = GetAccessor();
            params.fields[ColorPickerButton::Fields::Color] = "value";
            params.fields[ColorPickerButton::Fields::Range] = "range";

            ColorPickerButton* button = new DAVA::TArc::ColorPickerButton(params, GetAccessor(), reflectedModel);
            button->SetObjectName("ColorPickerButton_range");
            layout->AddControl(button);
        }

        GetUI()->AddView(wndKey, DAVA::TArc::PanelKey("ColorPickerButtonSandbox", DAVA::TArc::CentralPanelInfo()), w);
    }

    ColorPickerButtonDataSource dataSource;

    static ColorPickerButtonTestModule* instance;
    static DAVA::Color initialColor;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ColorPickerButtonTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<ColorPickerButtonTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

ColorPickerButtonTestModule* ColorPickerButtonTestModule::instance = nullptr;
DAVA::Color ColorPickerButtonTestModule::initialColor(1.0f, 0.0f, 0.0f, 1.0f);
}

DAVA_TARC_TESTCLASS(ColorPickerButtonTest)
{
    QToolButton* GetButton(const QString& name)
    {
        QList<QWidget*> widgets = LookupWidget(ColorPickerButtonTestDetails::wndKey, name);
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QToolButton* button = qobject_cast<QToolButton*>(w);
        TEST_VERIFY(button != nullptr);
        return button;
    }

    struct TestData
    {
        QString controlName;
        DAVA::String testName;
        bool finished = false;
        DAVA::Function<void()> finishFn;
    } currentTestData;

    void WritableValueTestStart()
    {
        using namespace DAVA::TArc;
        using namespace ColorPickerButtonTestDetails;

        delayedExecutor.DelayedExecute(DAVA::MakeFunction(this, &ColorPickerButtonTest::WritableValueTestAnimationSkip));

        QToolButton* button = GetButton(currentTestData.controlName);
        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers());
        eventList.simulate(button);
    }

    void WritableValueTestAnimationSkip()
    {
        qApp->processEvents();
        delayedExecutor.DelayedExecute(DAVA::MakeFunction(this, &ColorPickerButtonTest::WritableValueTestSimulateDialog));
    }

    void WritableValueTestSimulateDialog()
    {
        using namespace DAVA::TArc;
        using namespace ColorPickerButtonTestDetails;
        using namespace ::testing;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;

        QWidget* colorPickerDialog = QApplication::activeModalWidget();
        TEST_VERIFY(colorPickerDialog->objectName() == "ColorPickerDialog");

        QList<QWidget*> colorPaletteList = colorPickerDialog->findChildren<QWidget*>("ColorPickerRGBAM");
        TEST_VERIFY(colorPaletteList.size() == 1);
        QWidget* colorPaletteWidget = colorPaletteList.front();

        QList<QWidget*> okButtonList = colorPickerDialog->findChildren<QWidget*>("ok");
        TEST_VERIFY(okButtonList.size() == 1);
        QWidget* okButtonWidget = okButtonList.front();

        TEST_VERIFY(inst->dataSource.value == ColorPickerButtonTestModule::initialColor);

        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers());
        eventList.simulate(colorPaletteWidget);
        eventList.simulate(okButtonWidget);

        delayedExecutor.DelayedExecute(currentTestData.finishFn);
    }

    void WritableValueTestFinish()
    {
        using namespace ColorPickerButtonTestDetails;
        using namespace ::testing;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke([this, inst]() {
            if (currentTestData.finished == false)
            {
                TEST_VERIFY(inst->dataSource.value != ColorPickerButtonTestModule::initialColor);
                currentTestData.finished = true;
            }
        }));
    }

    DAVA_TEST (ValueWritableTest)
    {
        using namespace ColorPickerButtonTestDetails;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;
        inst->dataSource.value = ColorPickerButtonTestModule::initialColor;

        currentTestData.controlName = "ColorPickerButton_value_readonly";
        currentTestData.testName = "ValueWritableTest";
        currentTestData.finished = false;
        currentTestData.finishFn = DAVA::MakeFunction(this, &ColorPickerButtonTest::WritableValueTestFinish);

        WritableValueTestStart();
    }

    DAVA_TEST (MethodWritableTest)
    {
        using namespace ColorPickerButtonTestDetails;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;
        inst->dataSource.value = ColorPickerButtonTestModule::initialColor;

        currentTestData.controlName = "ColorPickerButton_writableValue";
        currentTestData.testName = "MethodWritableTest";
        currentTestData.finished = false;
        currentTestData.finishFn = DAVA::MakeFunction(this, &ColorPickerButtonTest::WritableValueTestFinish);

        WritableValueTestStart();
    }

    void RangeTestFinish()
    {
        using namespace ColorPickerButtonTestDetails;
        using namespace ::testing;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke([this, inst]() {
            if (currentTestData.finished == false)
            {
                TEST_VERIFY(inst->dataSource.value != ColorPickerButtonTestModule::initialColor);

                const DAVA::M::Range* colorRange = inst->dataSource.GetRangeMeta();
                TEST_VERIFY(colorRange != nullptr);

                DAVA::Color minV = colorRange->minValue.Get<DAVA::Color>();
                DAVA::Color maxV = colorRange->maxValue.Get<DAVA::Color>();

                TEST_VERIFY(inst->dataSource.value.r >= minV.r && inst->dataSource.value.r <= maxV.r);
                TEST_VERIFY(inst->dataSource.value.g >= minV.g && inst->dataSource.value.g <= maxV.g);
                TEST_VERIFY(inst->dataSource.value.b >= minV.b && inst->dataSource.value.b <= maxV.b);
                TEST_VERIFY(inst->dataSource.value.a >= minV.a && inst->dataSource.value.a <= maxV.a);

                currentTestData.finished = true;
            }
        }));
    }

    DAVA_TEST (RangeMetaTest)
    {
        using namespace ColorPickerButtonTestDetails;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;
        inst->dataSource.value = ColorPickerButtonTestModule::initialColor;

        currentTestData.controlName = "ColorPickerButton_valueWithRange";
        currentTestData.testName = "RangeMetaTest";
        currentTestData.finished = false;
        currentTestData.finishFn = DAVA::MakeFunction(this, &ColorPickerButtonTest::RangeTestFinish);

        WritableValueTestStart();
    }

    DAVA_TEST (RangeValueTest)
    {
        using namespace ColorPickerButtonTestDetails;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;
        inst->dataSource.value = ColorPickerButtonTestModule::initialColor;

        currentTestData.controlName = "ColorPickerButton_range";
        currentTestData.testName = "RangeValueTest";
        currentTestData.finished = false;
        currentTestData.finishFn = DAVA::MakeFunction(this, &ColorPickerButtonTest::RangeTestFinish);

        WritableValueTestStart();
    }

    void ReadOnlyTest()
    {
        using namespace DAVA::TArc;
        using namespace ColorPickerButtonTestDetails;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;
        TEST_VERIFY(inst->dataSource.value == ColorPickerButtonTestModule::initialColor);

        QToolButton* button = GetButton(currentTestData.controlName);
        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers());
        eventList.simulate(button);

        TEST_VERIFY(inst->dataSource.value == ColorPickerButtonTestModule::initialColor);
        currentTestData.finished = true;
    }

    DAVA_TEST (ReadOnlyFieldValueTest)
    {
        using namespace ColorPickerButtonTestDetails;
        using namespace ::testing;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;
        inst->dataSource.value = ColorPickerButtonTestModule::initialColor;
        inst->dataSource.isReadOnly = true;

        currentTestData.controlName = "ColorPickerButton_value_readonly";
        currentTestData.testName = "ReadOnlyFieldValueTest";
        currentTestData.finished = false;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke([this]() {
            ReadOnlyTest();
        }));
    }

    DAVA_TEST (ReadOnlyValueTest)
    {
        using namespace ColorPickerButtonTestDetails;
        using namespace ::testing;

        ColorPickerButtonTestModule* inst = ColorPickerButtonTestModule::instance;
        inst->dataSource.value = ColorPickerButtonTestModule::initialColor;
        inst->dataSource.isReadOnly = false;

        currentTestData.controlName = "ColorPickerButton_readOnlyValue";
        currentTestData.testName = "ReadOnlyValueTest";
        currentTestData.finished = false;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke([this]() {
            ReadOnlyTest();
        }));
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        bool testCompleted = true;
        if (testName == currentTestData.testName)
        {
            testCompleted = currentTestData.finished;
        }

        return testCompleted;
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ColorPickerButtonTestDetails::ColorPickerButtonTestModule);
    END_TESTED_MODULES()

    DAVA::TArc::QtConnections connections;
    QtDelayedExecutor delayedExecutor;
};
