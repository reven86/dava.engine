#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsListModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BranchTreeModel.h"

#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/BranchDiff.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"

#include "Qt/DeviceInfo/MemoryTool/Widgets/SymbolsWidget.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/FilterAndSortBar.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/MemoryBlocksWidget.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/SnapshotViewerWidget.h"

#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>

using namespace DAVA;

SnapshotViewerWidget::SnapshotViewerWidget(const ProfilingSession* session_, size_t snapshotIndex, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , session(session_)
{
    DVASSERT(session != nullptr);

    snapshot = &session->Snapshot(snapshotIndex);

    allBlocksLinked = BlockLink::CreateBlockLink(snapshot);

    Init();
}

SnapshotViewerWidget::~SnapshotViewerWidget() = default;

void SnapshotViewerWidget::Init()
{
    tab = new QTabWidget;

    InitMemoryBlocksView();
    InitSymbolsView();
    InitBranchView();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    setLayout(mainLayout);
}

void SnapshotViewerWidget::InitSymbolsView()
{
    symbolWidget = new SymbolsWidget(*snapshot->SymbolTable());
    QPushButton* buildTree = new QPushButton("Build tree");
    connect(buildTree, &QPushButton::clicked, this, &SnapshotViewerWidget::SymbolView_OnBuldTree);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(buildTree);
    layout->addWidget(symbolWidget);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);
    tab->addTab(frame, "Symbols");
}

void SnapshotViewerWidget::InitBranchView()
{
    branchTreeModel.reset(new BranchTreeModel(snapshot));

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    branchTree->setModel(branchTreeModel.get());

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &SnapshotViewerWidget::BranchView_SelectionChanged);

    branchBlocksWidget = new MemoryBlocksWidget(session, &branchBlockLinked, false);
    connect(branchBlocksWidget, &MemoryBlocksWidget::MemoryBlockDoubleClicked, this, &SnapshotViewerWidget::MemoryBlockDoubleClicked);

    QSplitter* splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(branchTree);
    splitter->addWidget(branchBlocksWidget);

    tab->addTab(splitter, "Branches");
}

void SnapshotViewerWidget::InitMemoryBlocksView()
{
    memoryBlocksWidget = new MemoryBlocksWidget(session, &allBlocksLinked);
    tab->addTab(memoryBlocksWidget, "Memory blocks");
}

void SnapshotViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const String*> selection = symbolWidget->GetSelectedSymbols();
    if (!selection.empty())
    {
        branchTreeModel->PrepareModel(selection);
        tab->setCurrentIndex(2);
    }
}

void SnapshotViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Branch* branch = static_cast<Branch*>(current.internalPointer());
    if (branch != nullptr)
    {
        branchBlocks = branch->GetMemoryBlocks();
        branchBlockLinked = BlockLink::CreateBlockLink(branchBlocks, snapshot);
        branchBlocksWidget->SetBlockLink(&branchBlockLinked);
    }
}

void SnapshotViewerWidget::MemoryBlockDoubleClicked(const BlockLink::Item& item)
{
    // TODO: expand callstack tree to view block allocation site
}
