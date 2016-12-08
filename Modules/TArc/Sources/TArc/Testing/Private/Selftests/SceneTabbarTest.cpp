#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"

#include "TArc/Testing/MockClientModule.h"
#include "TArc/Testing/MockControllerModule.h"
#include "TArc/Controls/SceneTabbar.h"

#include "UnitTests/UnitTests.h"

#include <QtTest/QTest>

namespace SceneTabbarDetail
{
struct Tab
{
    DAVA::String title;

    bool operator==(const Tab& other) const
    {
        return title == other.title;
    }

    DAVA_REFLECTION(Tab)
    {
        DAVA::ReflectionRegistrator<Tab>::Begin()
        .Field(DAVA::TArc::SceneTabbar::tabTitlePropertyName, &Tab::title)
        .End();
    }
};

class TabsModel : public DAVA::TArc::DataNode
{
public:
    DAVA::uint64 activeTab = 0;
    DAVA::Map<DAVA::uint64, Tab> tabs;

    DAVA_VIRTUAL_REFLECTION(TabsModel, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<TabsModel>::Begin()
        .Field(DAVA::TArc::SceneTabbar::activeTabPropertyName, &TabsModel::activeTab)
        .Field(DAVA::TArc::SceneTabbar::tabsPropertyName, &TabsModel::tabs)
        .End();
    }
};

DAVA::TArc::WindowKey wndKey = DAVA::TArc::WindowKey(DAVA::FastName("SceneTabbarTest"));
QString tabbarObjectName = QString("tabbar");

class Tag
{
};

class TabbarModule : public DAVA::TArc::MockClientModule<Tag>
{
public:
    void OnContextCreatedImpl(DAVA::TArc::DataContext* context) override
    {
        TabsModel* model = GetAccessor()->GetGlobalContext()->GetData<TabsModel>();
        Tab tab;
        tab.title = QString::number(context->GetID()).toStdString();
        model->tabs[context->GetID()] = tab;
    }

    void OnContextDeletedImpl(DAVA::TArc::DataContext* context) override
    {
        TabsModel* model = GetAccessor()->GetGlobalContext()->GetData<TabsModel>();
        model->tabs.erase(context->GetID());
    }

    void OnContextWillBeChangedImpl(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne) override
    {
    }

    void OnContextWasChangedImpl(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne) override
    {
        TabsModel* model = GetAccessor()->GetGlobalContext()->GetData<TabsModel>();
        model->activeTab = current->GetID();
    }

    void PostInitImpl()
    {
        GetAccessor()->GetGlobalContext()->CreateData(std::make_unique<TabsModel>());

        DAVA::TArc::UI* ui = GetUI();

        DAVA::TArc::SceneTabbar* tabbar = new DAVA::TArc::SceneTabbar(GetAccessor(), DAVA::Reflection::Create(GetAccessor()->GetGlobalContext()->GetData<TabsModel>()));
        DAVA::TArc::PanelKey panelKey(tabbarObjectName, DAVA::TArc::CentralPanelInfo());
        ui->AddView(wndKey, panelKey, tabbar);
    }
};

class TestProxy
{
public:
    TestProxy(DAVA::TArc::TestClass* tstClass)
        : testClass(tstClass)
    {
    }

    virtual void Update()
    {
    }
    virtual bool IsCompleted()
    {
        return true;
    }

protected:
    DAVA::TArc::TestClass* testClass;
};

class CreateTabTestProxy : public TestProxy
{
public:
    CreateTabTestProxy(DAVA::TArc::TestClass* tstClass)
        : TestProxy(tstClass)
    {
        using namespace DAVA::TArc;

        ContextManager* mng = testClass->GetContextManager();
        contextID = mng->CreateContext(DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>>());
        mng->ActivateContext(contextID);
    }

    void Update() override
    {
        updateCount++;
        TEST_VERIFY(updateCount < updateLimit);
    }

    bool IsCompleted()
    {
        QList<QWidget*> w = testClass->LookupWidget(wndKey, tabbarObjectName);
        TEST_VERIFY(w.size() == 1);
        QTabBar* bar = qobject_cast<QTabBar*>(w.front());
        TEST_VERIFY(bar != nullptr);

        if (bar->count() == 0)
            return false;

        TEST_VERIFY(bar->count() == 1);
        TEST_VERIFY(bar->tabData(0).value<DAVA::uint64>() == contextID);
        TEST_VERIFY(bar->tabData(bar->currentIndex()).value<DAVA::uint64>() == contextID);
        return true;
    }

private:
    DAVA::TArc::DataContext::ContextID contextID = DAVA::TArc::DataContext::Empty;
    const DAVA::int32 updateLimit = 30;
    DAVA::int32 updateCount = 0;
};

class SwitchContextInCode : public TestProxy
{
public:
    SwitchContextInCode(DAVA::TArc::TestClass* tstClass)
        : TestProxy(tstClass)
    {
        TEST_VERIFY(testClass->GetAccessor()->GetActiveContext() != nullptr);
        firstContext = testClass->GetAccessor()->GetActiveContext()->GetID();
    }

    void Update() override
    {
        using namespace DAVA::TArc;
        switch (phase)
        {
        case 0:
            secondContext = testClass->GetContextManager()->CreateContext(DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>>());
            break;
        case 1:
            testClass->GetContextManager()->ActivateContext(secondContext);
            break;
        default:
            break;
        }
        updateCount++;
        TEST_VERIFY(updateCount < updateLimit);
    }

    bool IsCompleted()
    {
        QList<QWidget*> w = testClass->LookupWidget(wndKey, tabbarObjectName);
        TEST_VERIFY(w.size() == 1);
        QTabBar* bar = qobject_cast<QTabBar*>(w.front());
        TEST_VERIFY(bar != nullptr);

        switch (phase)
        {
        case 0:
            if (bar->count() == 1)
            {
                TEST_VERIFY(bar->tabData(0).value<DAVA::uint64>() == firstContext);
                TEST_VERIFY(bar->tabData(bar->currentIndex()).value<DAVA::uint64>() == firstContext);
            }
            else
            {
                TEST_VERIFY(bar->count() == 2);
                TEST_VERIFY(bar->tabData(0).value<DAVA::uint64>() == firstContext);
                TEST_VERIFY(bar->tabData(1).value<DAVA::uint64>() == secondContext);
                TEST_VERIFY(bar->tabData(bar->currentIndex()).value<DAVA::uint64>() == firstContext);
                NextPhase();
            }
            break;
        case 1:
            TEST_VERIFY(bar->count() == 2);
            TEST_VERIFY(bar->tabData(bar->currentIndex()).value<DAVA::uint64>() == secondContext);
            NextPhase();
            break;
        default:
            break;
        }

        return phase == 2;
    }

private:
    void NextPhase()
    {
        phase++;
    }
    int phase = 0;
    DAVA::TArc::DataContext::ContextID firstContext = DAVA::TArc::DataContext::Empty;
    DAVA::TArc::DataContext::ContextID secondContext = DAVA::TArc::DataContext::Empty;

    const DAVA::int32 updateLimit = 30;
    DAVA::int32 updateCount = 0;
};

class SwitchContextInGUI : public TestProxy
{
public:
    SwitchContextInGUI(DAVA::TArc::TestClass* tstClass)
        : TestProxy(tstClass)
    {
        QList<QWidget*> w = testClass->LookupWidget(wndKey, tabbarObjectName);
        TEST_VERIFY(w.size() == 1);
        QTabBar* bar = qobject_cast<QTabBar*>(w.front());
        TEST_VERIFY(bar != nullptr);

        TEST_VERIFY(bar->currentIndex() != 0);
        QRect tabRect = bar->tabRect(0);
        QTest::mouseClick(bar, Qt::LeftButton, Qt::KeyboardModifiers(), tabRect.center());
    }

    void Update() override
    {
        updateCount++;
        TEST_VERIFY(updateCount < updateLimit);
    }

    bool IsCompleted() override
    {
        QList<QWidget*> w = testClass->LookupWidget(wndKey, tabbarObjectName);
        TEST_VERIFY(w.size() == 1);
        QTabBar* bar = qobject_cast<QTabBar*>(w.front());
        TEST_VERIFY(bar != nullptr);

        TabsModel* model = testClass->GetAccessor()->GetGlobalContext()->GetData<TabsModel>();
        return model->activeTab == bar->tabData(bar->currentIndex()).value<DAVA::uint64>();
    }

private:
    const DAVA::int32 updateLimit = 30;
    DAVA::int32 updateCount = 0;
};

class TearDownTest : public TestProxy
{
public:
    TearDownTest(DAVA::TArc::TestClass* tstClass)
        : TestProxy(tstClass)
    {
    }

    void Update() override
    {
    }

    bool IsCompleted() override
    {
        return true;
    }
};
}

namespace DAVA
{
template <>
struct AnyCompare<DAVA::Map<DAVA::uint64, SceneTabbarDetail::Tab>>
{
    static bool IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
    {
        const DAVA::Map<DAVA::uint64, SceneTabbarDetail::Tab>& tabs1 = v1.Get<DAVA::Map<DAVA::uint64, SceneTabbarDetail::Tab>>();
        const DAVA::Map<DAVA::uint64, SceneTabbarDetail::Tab>& tabs2 = v2.Get<DAVA::Map<DAVA::uint64, SceneTabbarDetail::Tab>>();
        return tabs1 == tabs2;
    }
};
}

DAVA_TARC_TESTCLASS(SceneTabbarTest)
{
    DAVA_TEST (CreateTabTest)
    {
        using namespace ::testing;
        using namespace SceneTabbarDetail;

        TabbarModule* module = dynamic_cast<TabbarModule*>(TabbarModule::instance);
        ::testing::InSequence sequence;
        EXPECT_CALL(*module, OnContextCreated(_))
        .WillOnce(Invoke(module, &TabbarModule::OnContextCreatedImpl));
        EXPECT_CALL(*module, OnContextWillBeChanged(_, _))
        .WillOnce(Invoke(module, &TabbarModule::OnContextWillBeChangedImpl));
        EXPECT_CALL(*module, OnContextWasChanged(_, _))
        .WillOnce(Invoke(module, &TabbarModule::OnContextWasChangedImpl));

        test = new CreateTabTestProxy(this);
    }

    DAVA_TEST (SwitchContextInCodeTest)
    {
        using namespace ::testing;
        using namespace SceneTabbarDetail;

        TabbarModule* module = dynamic_cast<TabbarModule*>(TabbarModule::instance);
        ::testing::InSequence sequence;
        EXPECT_CALL(*module, OnContextCreated(_))
        .WillOnce(Invoke(module, &TabbarModule::OnContextCreatedImpl));
        EXPECT_CALL(*module, OnContextWillBeChanged(_, _))
        .WillOnce(Invoke(module, &TabbarModule::OnContextWillBeChangedImpl));
        EXPECT_CALL(*module, OnContextWasChanged(_, _))
        .WillOnce(Invoke(module, &TabbarModule::OnContextWasChangedImpl));

        test = new SwitchContextInCode(this);
    }

    DAVA_TEST (SwitchContextInGUITest)
    {
        using namespace SceneTabbarDetail;
        test = new SwitchContextInGUI(this);
    }

    DAVA_TEST (TearDownContextTest)
    {
        using namespace ::testing;
        using namespace SceneTabbarDetail;

        TabbarModule* module = dynamic_cast<TabbarModule*>(TabbarModule::instance);
        InSequence sequence;
        EXPECT_CALL(*module, OnContextWillBeChanged(_, nullptr))
        .WillOnce(Return());
        EXPECT_CALL(*module, OnContextWasChanged(nullptr, _))
        .WillOnce(Return());
        EXPECT_CALL(*module, OnContextDeleted(_))
        .Times(2);
        test = new TearDownTest(this);
    }

    void Update(DAVA::float32 timeElapsed, const DAVA::String& testName) override
    {
        DAVA::TArc::TestClass::Update(timeElapsed, testName);
        test->Update();
    }

    void TearDown(const DAVA::String& testName) override
    {
        DAVA::TArc::TestClass::TearDown(testName);
        delete test;
        test = nullptr;
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        return test->IsCompleted();
    }

    SceneTabbarDetail::TestProxy* test = nullptr;

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(DAVA::TArc::MockControllerModule);
    DECLARE_TESTED_MODULE(SceneTabbarDetail::TabbarModule);
    END_TESTED_MODULES()
};