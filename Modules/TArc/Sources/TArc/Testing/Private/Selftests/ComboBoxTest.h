#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QtTest>
#include <QComboBox>
#include <QStyle>
#include <QStyleOption>

#include <QAbstractItemView>
#include <QDebug>

namespace ComboBoxTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("ComboBoxTestWnd");

class ComboBoxTestModule : public DAVA::TArc::ClientModule
{
public:
    enum eTestedValue
    {
        first = 10,
        second,
        third
    };

    class TestModel : public ReflectionBase
    {
    public:
        int value = eTestedValue::second;

        DAVA::UnorderedMap<int, DAVA::FastName> enumeratorUnordered = DAVA::UnorderedMap<int, DAVA::FastName>
        {
          { third, DAVA::FastName("third") },
          { second, DAVA::FastName("second") },
          { first, DAVA::FastName("first") },
        };

        DAVA::Map<int, DAVA::String> enumeratorOrdered = DAVA::Map<int, DAVA::String>
        {
          { third, "third" },
          { second, "second" },
          { first, "first" }
        };

        DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestModel, ReflectionBase)
        {
            DAVA::ReflectionRegistrator<TestModel>::Begin()
            .Field("value", &TestModel::value)
            .Field("enumeratorUnordered", &TestModel::enumeratorUnordered)
            .Field("enumeratorOrdered", &TestModel::enumeratorOrdered)
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

        {
            ControlDescriptorBuilder<ComboBox::Fields> descriptor;
            descriptor[ComboBox::Fields::Value] = "value";
            descriptor[ComboBox::Fields::Enumerator] = "enumeratorUnordered";
            ComboBox* comboBox = new ComboBox(descriptor, GetAccessor(), reflectedModel);
            DAVA::TArc::PanelKey panelKey("ComboBoxUnordered", DAVA::TArc::CentralPanelInfo());
            GetUI()->AddView(wndKey, panelKey, comboBox->ToWidgetCast());
        }

        {
            ControlDescriptorBuilder<ComboBox::Fields> descriptor;
            descriptor[ComboBox::Fields::Value] = "value";
            descriptor[ComboBox::Fields::Enumerator] = "enumeratorOrdered";
            ComboBox* comboBox = new ComboBox(descriptor, GetAccessor(), reflectedModel);
            DAVA::TArc::PanelKey panelKey("ComboBoxOrdered", DAVA::TArc::CentralPanelInfo());
            GetUI()->AddView(wndKey, panelKey, comboBox->ToWidgetCast());
        }
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

DAVA_TARC_TESTCLASS(ComboBoxTest)
{
    // disabled because of modal loop of QComboBox
    //    DAVA_TEST (ComboTestUnordered)
    //    {
    //        using namespace ComboBoxTestDetails;
    //
    //        QList<QWidget*> widgets = LookupWidget(wndKey, QString("ComboBoxUnordered"));
    //        TEST_VERIFY(widgets.size() == 1);
    //        QWidget* w = widgets.front();
    //
    //        QStyle* style = w->style();
    //        QStyleOptionButton option;
    //        option.initFrom(w);
    //        QRect r = style->subElementRect(QStyle::SE_ComboBoxLayoutItem, &option, w);
    //
    //        QTestEventList eventList;
    //        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), r.center());
    //
    //        QComboBox* comboBox = qobject_cast<QComboBox*>(w);
    //        TEST_VERIFY(comboBox != nullptr);
    //        TEST_VERIFY(comboBox->currentIndex() == 1);
    //
    //        ComboBoxTestModule* inst = ComboBoxTestModule::instance;
    //        TEST_VERIFY(inst->model.value == ComboBoxTestModule::eTestedValue::second);
    //
    //        eventList.simulate(w);
    //
    //        QTestEventList eventListPopup;
    //        eventListPopup.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers());
    //        TEST_VERIFY(comboBox != nullptr);
    //        TEST_VERIFY(comboBox->currentIndex() == 1);
    //        TEST_VERIFY(inst->model.value == ComboBoxTestModule::eTestedValue::second);
    //
    //        eventListPopup.simulate(comboBox->view());
    //        TEST_VERIFY(comboBox->currentIndex() == 2);
    //        TEST_VERIFY(inst->model.value == ComboBoxTestModule::eTestedValue::third);
    //    }

    DAVA_TEST (ComboModuleTestUnordered)
    {
        using namespace ComboBoxTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("ComboBoxUnordered"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QComboBox* checkBox = qobject_cast<QComboBox*>(w);
        connections.AddConnection(checkBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), DAVA::MakeFunction(this, &ComboBoxTest::IndexChangedUnordered));

        EXPECT_CALL(*this, IndexChangedUnordered(ComboBoxTestModule::eTestedValue::third - ComboBoxTestModule::eTestedValue::first))
        .WillOnce(Return());

        ComboBoxTestModule::instance->model.value = ComboBoxTestModule::eTestedValue::third;
    }

    DAVA_TEST (ComboModuleTestOrdered)
    {
        using namespace ComboBoxTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("ComboBoxOrdered"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QComboBox* checkBox = qobject_cast<QComboBox*>(w);
        connections.AddConnection(checkBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), DAVA::MakeFunction(this, &ComboBoxTest::IndexChangedOrdered));

        EXPECT_CALL(*this, IndexChangedOrdered(ComboBoxTestModule::eTestedValue::second - ComboBoxTestModule::eTestedValue::first))
        .WillOnce(Return());

        ComboBoxTestModule::instance->model.value = ComboBoxTestModule::eTestedValue::second;
    }

    MOCK_METHOD1_VIRTUAL(IndexChangedOrdered, void(int newCurrentItem));
    MOCK_METHOD1_VIRTUAL(IndexChangedUnordered, void(int newCurrentItem));

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ComboBoxTestDetails::ComboBoxTestModule);
    END_TESTED_MODULES()

    DAVA::TArc::QtConnections connections;
};
