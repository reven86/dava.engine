#include "Classes/DevFuncs/TestUIModule.h"
#include "Classes/DevFuncs/TestUIModuleData.h"
#include "Classes/Application/REGlobal.h"

#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/ComboBoxCheckable.h>
#include <TArc/Controls/IntSpinBox.h>
#include <TArc/Controls/DoubleSpinBox.h>
#include <TArc/Controls/IntSpinBox.h>
#include <TArc/Controls/LineEdit.h>
#include <TArc/Controls/PlainTextEdit.h>
#include <TArc/Controls/FilePathEdit.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/SubPropertiesEditor.h>
#include <TArc/Controls/PropertyPanel/Private/MultiDoubleSpinBox.h>
#include <TArc/Controls/ColorPicker/ColorPickerButton.h>
#include <TArc/Controls/Label.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Qt/QtString.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>

#include <Logger/Logger.h>
#include <Math/Rect.h>
#include <Math/AABBox3.h>
#include <Math/Vector.h>
#include <Math/Matrix2.h>
#include <Math/Matrix3.h>
#include <Math/Matrix4.h>
#include <Base/GlobalEnum.h>
#include <Base/Vector.h>

#include <QAction>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace TestUIModuleDetails
{
using namespace DAVA;
struct Result
{
    ReflectionBase* model = nullptr;
    QLayout* layout = nullptr;
};

using TestSpaceCreator = Result (*)(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent);

struct MethodTestData : public ReflectionBase
{
    void PrintPlus()
    {
        DAVA::Logger::Info("++++ PLUS ++++");
    }

    void PrintMinus()
    {
        DAVA::Logger::Info("---- Minus ----");
    }

    QIcon GetPlusIcon() const
    {
        return SharedIcon(":/QtIcons/cplus.png");
    }

    QIcon GetMinusIcon() const
    {
        return SharedIcon(":/QtIcons/cminus.png");
    }

    QString GetPlusText() const
    {
        return "plus text";
    }

    QString GetMinusText() const
    {
        return "minus text";
    }

    void SetEnabled(bool a)
    {
        enabled = a;
    }

    bool GetEnabled() const
    {
        return enabled;
    }

    bool GetInverseEnabled() const
    {
        return !enabled;
    }

    bool enabled = false;
    bool autoRaise = false;

    DAVA::String GetEnabledText() const
    {
        return (enabled) ? "Enabled" : "Disabled";
    }

    DAVA::String GetAutoRaiseText() const
    {
        return (autoRaise) ? "AutoRaise on" : "AutoRaise off";
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MethodTestData, ReflectionBase)
    {
        ReflectionRegistrator<MethodTestData>::Begin()
        .Field("enabled", &MethodTestData::enabled)
        .Field("enabledText", &MethodTestData::GetEnabledText, nullptr)
        .Field("autoRaise", &MethodTestData::autoRaise)
        .Field("autoRaiseText", &MethodTestData::GetAutoRaiseText, nullptr)
        .Field("plusIcon", &MethodTestData::GetPlusIcon, nullptr)
        .Field("plusText", &MethodTestData::GetPlusText, nullptr)
        .Field("minusIcon", &MethodTestData::GetMinusIcon, nullptr)
        .Field("minusText", &MethodTestData::GetMinusText, nullptr)
        .Method("plusMethod", &MethodTestData::PrintPlus)
        .Method("minusMethod", &MethodTestData::PrintMinus)
        .Method("inverseEnabled", &MethodTestData::GetInverseEnabled)
        .Field("enabledSetter", &MethodTestData::GetEnabled, &MethodTestData::SetEnabled)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        using namespace DAVA::TArc;
        MethodTestData* data = new MethodTestData();
        QVBoxLayout* boxLayout = new QVBoxLayout();

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Print Plus Icon: ", parent));

            ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ReflectedButton::Fields::Clicked] = "plusMethod";
            params.fields[ReflectedButton::Fields::Icon] = "plusIcon";
            ReflectedButton* button = new ReflectedButton(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(button->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Print Minus Text: ", parent));

            ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ReflectedButton::Fields::Clicked] = "minusMethod";
            params.fields[ReflectedButton::Fields::Text] = "minusText";
            ReflectedButton* button = new ReflectedButton(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(button->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Print Plus Text&Icon: ", parent));

            ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ReflectedButton::Fields::Clicked] = "plusMethod";
            params.fields[ReflectedButton::Fields::Icon] = "plusIcon";
            params.fields[ReflectedButton::Fields::Text] = "plusText";
            ReflectedButton* button = new ReflectedButton(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(button->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Print Minus Icon AutoRize|Enabled: ", parent));

            ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ReflectedButton::Fields::Clicked] = "minusMethod";
            params.fields[ReflectedButton::Fields::Icon] = "minusIcon";
            params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
            params.fields[ReflectedButton::Fields::Enabled] = "enabled";
            ReflectedButton* button = new ReflectedButton(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(button->ToWidgetCast());
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "enabled";
                params.fields[CheckBox::Fields::TextHint] = "enabledText";
                CheckBox* checkBox = new CheckBox(params, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Read only check box
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "autoRaise";
                params.fields[CheckBox::Fields::TextHint] = "autoRaiseText";
                CheckBox* checkBox = new CheckBox(params, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Inverse method: ", parent));

            ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ReflectedButton::Fields::Clicked] = "inverseEnabled";
            params.fields[ReflectedButton::Fields::Icon] = "minusIcon";
            params.fields[ReflectedButton::Fields::Enabled] = "enabled";
            params.fields[ReflectedButton::Fields::Result] = "enabledSetter";
            ReflectedButton* button = new ReflectedButton(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(button->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct CheckBoxTestData : public ReflectionBase
{
    bool GetInvertedValue() const
    {
        return !value;
    }
    void SetInvertedValue(bool v)
    {
        Logger::Info("++++ newValue: %d", !v);
        value = !v;
    }

    String GetInvertedDescription()
    {
        return value == true ? "False" : "True";
    }
    bool value = false;

    static String ValueDescription(const Any& v)
    {
        bool value = v.Cast<bool>();
        return value == true ? "True" : "False";
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(CheckBoxTestData, ReflectionBase)
    {
        ReflectionRegistrator<CheckBoxTestData>::Begin()
        .Field("value", &CheckBoxTestData::value)[DAVA::M::ValueDescription(&CheckBoxTestData::ValueDescription)]
        .Field("invertedValue", &CheckBoxTestData::GetInvertedValue, &CheckBoxTestData::SetInvertedValue)
        .Field("valueDescription", &CheckBoxTestData::GetInvertedDescription, nullptr)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        Result r;
        CheckBoxTestData* data = new CheckBoxTestData();
        r.model = data;
        r.layout = new QVBoxLayout();
        Reflection reflection = Reflection::Create(data);

        {
            CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[CheckBox::Fields::Checked] = "value";
            CheckBox* valueBox = new CheckBox(params, accessor, reflection, parent); //->ToWidgetCast();
            r.layout->addWidget(valueBox->ToWidgetCast());
        }

        {
            CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[CheckBox::Fields::Checked] = "invertedValue";
            params.fields[CheckBox::Fields::TextHint] = "valueDescription";
            params.fields[CheckBox::Fields::IsReadOnly] = "value";
            CheckBox* valueInvertedBox = new CheckBox(params, accessor, reflection, parent); //->ToWidgetCast();
            r.layout->addWidget(valueInvertedBox->ToWidgetCast());
        }

        return r;
    }
};

struct LineEditTestData : public ReflectionBase
{
    String text = "Text in line edit";
    String placeHolder = "Empty string";
    bool isReadOnly = false;
    bool isEnabled = true;

    String GetText() const
    {
        return text;
    }

    void SetText(const String& t)
    {
        text = t;
    }

    static String GetReadOnlyDescription(const Any& v)
    {
        bool value = v.Cast<bool>();
        return value == true ? "Read only" : "Writable";
    }

    static String GetEnableDescription(const Any& v)
    {
        bool value = v.Cast<bool>();
        return value == true ? "Enabled" : "Disabled";
    }

    static M::ValidationResult ValidateText(const Any& value, const Any& prevValue)
    {
        String v = value.Cast<String>();
        String pv = prevValue.Cast<String>();
        M::ValidationResult r;
        r.state = M::ValidationResult::eState::Valid;

        if (v.size() > 20)
        {
            r.state = M::ValidationResult::eState::Invalid;
            r.message = "Too long text!";
        }

        return r;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LineEditTestData, ReflectionBase)
    {
        using namespace DAVA::M;

        ReflectionRegistrator<LineEditTestData>::Begin()
        .Field("readOnlyMetaText", &LineEditTestData::text)[ReadOnly()]
        .Field("readOnlyText", &LineEditTestData::GetText, nullptr)
        .Field("text", &LineEditTestData::GetText, &LineEditTestData::SetText)[Validator(&LineEditTestData::ValidateText)]
        .Field("isTextReadOnly", &LineEditTestData::isReadOnly)[DAVA::M::ValueDescription(&LineEditTestData::GetReadOnlyDescription)]
        .Field("isTextEnabled", &LineEditTestData::isEnabled)[DAVA::M::ValueDescription(&LineEditTestData::GetEnableDescription)]
        .Field("placeholder", &LineEditTestData::placeHolder)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;
        LineEditTestData* data = new LineEditTestData();
        QVBoxLayout* boxLayout = new QVBoxLayout();

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Meta read only : ", parent));

            LineEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[LineEdit::Fields::Text] = "readOnlyMetaText";
            LineEdit* lineEdit = new LineEdit(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Read only : ", parent));

            LineEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[LineEdit::Fields::Text] = "readOnlyText";
            LineEdit* lineEdit = new LineEdit(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Editable : ", parent));

            LineEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[LineEdit::Fields::Text] = "text";
            params.fields[LineEdit::Fields::PlaceHolder] = "placeholder";
            params.fields[LineEdit::Fields::IsReadOnly] = "isTextReadOnly";
            params.fields[LineEdit::Fields::IsEnabled] = "isTextEnabled";
            LineEdit* lineEdit = new LineEdit(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "isTextReadOnly";
                CheckBox* checkBox = new CheckBox(params, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Is enabled
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "isTextEnabled";
                CheckBox* checkBox = new CheckBox(params, accessor, Reflection::Create(data), parent);

                QVBoxLayout* vbox = new QVBoxLayout(checkBox->ToWidgetCast());
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct PlainTextEditTestData : public ReflectionBase
{
    String text = "Text in text edit";
    String placeHolder = "Enter text";
    bool isReadOnly = false;
    bool isEnabled = true;

    String GetText() const
    {
        return text;
    }

    void SetText(const String& t)
    {
        text = t;
    }

    static String GetReadOnlyDescription(const Any& v)
    {
        bool value = v.Cast<bool>();
        return value == true ? "Read only" : "Writable";
    }

    static String GetEnableDescription(const Any& v)
    {
        bool value = v.Cast<bool>();
        return value == true ? "Enabled" : "Disabled";
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(PlainTextEditTestData, ReflectionBase)
    {
        using namespace DAVA::M;

        ReflectionRegistrator<PlainTextEditTestData>::Begin()
        .Field("readOnlyMetaText", &PlainTextEditTestData::text)[ReadOnly()]
        .Field("readOnlyText", &PlainTextEditTestData::GetText, nullptr)
        .Field("text", &PlainTextEditTestData::GetText, &PlainTextEditTestData::SetText)
        .Field("shortText", &PlainTextEditTestData::GetText, &PlainTextEditTestData::SetText)[M::MaxLength(20)]
        .Field("isTextReadOnly", &PlainTextEditTestData::isReadOnly)[DAVA::M::ValueDescription(&PlainTextEditTestData::GetReadOnlyDescription)]
        .Field("isTextEnabled", &PlainTextEditTestData::isEnabled)[DAVA::M::ValueDescription(&PlainTextEditTestData::GetEnableDescription)]
        .Field("placeholder", &PlainTextEditTestData::placeHolder)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;
        PlainTextEditTestData* data = new PlainTextEditTestData();
        QVBoxLayout* boxLayout = new QVBoxLayout();

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Meta read only : ", parent));

            PlainTextEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[PlainTextEdit::Fields::Text] = "readOnlyMetaText";
            PlainTextEdit* textEdit = new PlainTextEdit(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(textEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Read only : ", parent));

            PlainTextEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[PlainTextEdit::Fields::Text] = "readOnlyText";
            PlainTextEdit* textEdit = new PlainTextEdit(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(textEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Short text : ", parent));

            PlainTextEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[PlainTextEdit::Fields::Text] = "shortText";
            params.fields[PlainTextEdit::Fields::IsReadOnly] = "isTextReadOnly";
            params.fields[PlainTextEdit::Fields::IsEnabled] = "isTextEnabled";
            PlainTextEdit* textEdit = new PlainTextEdit(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(textEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Editable : ", parent));

            PlainTextEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[PlainTextEdit::Fields::Text] = "text";
            params.fields[PlainTextEdit::Fields::PlaceHolder] = "placeholder";
            params.fields[PlainTextEdit::Fields::IsReadOnly] = "isTextReadOnly";
            params.fields[PlainTextEdit::Fields::IsEnabled] = "isTextEnabled";
            PlainTextEdit* textEdit = new PlainTextEdit(params, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(textEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "isTextReadOnly";
                CheckBox* checkBox = new CheckBox(params, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Is enabled
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "isTextEnabled";
                CheckBox* checkBox = new CheckBox(params, accessor, Reflection::Create(data), parent);

                QVBoxLayout* vbox = new QVBoxLayout(checkBox->ToWidgetCast());
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct ComboBoxTestData : public ReflectionBase
{
    enum eTest
    {
        FIRST = 0,
        SECOND,
        THIRD
    };

    //int
    int testValue = eTest::SECOND;
    int GetValue() const
    {
        return testValue;
    }
    void SetValue(int newValue)
    {
        testValue = newValue;
    }

    size_t GetValueSize_t() const
    {
        return testValue;
    }
    void SetValueSize_t(size_t newValue)
    {
        testValue = static_cast<int>(newValue);
    }

    Map<int, String> enumeratorMap = Map<int, String>{
        { FIRST, "FIRST" },
        { SECOND, "SECOND" },
        { THIRD, "THIRD" }
    };

    Vector<String> enumeratorVector = Vector<String>{ "FIRST", "SECOND", "THIRD" };

    const Map<int, ComboBoxTestDataDescr>& GetEnumeratorMap() const
    {
        static Map<int, ComboBoxTestDataDescr> iconEnumerator = Map<int, ComboBoxTestDataDescr>
        {
          { FIRST, { "FIRST", SharedIcon(":/QtIcons/openscene.png") } },
          { SECOND, { "SECOND", SharedIcon(":/QtIcons/sound.png") } },
          { THIRD, { "THIRD", SharedIcon(":/QtIcons/remove.png") } }
        };

        return iconEnumerator;
    }

    const Set<String>& GetEnumeratorSet() const
    {
        static Set<String> enumeratorSet = Set<String>{ "1_FIRST", "2_SECOND", "3_THIRD" };
        return enumeratorSet;
    }

    const String GetStringValue() const
    {
        const Set<String>& setEnumerator = GetEnumeratorSet();
        auto it = setEnumerator.begin();

        int i = 0;
        for (int i = 0; i < static_cast<int>(setEnumerator.size()); ++i, ++it)
        {
            if (i == testValue)
            {
                return *it;
            }
        }

        return String();
    }

    void SetStringValue(const String& str)
    {
        const Set<String>& setEnumerator = GetEnumeratorSet();
        auto it = setEnumerator.begin();

        int i = 0;
        for (int i = 0; i < static_cast<int>(setEnumerator.size()); ++i, ++it)
        {
            if (str == *it)
            {
                testValue = i;
                break;
            }
        }
    }

    bool readOnly = false;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ComboBoxTestData, ReflectionBase)
    {
        ReflectionRegistrator<ComboBoxTestData>::Begin()
        .Field("readOnly", &ComboBoxTestData::readOnly)
        .Field("value", &ComboBoxTestData::testValue)
        .Field("valueSize_t", &ComboBoxTestData::GetValueSize_t, &ComboBoxTestData::SetValueSize_t)
        .Field("valueString", &ComboBoxTestData::GetStringValue, &ComboBoxTestData::SetStringValue)
        .Field("method", &ComboBoxTestData::GetValue, &ComboBoxTestData::SetValue)

        .Field("enumeratorValueMap", &ComboBoxTestData::enumeratorMap)
        .Field("enumeratorValueVector", &ComboBoxTestData::enumeratorVector)
        .Field("enumeratorMethod", &ComboBoxTestData::GetEnumeratorMap, nullptr)
        .Field("enumeratorMethodSet", &ComboBoxTestData::GetEnumeratorSet, nullptr)

        .Field("valueMetaReadOnly", &ComboBoxTestData::testValue)[DAVA::M::EnumT<ComboBoxTestData::eTest>(), DAVA::M::ReadOnly()]
        .Field("methodMetaOnlyGetter", &ComboBoxTestData::GetValue, nullptr)[DAVA::M::EnumT<ComboBoxTestData::eTest>()]
        .Field("valueMeta", &ComboBoxTestData::testValue)[DAVA::M::EnumT<ComboBoxTestData::eTest>()]
        .Field("methodMeta", &ComboBoxTestData::GetValue, &ComboBoxTestData::SetValue)[DAVA::M::EnumT<ComboBoxTestData::eTest>()]
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        ComboBoxTestData* data = new ComboBoxTestData();
        Reflection dataModel = Reflection::Create(data);

        QVBoxLayout* boxLayout = new QVBoxLayout();

        auto addTest = [&](const QString& testName, const ComboBox::Params& params)
        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel(testName, parent));

            ComboBox* comboBox = new ComboBox(params, accessor, dataModel, parent);
            lineLayout->addWidget(comboBox->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        };

        // read only
        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "valueMeta";
            params.fields[ComboBox::Fields::IsReadOnly] = "readOnly";
            addTest("Value[Meta][ReadOnly by field]:", params);
        }

        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "valueMetaReadOnly";
            addTest("Value[Meta][ReadOnly]: ", params);
        }

        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "methodMetaOnlyGetter";
            addTest("Method[Meta][NoSetter == ReadOnly]: ", params);
        }

        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "methodMeta";
            addTest("Method[Meta]: ", params);
        }

        //enumerator
        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "value";
            params.fields[ComboBox::Fields::Enumerator] = "enumeratorValueMap";
            addTest("Value[Enumerator]: ", params);
        }

        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "value";
            params.fields[ComboBox::Fields::Enumerator] = "enumeratorMethod";
            addTest("Value[EnumeratorMethod]: ", params);
        }

        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "valueSize_t";
            params.fields[ComboBox::Fields::Enumerator] = "enumeratorValueVector";
            addTest("Value[EnumeratorVector]: ", params);
        }

        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "method";
            params.fields[ComboBox::Fields::Enumerator] = "enumeratorMethod";
            addTest("Method[EnumeratorMethod]: ", params);
        }

        {
            ComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBox::Fields::Value] = "valueString";
            params.fields[ComboBox::Fields::Enumerator] = "enumeratorMethodSet";
            addTest("StringValue[EnumeratorSet]: ", params);
        }

        //readonly check box
        {
            //add read only check box
            CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[CheckBox::Fields::Checked] = "readOnly";
            CheckBox* readOnlyBox = new CheckBox(params, accessor, dataModel, parent);
            boxLayout->addWidget(readOnlyBox->ToWidgetCast());
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct ComboBoxCheckableTestData : public ReflectionBase
{
    enum eFlags
    {
        NONE = 0,
        BIT_1 = 1 << 0,
        BIT_2 = 1 << 1,
        BIT_3 = 1 << 2,

        MIX_1_2 = BIT_1 | BIT_2,
        MIX_1_3 = BIT_1 | BIT_3,
        MIX_2_3 = BIT_2 | BIT_3,

        ALL = BIT_1 | BIT_2 | BIT_3
    };

    //int
    int testValue = eFlags::BIT_2;
    bool readOnly = false;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ComboBoxCheckableTestData, ReflectionBase)
    {
        ReflectionRegistrator<ComboBoxCheckableTestData>::Begin()
        .Field("readOnly", &ComboBoxCheckableTestData::readOnly)
        .Field("value", &ComboBoxCheckableTestData::testValue)[DAVA::M::FlagsT<ComboBoxCheckableTestData::eFlags>()]
        .Field("valueMetaReadOnly", &ComboBoxCheckableTestData::testValue)[DAVA::M::FlagsT<ComboBoxCheckableTestData::eFlags>(), DAVA::M::ReadOnly()]
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        ComboBoxCheckableTestData* data = new ComboBoxCheckableTestData();
        Reflection dataModel = Reflection::Create(data);

        QVBoxLayout* boxLayout = new QVBoxLayout();

        auto addTest = [&](const QString& testName, const ComboBoxCheckable::Params& params)
        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel(testName, parent));

            ComboBoxCheckable* comboBox = new ComboBoxCheckable(params, accessor, dataModel, parent);
            lineLayout->addWidget(comboBox->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        };

        // read only
        {
            ComboBoxCheckable::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBoxCheckable::Fields::Value] = "value";
            params.fields[ComboBoxCheckable::Fields::IsReadOnly] = "readOnly";
            addTest("Value[ReadOnly by field]:", params);
        }

        {
            ComboBoxCheckable::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBoxCheckable::Fields::Value] = "value";
            addTest("Value: ", params);
        }

        {
            ComboBoxCheckable::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ComboBoxCheckable::Fields::Value] = "valueMetaReadOnly";
            addTest("Value[ReadOnly by Meta]:", params);
        }

        //readonly check box
        {
            //add read only check box
            CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[CheckBox::Fields::Checked] = "readOnly";
            CheckBox* readOnlyBox = new CheckBox(params, accessor, dataModel, parent);
            boxLayout->addWidget(readOnlyBox->ToWidgetCast());
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct IntSpinBoxTestData : public ReflectionBase
{
    int v = 10;

    Any GetValue() const
    {
        if (v == 10)
        {
            return Any();
        }
        return Any(v);
    }

    void SetValue(const Any& value)
    {
        v = value.Cast<int>();
    }

    String GetTextValue() const
    {
        return std::to_string(v);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(IntSpinBoxTestData)
    {
        ReflectionRegistrator<IntSpinBoxTestData>::Begin()
        .Field("value", &IntSpinBoxTestData::GetValue, &IntSpinBoxTestData::SetValue)[M::Range(0, 300, 2)]
        .Field("text", &IntSpinBoxTestData::GetTextValue, nullptr)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        Result r;
        IntSpinBoxTestData* data = new IntSpinBoxTestData();
        r.model = data;
        QtVBoxLayout* layout = new QtVBoxLayout();
        r.layout = layout;
        Reflection reflection = Reflection::Create(data);

        {
            IntSpinBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[IntSpinBox::Fields::Value] = "value";
            IntSpinBox* spinBox = new IntSpinBox(params, accessor, reflection, parent);
            layout->AddControl(spinBox);
        }

        {
            LineEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[LineEdit::Fields::Text] = "text";
            layout->AddControl(new LineEdit(params, accessor, reflection, parent));
        }

        return r;
    }
};

struct DoubleSpinBoxTestData : public ReflectionBase
{
    double v = 10;
    int accuracy = 3;

    Any GetValue() const
    {
        if (v == 10.0)
        {
            return Any("No value");
        }
        return Any(v);
    }

    void SetValue(const Any& value)
    {
        v = value.Cast<double>();
    }

    int GetAccuracy() const
    {
        return accuracy;
    }

    void SetAccuracy(int v)
    {
        accuracy = v;
    }

    String GetTextValue() const
    {
        return std::to_string(v);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(IntSpinBoxTestData)
    {
        ReflectionRegistrator<DoubleSpinBoxTestData>::Begin()
        .Field("value", &DoubleSpinBoxTestData::GetValue, &DoubleSpinBoxTestData::SetValue)[M::Range(-100.0, 300, 0.2), M::FloatNumberAccuracy(4)]
        .Field("accuracy", &DoubleSpinBoxTestData::GetAccuracy, &DoubleSpinBoxTestData::SetAccuracy)
        .Field("text", &DoubleSpinBoxTestData::GetTextValue, nullptr)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        Result r;
        DoubleSpinBoxTestData* data = new DoubleSpinBoxTestData();
        r.model = data;
        QtVBoxLayout* layout = new QtVBoxLayout();
        r.layout = layout;
        Reflection reflection = Reflection::Create(data);

        {
            DoubleSpinBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[DoubleSpinBox::Fields::Value] = "value";
            layout->AddControl(new DoubleSpinBox(params, accessor, reflection, parent));
        }

        {
            DoubleSpinBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[DoubleSpinBox::Fields::Value] = "value";
            params.fields[DoubleSpinBox::Fields::Accuracy] = "accuracy";
            layout->AddControl(new DoubleSpinBox(params, accessor, reflection, parent));
        }

        {
            IntSpinBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[IntSpinBox::Fields::Value] = "accuracy";
            layout->AddControl(new IntSpinBox(params, accessor, reflection, parent));
        }

        {
            LineEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[LineEdit::Fields::Text] = "text";
            layout->AddControl(new LineEdit(params, accessor, reflection, parent));
        }

        return r;
    }
};

struct FilePathEditTestData : public ReflectionBase
{
    FilePath path = "~res:/Materials/2d.Color.material";
    FilePath root = "~res:/Materials/";
    String placeHolder = "Empty string";
    bool isReadOnly = false;
    bool isEnabled = true;

    const FilePath& GetText() const
    {
        return path;
    }

    void SetText(const FilePath& t)
    {
        path = t;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(FilePathEditTestData, ReflectionBase)
    {
        using namespace DAVA;

        ReflectionRegistrator<FilePathEditTestData>::Begin()
        .Field("readOnlyMetaText", &FilePathEditTestData::path)[M::ReadOnly()]
        .Field("readOnlyText", &FilePathEditTestData::GetText, nullptr)
        .Field("path", &FilePathEditTestData::GetText, &FilePathEditTestData::SetText)[M::File("Materials (*.material);;Meta (*.meta)")]
        .Field("isTextReadOnly", &FilePathEditTestData::isReadOnly)
        .Field("isTextEnabled", &FilePathEditTestData::isEnabled)
        .Field("placeholder", &FilePathEditTestData::placeHolder)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;
        FilePathEditTestData* data = new FilePathEditTestData();
        QVBoxLayout* boxLayout = new QVBoxLayout();

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Meta read only : ", parent));

            FilePathEdit::Params p(accessor, ui, DAVA::TArc::mainWindowKey);
            p.fields[FilePathEdit::Fields::Value] = "readOnlyMetaText";
            lineLayout->AddControl(new FilePathEdit(p, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Read only : ", parent));

            FilePathEdit::Params p(accessor, ui, DAVA::TArc::mainWindowKey);
            p.fields[FilePathEdit::Fields::Value] = "readOnlyText";
            lineLayout->AddControl(new FilePathEdit(p, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Editable : ", parent));

            FilePathEdit::Params p(accessor, ui, DAVA::TArc::mainWindowKey);
            p.fields[FilePathEdit::Fields::Value] = "path";
            p.fields[FilePathEdit::Fields::PlaceHolder] = "placeholder";
            p.fields[FilePathEdit::Fields::IsReadOnly] = "isTextReadOnly";
            p.fields[FilePathEdit::Fields::IsEnabled] = "isTextEnabled";
            lineLayout->AddControl(new FilePathEdit(p, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "isTextReadOnly";
                CheckBox* checkBox = new CheckBox(params, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Is enabled
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "isTextEnabled";
                CheckBox* checkBox = new CheckBox(params, accessor, Reflection::Create(data), parent);

                QVBoxLayout* vbox = new QVBoxLayout(checkBox->ToWidgetCast());
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct SubPropertiesControlTest : public ReflectionBase
{
    DAVA::Vector2 v2 = DAVA::Vector2(0.2f, 0.2f);
    DAVA::Vector3 v3 = DAVA::Vector3(0.3f, 0.3f, 0.3f);
    DAVA::Vector4 v4 = DAVA::Vector4(0.4f, 0.4f, 0.4f, 0.4f);
    DAVA::Rect rect = DAVA::Rect(0.1f, 0.123456f, 300.2f, 102.2f);
    DAVA::AABBox3 box = DAVA::AABBox3(DAVA::Vector3(0.1f, 0.1f, 0.1f), DAVA::Vector3(100.0f, 100.0f, 100.0f));

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;
        SubPropertiesControlTest* data = new SubPropertiesControlTest();
        QtVBoxLayout* boxLayout = new QtVBoxLayout();
        Reflection model = Reflection::Create(data);

        {
            SubPropertiesEditor::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[SubPropertiesEditor::Fields::Value] = "v2";
            boxLayout->AddControl(new SubPropertiesEditor(params, accessor, model));
        }

        {
            SubPropertiesEditor::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[SubPropertiesEditor::Fields::Value] = "v3";
            boxLayout->AddControl(new SubPropertiesEditor(params, accessor, model));
        }

        {
            SubPropertiesEditor::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[SubPropertiesEditor::Fields::Value] = "v4";
            boxLayout->AddControl(new SubPropertiesEditor(params, accessor, model));
        }

        {
            SubPropertiesEditor::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[SubPropertiesEditor::Fields::Value] = "rect";
            boxLayout->AddControl(new SubPropertiesEditor(params, accessor, model));
        }

        {
            SubPropertiesEditor::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[SubPropertiesEditor::Fields::Value] = "box";
            boxLayout->AddControl(new SubPropertiesEditor(params, accessor, model));
        }

        Result r;
        r.layout = boxLayout;
        r.model = data;

        return r;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SubPropertiesControlTest)
    {
        DAVA::ReflectionRegistrator<SubPropertiesControlTest>::Begin()
        .Field("v2", &SubPropertiesControlTest::v2)[DAVA::M::ReadOnly()]
        .Field("v3", &SubPropertiesControlTest::v3)
        .Field("v4", &SubPropertiesControlTest::v4)
        .Field("rect", &SubPropertiesControlTest::rect)
        .Field("box", &SubPropertiesControlTest::box)
        .End();
    }
};

struct ColorButtonTestData : public ReflectionBase
{
    ColorButtonTestData()
        : ReflectionBase()
    {
        colorRange.reset(new M::Range(Color(0.2f, 0.2f, 0.2f, 0.2f), Color(0.4f, 0.4f, 0.4f, 0.4f), Color(0.1f, 0.1f, 0.1f, 0.1f)));
    }

    Color GetColorInverted() const
    {
        Color c = Color::White - color;
        c.a = 1.0f;
        return c;
    }

    void SetColor(const Color& c)
    {
        color = c;
    }

    Color color = Color(1.0f, 0.0f, 0.0f, 1.0f);
    std::shared_ptr<M::Range> colorRange;
    const M::Range* GetColorRange() const
    {
        return colorRange.get();
    }

    bool readOnly = false;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ColorButtonTestData, ReflectionBase)
    {
        ReflectionRegistrator<ColorButtonTestData>::Begin()
        .Field("color", &ColorButtonTestData::color)
        .Field("colorReadOnly", &ColorButtonTestData::color)[M::ReadOnly()]
        .Field("colorMethod", &ColorButtonTestData::GetColorInverted, &ColorButtonTestData::SetColor)
        .Field("colorMethodReadOnly", &ColorButtonTestData::GetColorInverted, nullptr)
        .Field("readOnly", &ColorButtonTestData::readOnly)
        .Field("colorRange", &ColorButtonTestData::color)[M::Range(Color(0.8f, 0.8f, 0.8f, 1.f), Color(1.0f, 1.0f, 0.8f, 1.f), Color(0.1f, 0.1f, 0.1f, 0.1f))]
        .Field("range", &ColorButtonTestData::GetColorRange, nullptr)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        ColorButtonTestData* data = new ColorButtonTestData();
        QtVBoxLayout* boxLayout = new QtVBoxLayout();

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Color: ", parent));

            ColorPickerButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ColorPickerButton::Fields::Color] = "color";
            lineLayout->AddControl(new ColorPickerButton(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("ColorReadOnlyMeta: ", parent));

            ColorPickerButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ColorPickerButton::Fields::Color] = "colorReadOnly";
            lineLayout->AddControl(new ColorPickerButton(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Color Method: ", parent));

            ColorPickerButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ColorPickerButton::Fields::Color] = "colorMethod";
            lineLayout->AddControl(new ColorPickerButton(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Color Method ReadOnly: ", parent));

            ColorPickerButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ColorPickerButton::Fields::Color] = "colorMethodReadOnly";
            lineLayout->AddControl(new ColorPickerButton(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Color Meta Range: ", parent));

            ColorPickerButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ColorPickerButton::Fields::Color] = "colorRange";
            lineLayout->AddControl(new ColorPickerButton(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Color Range Field: ", parent));

            ColorPickerButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ColorPickerButton::Fields::Color] = "color";
            params.fields[ColorPickerButton::Fields::Range] = "range";
            lineLayout->AddControl(new ColorPickerButton(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Color ReadOnly value: ", parent));

            {
                ColorPickerButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[ColorPickerButton::Fields::Color] = "color";
                params.fields[ColorPickerButton::Fields::IsReadOnly] = "readOnly";
                lineLayout->AddControl(new ColorPickerButton(params, accessor, Reflection::Create(data), parent));
            }

            {
                //add read only check box
                CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
                params.fields[CheckBox::Fields::Checked] = "readOnly";
                lineLayout->AddControl(new CheckBox(params, accessor, Reflection::Create(data), parent));
            }
            boxLayout->addLayout(lineLayout);
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct MultiEditorsControlTest : public ReflectionBase
{
    DAVA::Vector3 v3 = DAVA::Vector3(0.3f, 0.3f, 0.3f);
    Vector<DAVA::TArc::MultiDoubleSpinBox::FieldDescriptor> descriptorList;
    bool isReadOnly = false;
    int32 accuracy = 4;
    int32 accuracy2 = 6;
    const DAVA::M::Range* rangeX = nullptr;
    const DAVA::M::Range* rangeY = nullptr;
    const DAVA::M::Range* rangeZ = nullptr;

    MultiEditorsControlTest()
        : rangeX(new DAVA::M::Range(0.0f, 1.0f, 0.1))
        , rangeY(new DAVA::M::Range(-1.0f, 1.0f, 0.2))
        , rangeZ(new DAVA::M::Range(-0.5f, 0.5f, 0.3))
    {
        using namespace DAVA::TArc;
        {
            MultiDoubleSpinBox::FieldDescriptor d;
            d.valueRole = "V3X";
            d.accuracyRole = "accuracy";
            d.readOnlyRole = "isReadOnly";
            d.rangeRole = "RangeX";
            descriptorList.push_back(d);
        }

        {
            MultiDoubleSpinBox::FieldDescriptor d;
            d.valueRole = "V3Y";
            d.accuracyRole = "accuracy2";
            d.readOnlyRole = "isReadOnly";
            d.rangeRole = "RangeY";
            descriptorList.push_back(d);
        }

        {
            MultiDoubleSpinBox::FieldDescriptor d;
            d.valueRole = "V3Z";
            d.accuracyRole = "accuracy";
            d.readOnlyRole = "isReadOnly";
            d.rangeRole = "RangeZ";
            descriptorList.push_back(d);
        }
    }

    float32 GetX() const
    {
        return v3.x;
    }

    void SetX(float32 v)
    {
        v3.x = v;
    }

    float32 GetY() const
    {
        return v3.y;
    }

    void SetY(float32 v)
    {
        v3.y = v;
    }

    float32 GetZ() const
    {
        return v3.z;
    }

    void SetZ(float32 v)
    {
        v3.z = v;
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;
        MultiEditorsControlTest* data = new MultiEditorsControlTest();
        QtVBoxLayout* boxLayout = new QtVBoxLayout();
        Reflection model = Reflection::Create(data);

        {
            MultiDoubleSpinBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[MultiDoubleSpinBox::Fields::FieldsList] = "list";
            boxLayout->AddControl(new MultiDoubleSpinBox(params, accessor, model));
        }

        {
            CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[CheckBox::Fields::Checked] = "isReadOnly";
            boxLayout->AddControl(new CheckBox(params, accessor, model));
        }

        Result r;
        r.layout = boxLayout;
        r.model = data;

        return r;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MultiEditorsControlTest)
    {
        DAVA::ReflectionRegistrator<MultiEditorsControlTest>::Begin()
        .Field("list", &MultiEditorsControlTest::descriptorList)
        .Field("V3X", &MultiEditorsControlTest::GetX, &MultiEditorsControlTest::SetX)
        .Field("V3Y", &MultiEditorsControlTest::GetY, &MultiEditorsControlTest::SetY)
        .Field("V3Z", &MultiEditorsControlTest::GetZ, &MultiEditorsControlTest::SetZ)
        .Field("accuracy", &MultiEditorsControlTest::accuracy)
        .Field("accuracy2", &MultiEditorsControlTest::accuracy2)
        .Field("isReadOnly", &MultiEditorsControlTest::isReadOnly)
        .Field("RangeX", &MultiEditorsControlTest::rangeX)
        .Field("RangeY", &MultiEditorsControlTest::rangeY)
        .Field("RangeZ", &MultiEditorsControlTest::rangeZ)
        .End();
    }
};

struct LabelTestData : public ReflectionBase
{
    String testString = "Test String";
    QString testQString = "Test QString";
    Matrix2 matrix2;
    Matrix3 matrix3;
    Matrix4 matrix4;

    const String& GetString() const
    {
        return testString;
    }

    const QString& GetQString() const
    {
        return testQString;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LabelTestData, ReflectionBase)
    {
        using namespace DAVA;

        ReflectionRegistrator<LabelTestData>::Begin()
        .Field("string", &LabelTestData::testString)
        .Field("qstring", &LabelTestData::testQString)
        .Field("getString", &LabelTestData::GetString, nullptr)
        .Field("getQString", &LabelTestData::GetQString, nullptr)
        .Field("matrix2", &LabelTestData::matrix2)
        .Field("matrix3", &LabelTestData::matrix3)
        .Field("matrix4", &LabelTestData::matrix4)
        .End();
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        LabelTestData* data = new LabelTestData();
        QVBoxLayout* boxLayout = new QVBoxLayout();

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("String : ", parent));

            Label::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[Label::Fields::Text] = "string";
            lineLayout->AddControl(new Label(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("QString : ", parent));

            Label::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[Label::Fields::Text] = "qstring";
            lineLayout->AddControl(new Label(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("GetString : ", parent));

            Label::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[Label::Fields::Text] = "getString";
            lineLayout->AddControl(new Label(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("GetQString : ", parent));

            Label::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[Label::Fields::Text] = "getQString";
            lineLayout->AddControl(new Label(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Matrix2 : ", parent));

            Label::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[Label::Fields::Text] = "matrix2";
            lineLayout->AddControl(new Label(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Matrix3 : ", parent));

            Label::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[Label::Fields::Text] = "matrix3";
            lineLayout->AddControl(new Label(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Matrix4 : ", parent));

            Label::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[Label::Fields::Text] = "matrix4";
            lineLayout->AddControl(new Label(params, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct Node
{
    TestSpaceCreator creator;
    QString title;
};
}

ENUM_DECLARE(TestUIModuleDetails::ComboBoxTestData::eTest)
{
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxTestData::eTest::FIRST), "1st");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxTestData::eTest::SECOND), "2nd");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxTestData::eTest::THIRD), "3rd");
}

ENUM_DECLARE(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags)
{
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags::NONE), "None");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags::BIT_1), "1-st");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags::BIT_2), "2-nd");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags::BIT_3), "3-rd");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags::MIX_1_2), "MIX_1_2");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags::MIX_1_3), "MIX_1_3");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags::MIX_2_3), "MIX_2_3");
    ENUM_ADD_DESCR(static_cast<int>(TestUIModuleDetails::ComboBoxCheckableTestData::eFlags::ALL), "All");
}

void TestUIModule::PostInit()
{
    using namespace DAVA::TArc;
    UI* ui = GetUI();
    ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "DebugFunctions"));

    QAction* assertAction = new QAction("UI Sandbox", nullptr);
    connections.AddConnection(assertAction, &QAction::triggered, DAVA::MakeFunction(this, &TestUIModule::ShowDialog));
    ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, assertAction);
}

void TestUIModule::ShowDialog()
{
    using namespace TestUIModuleDetails;
    QDialog* dlg = new QDialog();
    QHBoxLayout* layout = new QHBoxLayout(dlg);
    QTabWidget* tabWidget = new QTabWidget(dlg);
    layout->addWidget(tabWidget);
    dlg->setLayout(layout);

    DAVA::Vector<Node> nodes = DAVA::Vector<Node>
    {
      { &MethodTestData::Create, "Method Test" },
      { &CheckBoxTestData::Create, "CheckBox Test" },
      { &LineEditTestData::Create, "LineEdit Test" },
      { &PlainTextEditTestData::Create, "PlainTextEdit Test" },
      { &ComboBoxTestData::Create, "ComboBox Test" },
      { &ComboBoxCheckableTestData::Create, "ComboBoxCheckable Test" },
      { &IntSpinBoxTestData::Create, "SpinBoxTest" },
      { &DoubleSpinBoxTestData::Create, "Double Spin" },
      { &FilePathEditTestData::Create, "FilePath" },
      { &SubPropertiesControlTest::Create, "SubPropsControl Test" },
      { &MultiEditorsControlTest::Create, "MultiEditorsControl Test" },
      { &ColorButtonTestData::Create, "ColorButton Test" },
      { &LabelTestData::Create, "Label Test" }
    };

    DAVA::Vector<DAVA::ReflectionBase*> data;

    const int columnCount = 4;
    int currentColumn = 0;
    int currentRow = 0;
    DAVA::TArc::ContextAccessor* accessor = GetAccessor();
    DAVA::TArc::UI* ui = GetUI();
    for (Node& node : nodes)
    {
        QGroupBox* groupBox = new QGroupBox(node.title, dlg);

        TestUIModuleDetails::Result r = node.creator(ui, accessor, groupBox);
        data.push_back(r.model);

        groupBox->setLayout(r.layout);
        tabWidget->addTab(groupBox, node.title);
        ++currentColumn;
        if (currentColumn > columnCount)
        {
            currentColumn = 0;
            ++currentRow;
        }
    }

    dlg->exec();
    delete dlg;
    for (ReflectionBase* d : data)
    {
        delete d;
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(TestUIModule)
{
    DAVA::ReflectionRegistrator<TestUIModule>::Begin()
    .ConstructorByPointer()
    .End();
}

#if !defined(DEPLOY_BUILD)
DECL_GUI_MODULE(TestUIModule);
#endif
