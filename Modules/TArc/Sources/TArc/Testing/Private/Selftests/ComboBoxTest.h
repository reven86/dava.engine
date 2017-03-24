#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/GlobalEnum.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QtTest>
#include <QComboBox>
#include <QStyle>
#include <QStyleOption>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QWidget>

namespace ComboBoxTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("ComboBoxTestWnd");

class ComboBoxTestModule : public DAVA::TArc::ClientModule
{
public:
    enum eTestedValue
    {
        first = 0,
        second,
        third,
        count
    };

    class TestModel : public ReflectionBase
    {
    public:
        int value = eTestedValue::first;

        DAVA::UnorderedMap<int, DAVA::FastName> enumeratorUnorderedMap = DAVA::UnorderedMap<int, DAVA::FastName>
        {
          { first, DAVA::FastName("first") },
          { second, DAVA::FastName("second") },
          { third, DAVA::FastName("third") }
        };

        DAVA::Map<int, DAVA::String> enumeratorOrderedMap = DAVA::Map<int, DAVA::String>
        {
          { third, "third" },
          { second, "second" },
          { first, "first" }
        };

        DAVA::Vector<DAVA::String> enumeratorVector = DAVA::Vector<DAVA::String>
        {
          "first", "second", "third"
        };

        DAVA::Set<DAVA::String> enumeratorSet = DAVA::Set<DAVA::String>
        {
          "first", "second", "third"
        };

        size_t GetSize_tValue() const
        {
            return value;
        }

        void SetSize_tValue(size_t newValue)
        {
            value = static_cast<int>(newValue);
        }

        const DAVA::String GetStringValue() const
        {
            auto it = enumeratorSet.begin();
            std::advance(it, value);
            return *it;
        }

        void SetStringValue(const DAVA::String& newStringValue)
        {
            auto it = enumeratorSet.begin();
            for (int i = 0; i < static_cast<int>(enumeratorSet.size()); ++i, ++it)
            {
                if (newStringValue == *it)
                {
                    value = i;
                    break;
                }
            }
        }

        DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestModel, ReflectionBase)
        {
            DAVA::ReflectionRegistrator<TestModel>::Begin()
            .Field("valueMeta", &TestModel::value)[DAVA::M::EnumT<ComboBoxTestModule::eTestedValue>()]
            .Field("value", &TestModel::value)
            .Field("valueSize_t", &TestModel::GetSize_tValue, &TestModel::SetSize_tValue)
            .Field("valueString", &TestModel::GetStringValue, &TestModel::SetStringValue)
            .Field("enumeratorUnorderedMap", &TestModel::enumeratorUnorderedMap)
            .Field("enumeratorOrderedMap", &TestModel::enumeratorOrderedMap)
            .Field("enumeratorVector", &TestModel::enumeratorVector)
            .Field("enumeratorSet", &TestModel::enumeratorSet)
            .End();
        }
    };

    TestModel model;

    ComboBoxTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;

        DAVA::Reflection reflectedModel = DAVA::Reflection::Create(&model);

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        {
            ControlDescriptorBuilder<ComboBox::Fields> descriptor;
            descriptor[ComboBox::Fields::Value] = "valueMeta";
            ComboBox* comboBox = new ComboBox(descriptor, GetAccessor(), reflectedModel);
            comboBox->SetObjectName("ComboBoxMeta");
            layout->AddControl(comboBox);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> descriptor;
            descriptor[ComboBox::Fields::Value] = "value";
            descriptor[ComboBox::Fields::Enumerator] = "enumeratorUnorderedMap";
            ComboBox* comboBox = new ComboBox(descriptor, GetAccessor(), reflectedModel);
            comboBox->SetObjectName("ComboBoxUnorderedMap");
            layout->AddControl(comboBox);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> descriptor;
            descriptor[ComboBox::Fields::Value] = "value";
            descriptor[ComboBox::Fields::Enumerator] = "enumeratorOrderedMap";
            ComboBox* comboBox = new ComboBox(descriptor, GetAccessor(), reflectedModel);
            comboBox->SetObjectName("ComboBoxOrderedMap");
            layout->AddControl(comboBox);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> descriptor;
            descriptor[ComboBox::Fields::Value] = "valueSize_t";
            descriptor[ComboBox::Fields::Enumerator] = "enumeratorVector";
            ComboBox* comboBox = new ComboBox(descriptor, GetAccessor(), reflectedModel);
            comboBox->SetObjectName("ComboBoxVector");
            layout->AddControl(comboBox);
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> descriptor;
            descriptor[ComboBox::Fields::Value] = "valueString";
            descriptor[ComboBox::Fields::Enumerator] = "enumeratorSet";
            ComboBox* comboBox = new ComboBox(descriptor, GetAccessor(), reflectedModel);
            comboBox->SetObjectName("ComboBoxSet");
            layout->AddControl(comboBox);
        }

        DAVA::TArc::PanelKey panelKey("ComboBoxTest", DAVA::TArc::CentralPanelInfo());
        GetUI()->AddView(wndKey, panelKey, w);
    }

    static ComboBoxTestModule* instance;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ComboBoxTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<ComboBoxTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

ComboBoxTestModule* ComboBoxTestModule::instance = nullptr;
}

ENUM_DECLARE(ComboBoxTestDetails::ComboBoxTestModule::eTestedValue)
{
    ENUM_ADD_DESCR(static_cast<int>(ComboBoxTestDetails::ComboBoxTestModule::eTestedValue::first), "1st");
    ENUM_ADD_DESCR(static_cast<int>(ComboBoxTestDetails::ComboBoxTestModule::eTestedValue::second), "2nd");
    ENUM_ADD_DESCR(static_cast<int>(ComboBoxTestDetails::ComboBoxTestModule::eTestedValue::third), "3rd");
}

DAVA_TARC_TESTCLASS(ComboBoxTest)
{
    DAVA::Vector<std::pair<QString, DAVA::Vector<int>>> comboTestData;
    int currentTest = 0;

    void InitTestData()
    {
        using namespace ComboBoxTestDetails;

        comboTestData.clear();
        comboTestData.reserve(5);

        DAVA::Vector<int> unordered;
        ComboBoxTestModule* inst = ComboBoxTestModule::instance;
        for (const std::pair<int, DAVA::FastName>& un : inst->model.enumeratorUnorderedMap)
        {
            unordered.push_back(un.first);
        }
        comboTestData.push_back({ "ComboBoxMeta", { ComboBoxTestModule::eTestedValue::first, ComboBoxTestModule::eTestedValue::second, ComboBoxTestModule::eTestedValue::third } });
        comboTestData.push_back({ "ComboBoxUnorderedMap", unordered });
        comboTestData.push_back({ "ComboBoxOrderedMap", { ComboBoxTestModule::eTestedValue::first, ComboBoxTestModule::eTestedValue::second, ComboBoxTestModule::eTestedValue::third } });
        comboTestData.push_back({ "ComboBoxVector", { ComboBoxTestModule::eTestedValue::first, ComboBoxTestModule::eTestedValue::second, ComboBoxTestModule::eTestedValue::third } });
        comboTestData.push_back({ "ComboBoxSet", { ComboBoxTestModule::eTestedValue::first, ComboBoxTestModule::eTestedValue::second, ComboBoxTestModule::eTestedValue::third } });
    }

    QComboBox* GetComboBox(const QString& comboName)
    {
        using namespace ComboBoxTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, comboName);
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QComboBox* comboBox = qobject_cast<QComboBox*>(w);
        TEST_VERIFY(comboBox != nullptr);

        return comboBox;
    }

    void TestComboEventsEnd()
    {
        using namespace ComboBoxTestDetails;

        const QString& comboName = comboTestData[currentTest].first;
        const DAVA::Vector<int>& testedValues = comboTestData[currentTest].second;

        QTestEventList eventList;
        eventList.addKeyPress(Qt::Key_Down);

        QComboBox* comboBox = GetComboBox(comboName);
        ComboBoxTestModule* inst = ComboBoxTestModule::instance;

        TEST_VERIFY(comboBox->currentIndex() == 0);
        TEST_VERIFY(inst->model.value == testedValues[comboBox->currentIndex()]);

        eventList.simulate(comboBox);
        TEST_VERIFY(comboBox->currentIndex() == 1);
        TEST_VERIFY(inst->model.value == testedValues[comboBox->currentIndex()]);

        eventList.simulate(comboBox);
        TEST_VERIFY(comboBox->currentIndex() == 2);
        TEST_VERIFY(inst->model.value == testedValues[comboBox->currentIndex()]);

        ++currentTest;
        if (currentTest < static_cast<int>(comboTestData.size()))
        {
            TestComboEventsStart();
        }
    }

    void TestComboEventsStart()
    {
        using namespace ComboBoxTestDetails;
        using namespace testing;

        const DAVA::Vector<int>& testedValues = comboTestData[currentTest].second;
        TEST_VERIFY(static_cast<int>(testedValues.size()) == ComboBoxTestModule::eTestedValue::count);

        ComboBoxTestModule* inst = ComboBoxTestModule::instance;
        inst->model.value = testedValues[0];

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &ComboBoxTest::TestComboEventsEnd));
    }

    DAVA_TEST (ShouldBeFirst_ComboTest)
    {
        currentTest = 0;
        InitTestData();

        TestComboEventsStart();
    }

    void TestComboModuleStart()
    {
        using namespace ComboBoxTestDetails;
        using namespace testing;

        const QString& comboName = comboTestData[currentTest].first;
        QComboBox* comboBox = GetComboBox(comboName);
        connections.AddConnection(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), DAVA::MakeFunction(this, &ComboBoxTest::IndexChanged));

        ComboBoxTestModule::instance->model.value = (ComboBoxTestModule::instance->model.value + 1) % ComboBoxTestModule::eTestedValue::count;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &ComboBoxTest::TestComboModuleEnd));
    }

    void TestComboModuleEnd()
    {
        const QString& comboName = comboTestData[currentTest].first;
        QComboBox* comboBox = GetComboBox(comboName);
        connections.RemoveConnection(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged));

        ++currentTest;
        if (currentTest < static_cast<int>(comboTestData.size()))
        {
            TestComboModuleStart();
        }
    }

    DAVA_TEST (ShouldBeSecond_ComboTest)
    {
        currentTest = 0;
        InitTestData();

        using namespace testing;

        //we should wait untill all dataModels and controls will by syncronized
        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &ComboBoxTest::TestComboModuleStart));

        EXPECT_CALL(*this, IndexChanged(_))
        .Times(static_cast<int>(comboTestData.size()));
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        bool testCompleted = true;

        if (testName == "ShouldBeFirst_ComboTest")
        {
            testCompleted = (currentTest == static_cast<int>(comboTestData.size()));
        }
        else if (testName == "ShouldBeSecond_ComboTest")
        {
            testCompleted = (currentTest == static_cast<int>(comboTestData.size()));
        }
        return testCompleted;
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());
    MOCK_METHOD1_VIRTUAL(IndexChanged, void(int newCurrentItem));

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ComboBoxTestDetails::ComboBoxTestModule);
    END_TESTED_MODULES()

    DAVA::TArc::QtConnections connections;
};
