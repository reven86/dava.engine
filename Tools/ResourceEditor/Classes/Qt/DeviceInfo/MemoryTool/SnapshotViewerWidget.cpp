#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "Qt/DeviceInfo/MemoryTool/SnapshotViewerWidget.h"

#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsListModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BranchTreeModel.h"

#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"
#include "Qt/DeviceInfo/MemoryTool/SymbolsWidget.h"
#include "Qt/DeviceInfo/MemoryTool/FilterAndSortBar.h"
#include "Qt/DeviceInfo/MemoryTool/MemoryBlocksWidget.h"

#include <QDebug>
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

SnapshotViewerWidget::~SnapshotViewerWidget()
{
    delete branchTreeModel;
}

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
    branchTreeModel = new BranchTreeModel(snapshot);
    branchFilterModel = new BranchFilterModel;
    branchFilterModel->setSourceModel(branchTreeModel);

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    //branchTree->setModel(branchTreeModel);
    branchTree->setModel(branchFilterModel);

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &SnapshotViewerWidget::BranchView_SelectionChanged);
    //connect(blockList, &QTreeView::doubleClicked, this, &SnapshotViewerWidget::BranchBlockView_DoubleClicked);

    branchBlocksWidget = new MemoryBlocksWidget(session, &branchBlockLinked, false);

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
        tab->setCurrentIndex(1);
    }
}

void SnapshotViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    QModelIndex index = branchFilterModel->mapToSource(current);

    Branch* branch = static_cast<Branch*>(index.internalPointer());
    if (branch != nullptr)
    {
        Vector<MMBlock*> blocks = branch->GetMemoryBlocks();
        branchBlockLinked = BlockLink::CreateBlockLink(blocks, snapshot);
        branchBlocksWidget->SetBlockLink(&branchBlockLinked);
    }
}

void SnapshotViewerWidget::BranchBlockView_DoubleClicked(const QModelIndex& current)
{
    //const MMBlock* block = blockListModel->GetBlock(current);
    //if (block != nullptr)
    //{
    //    // TODO: expand callstack tree to view block allocation site
    //}
}
