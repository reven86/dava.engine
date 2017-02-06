#include "Classes/DevFuncs/TestUIModule.h"
#include "Classes/DevFuncs/TestUIModuleData.h"
#include "Classes/Application/REGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/LineEdit.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>


#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <Base/GlobalEnum.h>
#include <Logger/Logger.h>

#include <QAction>
#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

namespace TestUIModuleDetails
{
using namespace DAVA;
struct Result
{
    ReflectionBase* model = nullptr;
    QLayout* layout = nullptr;
};

using TestSpaceCreator = Result (*)(TArc::ContextAccessor* accessor, QWidget* parent);

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

    static Result Create(TArc::ContextAccessor* accessor, QWidget* parent)
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

    static M::ValidatorResult ValidateText(const Any& value, const Any& prevValue)
    {
        String v = value.Cast<String>();
        String pv = prevValue.Cast<String>();
        M::ValidatorResult r;
        r.state = M::ValidatorResult::eState::Valid;

        if (v.size() > 20)
        {
            r.state = M::ValidatorResult::eState::Invalid;
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

    static Result Create(TArc::ContextAccessor* accessor, QWidget* parent)
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

    static Result Create(TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;
        ComboBoxTestData* data = new ComboBoxTestData();
        Reflection dataModel = Reflection::Create(data);

        QVBoxLayout* boxLayout = new QVBoxLayout();

        auto addTest = [&](const QString& testName, const ControlDescriptorBuilder<ComboBox::Fields> descriptor)
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
    QGridLayout* layout = new QGridLayout(dlg);
    dlg->setLayout(layout);

    DAVA::Vector<Node> nodes =
    {
      { &CheckBoxTestData::Create, "CheckBox Test" },
      { &LineEditTestData::Create, "LineEdit Test" },
      { &ComboBoxTestData::Create, "ComboBox Test" }
    };
    DAVA::Vector<DAVA::ReflectionBase*> data;

    const int columnCount = 6;
    int currentColumn = 0;
    int currentRow = 0;
    DAVA::TArc::ContextAccessor* accessor = GetAccessor();
    for (Node& node : nodes)
    {
        QGroupBox* groupBox = new QGroupBox(node.title, dlg);

        Result r = node.creator(accessor, groupBox);
        data.push_back(r.model);

        groupBox->setLayout(r.layout);
        layout->addWidget(groupBox, currentRow, currentColumn);
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
