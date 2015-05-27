#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "SnapshotViewerWidget.h"

#include "Classes/Qt/DeviceInfo/DeviceInfo/Models/SymbolsTreeModel.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/Models/SymbolsFilterModel.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/Models/BranchTreeModel.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/Models/BlockListModel.h"

#include "Classes/Qt/DeviceInfo/DeviceInfo/Branch.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/ProfilingSession.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/MemorySnapshot.h"

#include <QDebug>
#include <QFileDialog>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QAction>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QLineEdit>
#include <QListView>

using namespace DAVA;

SnapshotViewerWidget::SnapshotViewerWidget(const MemorySnapshot* snapshot_, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , snapshot(snapshot_)
{
    DVASSERT(snapshot != nullptr && snapshot->IsLoaded());
    Init();
}

SnapshotViewerWidget::~SnapshotViewerWidget()
{
    delete symbolsFilterModel;
    delete symbolsTreeModel;
    delete branchTreeModel;
    delete blockListModel;
}

void SnapshotViewerWidget::Init()
{
    tab = new QTabWidget;

    InitSymbolsView();
    InitBranchView();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    setLayout(mainLayout);
}

void SnapshotViewerWidget::InitSymbolsView()
{
    symbolsTreeModel = new SymbolsTreeModel(*snapshot->SymbolTable());
    symbolsFilterModel = new SymbolsFilterModel;
    symbolsFilterModel->setSourceModel(symbolsTreeModel);
    symbolsFilterModel->sort(0);

    symbolsTree = new QTreeView;
    symbolsTree->setFont(QFont("Consolas", 10, 500));
    symbolsTree->setSelectionMode(QAbstractItemView::ContiguousSelection);
    symbolsTree->setModel(symbolsFilterModel);

    QLineEdit* filter = new QLineEdit;
    QPushButton* toggleStd = new QPushButton("Toggle 'std::' on/off");
    QPushButton* buildTree = new QPushButton("Build tree");

    connect(filter, &QLineEdit::textChanged, symbolsFilterModel, &SymbolsFilterModel::SetFilterString);
    connect(toggleStd, &QPushButton::clicked, symbolsFilterModel, &SymbolsFilterModel::ToggleStd);
    connect(buildTree, &QPushButton::clicked, this, &SnapshotViewerWidget::SymbolView_OnBuldTree);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(filter);
    layout->addWidget(toggleStd);
    layout->addWidget(buildTree);
    layout->addWidget(symbolsTree);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);
    tab->addTab(frame, "Symbols");
}

void SnapshotViewerWidget::InitBranchView()
{
    branchTreeModel = new BranchTreeModel(snapshot);
    blockListModel = new BlockListModel;

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    branchTree->setModel(branchTreeModel);

    blockList = new QListView;
    blockList->setFont(QFont("Consolas", 10, 500));
    blockList->setModel(blockListModel);

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &SnapshotViewerWidget::BranchView_SelectionChanged);
    connect(blockList, &QTreeView::doubleClicked, this, &SnapshotViewerWidget::BranchBlockView_DoubleClicked);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(branchTree);
    layout->addWidget(blockList);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);

    tab->addTab(frame, "Branches");
}

void SnapshotViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const char*> selection = GetSelectedSymbols();
    if (!selection.empty())
    {
        branchTreeModel->PrepareModel(selection);
    }
}

void SnapshotViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Branch* branch = static_cast<Branch*>(current.internalPointer());
    Vector<MMBlock> blocks = branch->GetMemoryBlocks();

    blockListModel->PrepareModel(std::forward<Vector<MMBlock>>(blocks));
}

void SnapshotViewerWidget::BranchBlockView_DoubleClicked(const QModelIndex& current)
{
    const MMBlock* block = blockListModel->GetBlock(current);
    if (block != nullptr)
    {
        /*Vector<QModelIndex> v = branchTreeModel->Select2(p);
        if (!v.empty())
        {
            for (auto& x : v)
                branchTree->expand(x);
            branchTree->setCurrentIndex(v.back());
            branchTree->selectionModel()->select(v.back(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }*/
    }
}

Vector<const char*> SnapshotViewerWidget::GetSelectedSymbols()
{
    Vector<const char*> result;
    QItemSelectionModel* selModel = symbolsTree->selectionModel();
    if (selModel->hasSelection())
    {
        QModelIndexList list = selModel->selectedRows(0);
        for (auto x : list)
        {
            QModelIndex index = symbolsFilterModel->mapToSource(x);
            if (index.isValid())
            {
                GenericTreeNode* p = static_cast<GenericTreeNode*>(index.internalPointer());
                if (p->Type() == SymbolsTreeModel::TYPE_NAME)
                {
                    SymbolsTreeModel::NameNode* node = static_cast<SymbolsTreeModel::NameNode*>(p);
                    result.push_back(node->Name());
                }
            }
        }
    }
    return result;
}
