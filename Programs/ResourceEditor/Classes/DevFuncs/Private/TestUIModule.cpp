#include "Classes/DevFuncs/TestUIModule.h"
#include "Classes/DevFuncs/TestUIModuleData.h"
#include "Classes/Application/REGlobal.h"

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
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>

#include <Logger/Logger.h>
#include <Math/Rect.h>
#include <Math/AABBox3.h>
#include <Math/Vector.h>
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

        ControlDescriptorBuilder<CheckBox::Fields> descr;
        descr[CheckBox::Fields::Checked] = "value";
        CheckBox* valueBox = new CheckBox(descr, accessor, reflection, parent); //->ToWidgetCast();
        r.layout->addWidget(valueBox->ToWidgetCast());

        ControlDescriptorBuilder<CheckBox::Fields> valueInvertedDescr;
        valueInvertedDescr[CheckBox::Fields::Checked] = "invertedValue";
        valueInvertedDescr[CheckBox::Fields::TextHint] = "valueDescription";
        valueInvertedDescr[CheckBox::Fields::IsReadOnly] = "value";
        CheckBox* valueInvertedBox = new CheckBox(valueInvertedDescr, accessor, reflection, parent); //->ToWidgetCast();
        r.layout->addWidget(valueInvertedBox->ToWidgetCast());

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

            ControlDescriptorBuilder<LineEdit::Fields> desr;
            desr[LineEdit::Fields::Text] = "readOnlyMetaText";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Read only : ", parent));

            ControlDescriptorBuilder<LineEdit::Fields> desr;
            desr[LineEdit::Fields::Text] = "readOnlyText";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Editable : ", parent));

            ControlDescriptorBuilder<LineEdit::Fields> desr;
            desr[LineEdit::Fields::Text] = "text";
            desr[LineEdit::Fields::PlaceHolder] = "placeholder";
            desr[LineEdit::Fields::IsReadOnly] = "isTextReadOnly";
            desr[LineEdit::Fields::IsEnabled] = "isTextEnabled";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Fields::Checked] = "isTextReadOnly";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Is enabled
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Fields::Checked] = "isTextEnabled";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);

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

            ControlDescriptorBuilder<PlainTextEdit::Fields> desr;
            desr[PlainTextEdit::Fields::Text] = "readOnlyMetaText";
            PlainTextEdit* textEdit = new PlainTextEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(textEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Read only : ", parent));

            ControlDescriptorBuilder<PlainTextEdit::Fields> desr;
            desr[PlainTextEdit::Fields::Text] = "readOnlyText";
            PlainTextEdit* textEdit = new PlainTextEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(textEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Short text : ", parent));

            ControlDescriptorBuilder<PlainTextEdit::Fields> desr;
            desr[PlainTextEdit::Fields::Text] = "shortText";
            desr[PlainTextEdit::Fields::IsReadOnly] = "isTextReadOnly";
            desr[PlainTextEdit::Fields::IsEnabled] = "isTextEnabled";
            PlainTextEdit* textEdit = new PlainTextEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(textEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Editable : ", parent));

            ControlDescriptorBuilder<PlainTextEdit::Fields> desr;
            desr[PlainTextEdit::Fields::Text] = "text";
            desr[PlainTextEdit::Fields::PlaceHolder] = "placeholder";
            desr[PlainTextEdit::Fields::IsReadOnly] = "isTextReadOnly";
            desr[PlainTextEdit::Fields::IsEnabled] = "isTextEnabled";
            PlainTextEdit* textEdit = new PlainTextEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(textEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Fields::Checked] = "isTextReadOnly";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Is enabled
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Fields::Checked] = "isTextEnabled";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);

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

        auto addTest = [&](const QString& testName, const ControlDescriptorBuilder<ComboBox::Fields>& descriptor)
        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel(testName, parent));

            ComboBox* comboBox = new ComboBox(descriptor, accessor, dataModel, parent);
            lineLayout->addWidget(comboBox->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        };

        // read only
        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "valueMeta";
            desr[ComboBox::Fields::IsReadOnly] = "readOnly";
            addTest("Value[Meta][ReadOnly by field]:", desr);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "valueMetaReadOnly";
            addTest("Value[Meta][ReadOnly]: ", desr);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "methodMetaOnlyGetter";
            addTest("Method[Meta][NoSetter == ReadOnly]: ", desr);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "methodMeta";
            addTest("Method[Meta]: ", desr);
        }

        //enumerator
        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "value";
            desr[ComboBox::Fields::Enumerator] = "enumeratorValueMap";
            addTest("Value[Enumerator]: ", desr);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "value";
            desr[ComboBox::Fields::Enumerator] = "enumeratorMethod";
            addTest("Value[EnumeratorMethod]: ", desr);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "valueSize_t";
            desr[ComboBox::Fields::Enumerator] = "enumeratorValueVector";
            addTest("Value[EnumeratorVector]: ", desr);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "method";
            desr[ComboBox::Fields::Enumerator] = "enumeratorMethod";
            addTest("Method[EnumeratorMethod]: ", desr);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> desr;
            desr[ComboBox::Fields::Value] = "valueString";
            desr[ComboBox::Fields::Enumerator] = "enumeratorMethodSet";
            addTest("StringValue[EnumeratorSet]: ", desr);
        }

        //readonly check box
        {
            //add read only check box
            ControlDescriptorBuilder<CheckBox::Fields> descr;
            descr[CheckBox::Fields::Checked] = "readOnly";
            CheckBox* readOnlyBox = new CheckBox(descr, accessor, dataModel, parent);
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

        auto addTest = [&](const QString& testName, const ControlDescriptorBuilder<ComboBoxCheckable::Fields>& descriptor)
        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel(testName, parent));

            ComboBoxCheckable* comboBox = new ComboBoxCheckable(descriptor, accessor, dataModel, parent);
            lineLayout->addWidget(comboBox->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        };

        // read only
        {
            ControlDescriptorBuilder<ComboBoxCheckable::Fields> desr;
            desr[ComboBoxCheckable::Fields::Value] = "value";
            desr[ComboBoxCheckable::Fields::IsReadOnly] = "readOnly";
            addTest("Value[ReadOnly by field]:", desr);
        }

        {
            ControlDescriptorBuilder<ComboBoxCheckable::Fields> desr;
            desr[ComboBoxCheckable::Fields::Value] = "value";
            addTest("Value: ", desr);
        }

        {
            ControlDescriptorBuilder<ComboBoxCheckable::Fields> desr;
            desr[ComboBoxCheckable::Fields::Value] = "valueMetaReadOnly";
            addTest("Value[ReadOnly by Meeta]:", desr);
        }

        //readonly check box
        {
            //add read only check box
            ControlDescriptorBuilder<CheckBox::Fields> descr;
            descr[CheckBox::Fields::Checked] = "readOnly";
            CheckBox* readOnlyBox = new CheckBox(descr, accessor, dataModel, parent);
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
            ControlDescriptorBuilder<IntSpinBox::Fields> descr;
            descr[IntSpinBox::Fields::Value] = "value";
            IntSpinBox* spinBox = new IntSpinBox(descr, accessor, reflection, parent);
            layout->AddWidget(spinBox);
        }

        {
            ControlDescriptorBuilder<LineEdit::Fields> d;
            d[LineEdit::Fields::Text] = "text";
            layout->AddWidget(new LineEdit(d, accessor, reflection, parent));
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
            ControlDescriptorBuilder<DoubleSpinBox::Fields> descr;
            descr[DoubleSpinBox::Fields::Value] = "value";
            layout->AddWidget(new DoubleSpinBox(descr, accessor, reflection, parent));
        }

        {
            ControlDescriptorBuilder<DoubleSpinBox::Fields> descr;
            descr[DoubleSpinBox::Fields::Value] = "value";
            descr[DoubleSpinBox::Fields::Accuracy] = "accuracy";
            layout->AddWidget(new DoubleSpinBox(descr, accessor, reflection, parent));
        }

        {
            ControlDescriptorBuilder<IntSpinBox::Fields> descr;
            descr[IntSpinBox::Fields::Value] = "accuracy";
            layout->AddWidget(new IntSpinBox(descr, accessor, reflection, parent));
        }

        {
            ControlDescriptorBuilder<LineEdit::Fields> d;
            d[LineEdit::Fields::Text] = "text";
            layout->AddWidget(new LineEdit(d, accessor, reflection, parent));
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

    String GetFilters() const
    {
        return "Materials (*.material)";
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(FilePathEditTestData, ReflectionBase)
    {
        using namespace DAVA;

        ReflectionRegistrator<FilePathEditTestData>::Begin()
        .Field("readOnlyMetaText", &FilePathEditTestData::path)[M::ReadOnly()]
        .Field("readOnlyText", &FilePathEditTestData::GetText, nullptr)
        .Field("path", &FilePathEditTestData::GetText, &FilePathEditTestData::SetText)[M::File(true, "Materials (*.material);;Meta (*.meta)")]
        .Field("filters", &FilePathEditTestData::GetFilters, nullptr)
        .Field("isTextReadOnly", &FilePathEditTestData::isReadOnly)
        .Field("isTextEnabled", &FilePathEditTestData::isEnabled)
        .Field("placeholder", &FilePathEditTestData::placeHolder)
        .Field("rootDir", &FilePathEditTestData::root)
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

            FilePathEdit::Params p;
            p.ui = ui;
            p.wndKey = REGlobal::MainWindowKey;
            p.fields[FilePathEdit::Fields::Value] = "readOnlyMetaText";
            lineLayout->AddWidget(new FilePathEdit(p, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Read only : ", parent));

            FilePathEdit::Params p;
            p.ui = ui;
            p.wndKey = REGlobal::MainWindowKey;
            p.fields[FilePathEdit::Fields::Value] = "readOnlyText";
            lineLayout->AddWidget(new FilePathEdit(p, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);
        }

        {
            QtHBoxLayout* lineLayout = new QtHBoxLayout();
            lineLayout->addWidget(new QLabel("Editable : ", parent));

            FilePathEdit::Params p;
            p.ui = ui;
            p.wndKey = REGlobal::MainWindowKey;
            p.fields[FilePathEdit::Fields::Value] = "path";
            p.fields[FilePathEdit::Fields::PlaceHolder] = "placeholder";
            p.fields[FilePathEdit::Fields::IsReadOnly] = "isTextReadOnly";
            p.fields[FilePathEdit::Fields::IsEnabled] = "isTextEnabled";
            p.fields[FilePathEdit::Fields::DialogTitle] = "placeholder";
            p.fields[FilePathEdit::Fields::RootDirectory] = "rootDir";
            //p.fields[FilePathEdit::Fields::Filters] = "filters";
            lineLayout->AddWidget(new FilePathEdit(p, accessor, Reflection::Create(data), parent));
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Fields::Checked] = "isTextReadOnly";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Is enabled
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Fields::Checked] = "isTextEnabled";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);

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
            ControlDescriptorBuilder<SubPropertiesEditor::Fields> descr;
            descr[SubPropertiesEditor::Fields::Value] = "v2";
            boxLayout->AddWidget(new SubPropertiesEditor(descr, accessor, model));
        }

        {
            ControlDescriptorBuilder<SubPropertiesEditor::Fields> descr;
            descr[SubPropertiesEditor::Fields::Value] = "v3";
            boxLayout->AddWidget(new SubPropertiesEditor(descr, accessor, model));
        }

        {
            ControlDescriptorBuilder<SubPropertiesEditor::Fields> descr;
            descr[SubPropertiesEditor::Fields::Value] = "v4";
            boxLayout->AddWidget(new SubPropertiesEditor(descr, accessor, model));
        }

        {
            ControlDescriptorBuilder<SubPropertiesEditor::Fields> descr;
            descr[SubPropertiesEditor::Fields::Value] = "rect";
            boxLayout->AddWidget(new SubPropertiesEditor(descr, accessor, model));
        }

        {
            ControlDescriptorBuilder<SubPropertiesEditor::Fields> descr;
            descr[SubPropertiesEditor::Fields::Value] = "box";
            boxLayout->AddWidget(new SubPropertiesEditor(descr, accessor, model));
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
        v3.y;
    }

    float32 GetZ() const
    {
        return v3.z;
    }

    void SetZ(float32 v)
    {
        v3.z;
    }

    static Result Create(TArc::UI* ui, TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;
        MultiEditorsControlTest* data = new MultiEditorsControlTest();
        QtVBoxLayout* boxLayout = new QtVBoxLayout();
        Reflection model = Reflection::Create(data);

        {
            ControlDescriptorBuilder<MultiDoubleSpinBox::Fields> descr;
            descr[MultiDoubleSpinBox::Fields::FieldsList] = "list";
            boxLayout->AddWidget(new MultiDoubleSpinBox(descr, accessor, model));
        }

        {
            ControlDescriptorBuilder<CheckBox::Fields> descr;
            descr[CheckBox::Fields::Checked] = "isReadOnly";
            boxLayout->AddWidget(new CheckBox(descr, accessor, model));
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
    ui->AddAction(REGlobal::MainWindowKey, placementInfo, assertAction);
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
      { &CheckBoxTestData::Create, "CheckBox Test" },
      { &LineEditTestData::Create, "LineEdit Test" },
      { &PlainTextEditTestData::Create, "PlainTextEdit Test" },
      { &ComboBoxTestData::Create, "ComboBox Test" },
      { &ComboBoxCheckableTestData::Create, "ComboBoxCheckable Test" },
      { &IntSpinBoxTestData::Create, "SpinBoxTest" },
      { &DoubleSpinBoxTestData::Create, "Double Spin" },
      { &FilePathEditTestData::Create, "FilePath" },
      { &SubPropertiesControlTest::Create, "SubPropsControl Test" },
      { &MultiEditorsControlTest::Create, "MultiEditorsControl Test" }
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

        Result r = node.creator(ui, accessor, groupBox);
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
