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

#include "Platform/SystemTimer.h"

#include "DumpViewerWidget.h"

#include "Models/SymbolsTreeModel.h"
#include "Models/SymbolsFilterModel.h"
#include "Models/BranchTreeModel.h"
#include "Models/BlockListModel.h"

using namespace DAVA;

DumpViewerWidget::DumpViewerWidget(const char* filename, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , symbolsTreeModel(nullptr)
    , symbolsFilterModel(nullptr)
    , branchTreeModel(nullptr)
    , blockListModel(nullptr)
    , diffTreeModel(nullptr)
    , blockDiffListModel(nullptr)
    , tab(nullptr)
    , symbolsTree(nullptr)
    , branchTree(nullptr)
    , blockList(nullptr)
    , diffTree(nullptr)
    , diffList(nullptr)
{
    setWindowTitle(QString("Enhanced dump: %1").arg(filename));
    dumpSession.LoadDump(filename);
    Init();
}

DumpViewerWidget::~DumpViewerWidget()
{
    delete symbolsFilterModel;
    delete symbolsTreeModel;
    delete branchTreeModel;
    delete blockListModel;
    delete diffTreeModel;
    delete blockDiffListModel;
}

void DumpViewerWidget::Init()
{
    tab = new QTabWidget;

    InitSymbolsView();
    InitBranchView();
    InitDiffView();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    setLayout(mainLayout);
}

void DumpViewerWidget::InitSymbolsView()
{
    symbolsTreeModel = new SymbolsTreeModel(dumpSession.SymbolTable());
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
    QPushButton* buildDiff = new QPushButton("Build diff tree");
    QPushButton* loadDump = new QPushButton("Load another dump");

    connect(filter, &QLineEdit::textChanged, symbolsFilterModel, &SymbolsFilterModel::SetFilterString);
    connect(toggleStd, &QPushButton::clicked, symbolsFilterModel, &SymbolsFilterModel::ToggleStd);
    connect(buildTree, &QPushButton::clicked, this, &DumpViewerWidget::SymbolView_OnBuldTree);
    connect(buildDiff, &QPushButton::clicked, this, &DumpViewerWidget::SymbolView_OnBuldDiff);
    connect(loadDump, &QPushButton::clicked, this, &DumpViewerWidget::SymbolView_OnLoadAnotherDump);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(filter);
    layout->addWidget(toggleStd);
    layout->addWidget(buildTree);
    layout->addWidget(buildDiff);
    layout->addWidget(loadDump);
    layout->addWidget(symbolsTree);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);
    tab->addTab(frame, "Symbols");
}

void DumpViewerWidget::InitBranchView()
{
    branchTreeModel = new BranchTreeModel(dumpSession, false);
    blockListModel = new BlockListModel(false);

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    branchTree->setModel(branchTreeModel);

    //blockList = new QListView;
    blockList = new QTreeView;
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

void DumpViewerWidget::InitDiffView()
{
    diffTreeModel = new BranchTreeModel(dumpSession, true);
    blockDiffListModel = new BlockListModel(true);

    diffTree = new QTreeView;
    diffTree->setFont(QFont("Consolas", 10, 500));
    diffTree->setModel(diffTreeModel);

    //diffList = new QListView;
    diffList = new QTreeView;
    diffList->setFont(QFont("Consolas", 10, 500));
    diffList->setModel(blockDiffListModel);

    QItemSelectionModel* selModel = diffTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &DumpViewerWidget::DiffView_SelectionChanged);
    connect(diffList, &QTreeView::doubleClicked, this, &DumpViewerWidget::DiffBlockView_DoubleClicked);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(diffTree);
    layout->addWidget(diffList);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);

    tab->addTab(frame, "Diff");
}

void DumpViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const char*> selection = GetSelectedSymbols();
    if (!selection.empty())
    {
        //const char* name = selection[0];
        //branchTreeModel->PrepareModel(name);
        branchTreeModel->PrepareModel(selection);
    }
}

void DumpViewerWidget::SymbolView_OnBuldDiff()
{
    Vector<const char*> selection = GetSelectedSymbols();
    if (!selection.empty())
    {
        //const char* name = selection[0];
        //diffTreeModel->PrepareDiffModel(name);
        diffTreeModel->PrepareDiffModel(selection);
    }
}

void DumpViewerWidget::SymbolView_OnLoadAnotherDump()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select dump file", "d:\\share\\dumps\\test", "Dumps (*.bin)");
    if (!filename.isEmpty())
    {
        dumpSession.LoadDump(filename.toStdString().c_str());
    }
}

void DumpViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    qDebug() << "Selection: " << QString("%1, %2").arg(current.row()).arg(current.column())<<
        QString("%1, %2").arg(current.parent().row()).arg(current.parent().column());
    Branch* branch = static_cast<Branch*>(current.internalPointer());
    Vector<const MMBlock*> blocks1 = dumpSession.GetBlocks(branch, 0);

    auto f = [](const MMBlock* l, const MMBlock* r) -> bool {
        return l->orderNo < r->orderNo;
    };
    std::sort(blocks1.begin(), blocks1.end(), f);

    blockListModel->PrepareModel(blocks1);
}

void DumpViewerWidget::DiffView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Branch* branch = static_cast<Branch*>(current.internalPointer());
    Vector<const MMBlock*> blocks1 = dumpSession.GetBlocks(branch, 0);
    Vector<const MMBlock*> blocks2 = dumpSession.GetBlocks(branch, 1);

    auto f = [](const MMBlock* l, const MMBlock* r) -> bool {
        return l->orderNo < r->orderNo;
    };
    std::sort(blocks1.begin(), blocks1.end(), f);
    std::sort(blocks2.begin(), blocks2.end(), f);

    blockDiffListModel->PrepareDiffModel(blocks1, blocks2);
}

void DumpViewerWidget::BranchBlockView_DoubleClicked(const QModelIndex& current)
{
    const MMBlock* p = blockListModel->GetBlock(current);
    if (p != nullptr)
    {
        //view->hierarhyTree->collapseAll();
        //view->hierarhyTree->expand(index.parent());
        //view->hierarhyTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        //view->hierarhyTree->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

        Vector<QModelIndex> v = branchTreeModel->Select2(p);
        if (!v.empty())
        {
            for (auto& x : v)
                branchTree->expand(x);
            branchTree->setCurrentIndex(v.back());
            branchTree->selectionModel()->select(v.back(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
    }
}

void DumpViewerWidget::DiffBlockView_DoubleClicked(const QModelIndex& current)
{
    const MMBlock* p = blockDiffListModel->GetBlock(current);
    if (p != nullptr)
    {
        Vector<QModelIndex> v = diffTreeModel->Select2(p);

        if (!v.empty())
        {
            for (auto& x : v)
                diffTree->expand(x);
            diffTree->setCurrentIndex(v.back());
            diffTree->selectionModel()->select(v.back(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
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
