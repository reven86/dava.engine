#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "Qt/DeviceInfo/MemoryTool/SnapshotViewerWidget.h"

#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsListModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsFilterModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BranchTreeModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BlockListModel.h"

#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"

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
#include <QComboBox>
#include <QStandardItemModel>
#include <QStandardItem>

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
    delete symbolsListModel;
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
    symbolsListModel = new SymbolsListModel(*snapshot->SymbolTable());
    symbolsFilterModel = new SymbolsFilterModel;
    symbolsFilterModel->setSourceModel(symbolsListModel);
    symbolsFilterModel->sort(0);

    symbolsTree = new QTreeView;
    symbolsTree->setFont(QFont("Consolas", 10, 500));
    symbolsTree->setSelectionMode(QAbstractItemView::ContiguousSelection);
    symbolsTree->setModel(symbolsFilterModel);

    QLineEdit* filter = new QLineEdit;
    QPushButton* toggleStd = new QPushButton("Toggle 'std::' on/off");
    QPushButton* buildTree = new QPushButton("Build tree");

    connect(filter, &QLineEdit::textChanged, symbolsFilterModel, &SymbolsFilterModel::SetFilterString);
    connect(toggleStd, &QPushButton::clicked, symbolsFilterModel, &SymbolsFilterModel::ToggleHideStdAndUnresolved);
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
    branchFilterModel = new BranchFilterModel;
    branchFilterModel->setSourceModel(branchTreeModel);

    blockListModel = new BlockListModel;

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    //branchTree->setModel(branchTreeModel);
    branchTree->setModel(branchFilterModel);

    blockList = new QListView;
    blockList->setFont(QFont("Consolas", 10, 500));
    blockList->setModel(blockListModel);

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &SnapshotViewerWidget::BranchView_SelectionChanged);
    connect(blockList, &QTreeView::doubleClicked, this, &SnapshotViewerWidget::BranchBlockView_DoubleClicked);

    QPushButton* apply = new QPushButton("Apply");
    connect(apply, &QPushButton::clicked, this, &SnapshotViewerWidget::ApplyClicked);

    QHBoxLayout* hl = new QHBoxLayout;
    hl->addWidget(InitAllocPoolsCombo());
    hl->addWidget(InitTagsCombo());
    hl->addWidget(apply);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(hl);
    layout->addWidget(branchTree);
    layout->addWidget(blockList);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);

    tab->addTab(frame, "Branches");
}

QComboBox* SnapshotViewerWidget::InitAllocPoolsCombo()
{
    const ProfilingSession* session = snapshot->Session();

    int nrows = static_cast<int>(session->AllocPoolCount());
    QStandardItemModel* model = new QStandardItemModel(nrows, 1);
    for (int i = 0;i < nrows;++i)
    {
        const String& name = session->AllocPoolName(i);
        QStandardItem* item = new QStandardItem(QString(name.c_str()));
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setData(1 << i, Qt::UserRole + 1);

        model->setItem(i, 0, item);
    }
    connect(model, &QStandardItemModel::itemChanged, this, &SnapshotViewerWidget::AllocPoolComboItemChanged);

    QComboBox* widget = new QComboBox;
    widget->setModel(model);
    return widget;
}

QComboBox* SnapshotViewerWidget::InitTagsCombo()
{
    const ProfilingSession* session = snapshot->Session();

    int nrows = static_cast<int>(session->TagCount());
    QStandardItemModel* model = new QStandardItemModel(nrows, 1);
    for (int i = 0;i < nrows;++i)
    {
        const String& name = session->TagName(i);
        QStandardItem* item = new QStandardItem(QString(name.c_str()));
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setData(1 << i, Qt::UserRole + 1);

        model->setItem(i, 0, item);
    }
    connect(model, &QStandardItemModel::itemChanged, this, &SnapshotViewerWidget::TagComboItemChanged);

    QComboBox* widget = new QComboBox;
    widget->setModel(model);
    return widget;
}

void SnapshotViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const String*> selection = GetSelectedSymbols();
    if (!selection.empty())
    {
        branchTreeModel->PrepareModel(selection);
    }
}

void SnapshotViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    QModelIndex index = branchFilterModel->mapToSource(current);

    Branch* branch = static_cast<Branch*>(index.internalPointer());
    if (branch != nullptr)
    {
        Vector<MMBlock*> blocks = branch->GetMemoryBlocks();
        blockListModel->PrepareModel(std::forward<Vector<MMBlock*>>(blocks));
    }
}

void SnapshotViewerWidget::BranchBlockView_DoubleClicked(const QModelIndex& current)
{
    const MMBlock* block = blockListModel->GetBlock(current);
    if (block != nullptr)
    {
        // TODO: expand callstack tree to view block allocation site
    }
}

void SnapshotViewerWidget::AllocPoolComboItemChanged(QStandardItem* item)
{
    int chk = item->data(Qt::CheckStateRole).toInt();
    int v = item->data(Qt::UserRole + 1).toInt();
    if (chk == Qt::Checked)
    {
        poolFilter |= v;
    }
    else if (chk == Qt::Unchecked)
    {
        poolFilter &= ~v;
    }
}

void SnapshotViewerWidget::TagComboItemChanged(QStandardItem* item)
{
    int chk = item->data(Qt::CheckStateRole).toInt();
    int v = item->data(Qt::UserRole + 1).toInt();
    if (chk == Qt::Checked)
    {
        tagFilter |= v;
    }
    else if (chk == Qt::Unchecked)
    {
        tagFilter &= ~v;
    }
}

void SnapshotViewerWidget::ApplyClicked()
{
    branchFilterModel->SetFilter(poolFilter, tagFilter);
}

Vector<const String*> SnapshotViewerWidget::GetSelectedSymbols()
{
    Vector<const String*> result;
    QItemSelectionModel* selectionModel = symbolsTree->selectionModel();
    if (selectionModel->hasSelection())
    {
        QModelIndexList indexList = selectionModel->selectedRows(0);
        result.reserve(indexList.size());
        for (const QModelIndex& i : indexList)
        {
            QModelIndex index = symbolsFilterModel->mapToSource(i);
            if (index.isValid())
            {
                const String* name = symbolsListModel->Symbol(index.row());
                result.push_back(name);
            }
        }
    }
    return result;
}
