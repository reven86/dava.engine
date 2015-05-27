#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "DiffViewerWidget.h"

#include "Models/SymbolsTreeModel.h"
#include "Models/SymbolsFilterModel.h"
#include "Models/BranchDiffTreeModel.h"
#include "Models/BlockListModel.h"

#include "Classes/Qt/DeviceInfo/DeviceInfo/Branch.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/BranchDiff.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/ProfilingSession.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/MemorySnapshot.h"

#include <QDebug>
#include <QFileDialog>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QLineEdit>
#include <QListView>

using namespace DAVA;

DiffViewerWidget::DiffViewerWidget(const MemorySnapshot* snapshot1_, const MemorySnapshot* snapshot2_, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , snapshot1(snapshot1_)
    , snapshot2(snapshot2_)
{
    DVASSERT(snapshot1 != nullptr && snapshot1->IsLoaded());
    DVASSERT(snapshot2 != nullptr && snapshot2->IsLoaded());
    Init();
}

DiffViewerWidget::~DiffViewerWidget()
{
    delete symbolsFilterModel;
    delete symbolsTreeModel;
    delete branchTreeModel;
    delete blockListModel1;
    delete blockListModel2;
}

void DiffViewerWidget::Init()
{
    tab = new QTabWidget;

    InitSymbolsView();
    InitBranchView();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    setLayout(mainLayout);
}

void DiffViewerWidget::InitSymbolsView()
{
    symbolsTreeModel = new SymbolsTreeModel(*snapshot1->SymbolTable());
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
    connect(buildTree, &QPushButton::clicked, this, &DiffViewerWidget::SymbolView_OnBuldTree);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(filter);
    layout->addWidget(toggleStd);
    layout->addWidget(buildTree);
    layout->addWidget(symbolsTree);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);
    tab->addTab(frame, "Symbols");
}

void DiffViewerWidget::InitBranchView()
{
    branchTreeModel = new BranchDiffTreeModel(snapshot1, snapshot2);
    blockListModel1 = new BlockListModel;
    blockListModel2 = new BlockListModel;

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    branchTree->setModel(branchTreeModel);

    blockList1 = new QListView;
    blockList1->setFont(QFont("Consolas", 10, 500));
    blockList1->setModel(blockListModel1);

    blockList2 = new QListView;
    blockList2->setFont(QFont("Consolas", 10, 500));
    blockList2->setModel(blockListModel2);

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &DiffViewerWidget::BranchView_SelectionChanged);
    //connect(blockList, &QTreeView::doubleClicked, this, &DiffViewerWidget::BranchBlockView_DoubleClicked);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(blockList1);
    hlayout->addWidget(blockList2);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(branchTree);
    layout->addLayout(hlayout);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);

    tab->addTab(frame, "Branches");
}

void DiffViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const char*> selection = GetSelectedSymbols();
    if (!selection.empty())
    {
        branchTreeModel->PrepareModel(selection);
    }
}

void DiffViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    BranchDiff* branchDiff = static_cast<BranchDiff*>(current.internalPointer());

    if (branchDiff->left)
    {
        Vector<MMBlock> blocks = branchDiff->left->GetMemoryBlocks();
        blockListModel1->PrepareModel(std::forward<Vector<MMBlock>>(blocks));
    }
    else
        blockListModel1->ResetModel();

    if (branchDiff->right)
    {
        Vector<MMBlock> blocks = branchDiff->right->GetMemoryBlocks();
        blockListModel2->PrepareModel(std::forward<Vector<MMBlock>>(blocks));
    }
    else
        blockListModel2->ResetModel();
}

void DiffViewerWidget::BranchBlockView_DoubleClicked(const QModelIndex& current)
{
    
}

Vector<const char*> DiffViewerWidget::GetSelectedSymbols()
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
