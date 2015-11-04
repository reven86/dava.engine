#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsListModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BranchDiffTreeModel.h"

#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/BranchDiff.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"

#include "Qt/DeviceInfo/MemoryTool/Widgets/SymbolsWidget.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/FilterAndSortBar.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/MemoryBlocksWidget.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/SnapshotDiffViewerWidget.h"

#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>

using namespace DAVA;

SnapshotDiffViewerWidget::SnapshotDiffViewerWidget(const ProfilingSession* session_, size_t snapshotIndex1, size_t snapshotIndex2, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , session(session_)
{
    DVASSERT(session != nullptr);

    snapshot1 = &session->Snapshot(snapshotIndex1);
    snapshot2 = &session->Snapshot(snapshotIndex2);

    allBlocksLinked = BlockLink::CreateBlockLink(snapshot1, snapshot2);

    Init();
}

SnapshotDiffViewerWidget::~SnapshotDiffViewerWidget() = default;

void SnapshotDiffViewerWidget::Init()
{
    tab = new QTabWidget;

    InitMemoryBlocksView();
    InitSymbolsView();
    InitBranchView();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    setLayout(mainLayout);
}

void SnapshotDiffViewerWidget::InitSymbolsView()
{
    symbolWidget = new SymbolsWidget(*snapshot1->SymbolTable());
    QPushButton* buildTree = new QPushButton("Build tree");
    connect(buildTree, &QPushButton::clicked, this, &SnapshotDiffViewerWidget::SymbolView_OnBuldTree);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(buildTree);
    layout->addWidget(symbolWidget);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);
    tab->addTab(frame, "Symbols");
}

void SnapshotDiffViewerWidget::InitBranchView()
{
    branchTreeModel.reset(new BranchDiffTreeModel(snapshot1, snapshot2));

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    branchTree->setModel(branchTreeModel.get());

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &SnapshotDiffViewerWidget::BranchView_SelectionChanged);

    branchBlocksWidget = new MemoryBlocksWidget(session, &branchBlockLinked, false);
    connect(branchBlocksWidget, &MemoryBlocksWidget::MemoryBlockDoubleClicked, this, &SnapshotDiffViewerWidget::MemoryBlockDoubleClicked);

    QSplitter* splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(branchTree);
    splitter->addWidget(branchBlocksWidget);

    tab->addTab(splitter, "Branches");
}

void SnapshotDiffViewerWidget::InitMemoryBlocksView()
{
    memoryBlocksWidget = new MemoryBlocksWidget(session, &allBlocksLinked);
    tab->addTab(memoryBlocksWidget, "Memory blocks");
}

void SnapshotDiffViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const String*> selection = symbolWidget->GetSelectedSymbols();
    if (!selection.empty())
    {
        branchTreeModel->PrepareModel(selection);
        tab->setCurrentIndex(2);
    }
}

void SnapshotDiffViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    BranchDiff* branchDiff = static_cast<BranchDiff*>(current.internalPointer());

    branchBlocks1.clear();
    branchBlocks2.clear();

    if (branchDiff->left != nullptr)
    {
        branchBlocks1 = branchDiff->left->GetMemoryBlocks();
    }
    if (branchDiff->right)
    {
        branchBlocks2 = branchDiff->right->GetMemoryBlocks();
    }
    branchBlockLinked = BlockLink::CreateBlockLink(branchBlocks1, snapshot1, branchBlocks2, snapshot2);
    branchBlocksWidget->SetBlockLink(&branchBlockLinked);
}

void SnapshotDiffViewerWidget::MemoryBlockDoubleClicked(const BlockLink::Item& item)
{
    // TODO: expand callstack tree to view block allocation site
}
