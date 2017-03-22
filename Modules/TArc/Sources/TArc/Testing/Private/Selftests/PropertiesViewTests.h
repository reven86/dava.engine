#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Controls/PropertyPanel/Private/ReflectionPathTree.h"
#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <Base/BaseTypes.h>

#include <QTreeView>
#include <QAbstractItemModel>

#include <gmock/gmock.h>

namespace PropertiesViewTestsDetail
{
using namespace DAVA;

struct SubChild
{
    float32 v;
    DAVA_REFLECTION(SubChild);
};

DAVA_REFLECTION_IMPL(SubChild)
{
    ReflectionRegistrator<SubChild>::Begin()
    .Field("v", &SubChild::v)
    .End();
}

struct ChildNode
{
    int32 value;
    DAVA_REFLECTION(ChildNode);
};

DAVA_REFLECTION_IMPL(ChildNode)
{
    ReflectionRegistrator<ChildNode>::Begin()
    .Field("value", &ChildNode::value)
    .End();
}

struct RootNode
{
    int32 i;
    float32 f;
    String s;

    ChildNode child1;
    ChildNode child2;

    DAVA_REFLECTION(RootNode);
};

DAVA_REFLECTION_IMPL(RootNode)
{
    ReflectionRegistrator<RootNode>::Begin()
    .Field("i", &RootNode::i)
    .Field("f", &RootNode::f)
    .Field("s", &RootNode::s)
    .Field("child1", &RootNode::child1)
    .Field("child2", &RootNode::child2)
    .End();
}

WindowKey wnd = WindowKey(FastName("PropertiesViewTests"));

class SelectionData : public DAVA::TArc::DataNode
{
public:
    Vector<Reflection> selection;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SelectionData, DAVA::TArc::DataNode)
    {
        ReflectionRegistrator<SelectionData>::Begin()
        .Field("objects", &SelectionData::selection)
        .End();
    }
};

class TestModule : public DAVA::TArc::ClientModule
{
    void PostInit() override
    {
        using namespace DAVA::TArc;

        DataContext* ctx = GetAccessor()->GetGlobalContext();
        ctx->CreateData(std::make_unique<PropertiesViewTestsDetail::SelectionData>());

        PropertiesView::Params params(PropertiesViewTestsDetail::wnd);
        params.accessor = GetAccessor();
        params.invoker = GetInvoker();
        params.ui = GetUI();
        params.objectsField.type = ReflectedTypeDB::Get<PropertiesViewTestsDetail::SelectionData>();
        params.objectsField.fieldName = FastName("objects");
        params.settingsNodeName = "PropertyPanel";

        PropertiesView* view = new PropertiesView(params);

        PanelKey key("PropertiesView", CentralPanelInfo());
        GetUI()->AddView(PropertiesViewTestsDetail::wnd, key, view);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestModule, DAVA::TArc::ClientModule)
    {
        ReflectionRegistrator<TestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
}

DAVA_TARC_TESTCLASS(PropertiesViewTests)
{
    void WriteInitialSettings() override
    {
        using namespace DAVA;
        ReflectionPathTree tree(FastName("Root"));
        tree.AddLeaf(List<FastName>{ FastName("child1"), FastName("value") });
        tree.AddLeaf(List<FastName>{ FastName("child2") });

        const EngineContext* ctx = GetEngineContext();

        PropertiesHolder holder("TArcProperties", ctx->fileSystem->GetCurrentDocumentsDirectory());
        {
            PropertiesItem propertyPanel = holder.CreateSubHolder("PropertyPanel");
            PropertiesItem expanded = propertyPanel.CreateSubHolder("expandedItems");
            tree.Save(expanded);
        }
        holder.SaveToFile();
    }

    DAVA_TEST (ReadExpandedList)
    {
        using namespace DAVA::TArc;
        using namespace ::testing;

        DataContext* ctx = GetGlobalContext();
        PropertiesViewTestsDetail::SelectionData* data = ctx->GetData<PropertiesViewTestsDetail::SelectionData>();
        data->selection.push_back(DAVA::Reflection::Create(&node));

        auto verifyTestFn = [this]()
        {
            QTreeView* view = LookupSingleWidget<QTreeView>(PropertiesViewTestsDetail::wnd, "PropertyPanel_propertiesview");
            TEST_VERIFY(view != nullptr);

            QAbstractItemModel* model = view->model();
            TEST_VERIFY(model->columnCount(QModelIndex()) == 2);
            TEST_VERIFY(model->rowCount(QModelIndex()) == 5);

            QModelIndex child1Index = model->index(3, 0, QModelIndex());
            TEST_VERIFY(view->isExpanded(child1Index) == true);
            QModelIndex child1ValueIndex = model->index(0, 0, child1Index);
            TEST_VERIFY(view->isExpanded(child1ValueIndex) == true);

            QModelIndex child2Index = model->index(4, 0, QModelIndex());
            TEST_VERIFY(view->isExpanded(child2Index) == true);
            QModelIndex child2ValueIndex = model->index(0, 0, child2Index);
            TEST_VERIFY(view->isExpanded(child2ValueIndex) == false);

            view->setExpanded(child1Index, false);

            DataContext* ctx = GetGlobalContext();
            PropertiesViewTestsDetail::SelectionData* data = ctx->GetData<PropertiesViewTestsDetail::SelectionData>();
            data->selection.clear();
        };

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(verifyTestFn));
    }

    DAVA_TEST (SetOtherObjectTest)
    {
        using namespace DAVA::TArc;
        using namespace ::testing;

        DataContext* ctx = GetGlobalContext();
        PropertiesViewTestsDetail::SelectionData* data = ctx->GetData<PropertiesViewTestsDetail::SelectionData>();
        data->selection.push_back(DAVA::Reflection::Create(&node));

        auto verifyTestFn = [this]()
        {
            QTreeView* view = LookupSingleWidget<QTreeView>(PropertiesViewTestsDetail::wnd, "PropertyPanel_propertiesview");
            TEST_VERIFY(view != nullptr);

            QAbstractItemModel* model = view->model();
            TEST_VERIFY(model->columnCount(QModelIndex()) == 2);
            TEST_VERIFY(model->rowCount(QModelIndex()) == 5);

            QModelIndex child2Index = model->index(4, 0, QModelIndex());
            TEST_VERIFY(view->isExpanded(child2Index) == true);
            QModelIndex child2ValueIndex = model->index(0, 0, child2Index);
            TEST_VERIFY(view->isExpanded(child2ValueIndex) == false);

            QModelIndex child1Index = model->index(3, 0, QModelIndex());
            TEST_VERIFY(view->isExpanded(child1Index) == false);
            QModelIndex child1ValueIndex = model->index(0, 0, child1Index);
            TEST_VERIFY(view->isExpanded(child1ValueIndex) == false);

            view->setExpanded(child1Index, true);

            TEST_VERIFY(view->isExpanded(child1Index) == true);
            TEST_VERIFY(view->isExpanded(child1ValueIndex) == true);
        };

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(verifyTestFn));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    PropertiesViewTestsDetail::RootNode node;

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(PropertiesViewTestsDetail::TestModule);
    END_TESTED_MODULES()
};
