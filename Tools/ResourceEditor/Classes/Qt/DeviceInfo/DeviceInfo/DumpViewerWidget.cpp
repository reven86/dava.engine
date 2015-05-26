#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "DumpViewerWidget.h"

#include "Models/SymbolsTreeModel.h"
#include "Models/SymbolsFilterModel.h"
#include "Models/BranchTreeModel.h"
#include "Models/BlockListModel.h"

#include "ProfilingSession.h"
#include "Branch.h"
#include "MemoryDump.h"

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

DumpViewerWidget::DumpViewerWidget(const MemorySnapshot& brief, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , dumpBrief(brief)
    , memoryDump(brief.Dump())
{
    DVASSERT(memoryDump != nullptr);
    Init();
}

DumpViewerWidget::~DumpViewerWidget()
{
    delete symbolsFilterModel;
    delete symbolsTreeModel;
    delete branchTreeModel;
    delete blockListModel;
}

void DumpViewerWidget::Init()
{
    tab = new QTabWidget;

    InitSymbolsView();
    InitBranchView();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    setLayout(mainLayout);
}

void DumpViewerWidget::InitSymbolsView()
{
    symbolsTreeModel = new SymbolsTreeModel(memoryDump->SymbolTable());
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
    connect(buildTree, &QPushButton::clicked, this, &DumpViewerWidget::SymbolView_OnBuldTree);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(filter);
    layout->addWidget(toggleStd);
    layout->addWidget(buildTree);
    layout->addWidget(symbolsTree);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);
    tab->addTab(frame, "Symbols");
}

void DumpViewerWidget::InitBranchView()
{
    branchTreeModel = new BranchTreeModel(memoryDump);
    blockListModel = new BlockListModel;

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    branchTree->setModel(branchTreeModel);

    blockList = new QListView;
    blockList->setFont(QFont("Consolas", 10, 500));
    blockList->setModel(blockListModel);

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &DumpViewerWidget::BranchView_SelectionChanged);
    connect(blockList, &QTreeView::doubleClicked, this, &DumpViewerWidget::BranchBlockView_DoubleClicked);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(branchTree);
    layout->addWidget(blockList);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);

    tab->addTab(frame, "Branches");
}

void DumpViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const char*> selection = GetSelectedSymbols();
    if (!selection.empty())
    {
        branchTreeModel->PrepareModel(selection);
    }
}

void DumpViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Branch* branch = static_cast<Branch*>(current.internalPointer());
    Vector<const MMBlock*> blocks = branch->GetMemoryBlocks();

    blockListModel->PrepareModel(blocks);
}

void DumpViewerWidget::BranchBlockView_DoubleClicked(const QModelIndex& current)
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

Vector<const char*> DumpViewerWidget::GetSelectedSymbols()
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
