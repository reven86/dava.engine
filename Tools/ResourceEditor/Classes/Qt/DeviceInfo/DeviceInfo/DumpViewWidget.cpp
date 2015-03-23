#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QAction>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QLineEdit>

#include "Platform/SystemTimer.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "DumpViewWidget.h"
#include "Models/AllocationTreeModel.h"
#include "Models/BacktraceTreeModel.h"
#include "Models/SymbolsTreeModel.h"
#include "Models/SymbolsFilterModel.h"
#include "Models/CallStackTreeModel.h"

using namespace DAVA;

DumpViewWidget::DumpViewWidget(const char* filename, QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , tab(nullptr)
    , symbolTree(nullptr)
    , allocTreeModel(nullptr)
    , backtraceTreeModel(nullptr)
    , symbolTreeModel(nullptr)
    , symbolsFilterModel(nullptr)
    , callstackTreeModel(nullptr)
    , dumpHdr()
{
    setWindowTitle(filename);
    uint64 begin = SystemTimer::Instance()->AbsoluteMS();
    LoadDump(filename);
    loadTime = static_cast<uintptr_t>(SystemTimer::Instance()->AbsoluteMS() - begin);

    begin = SystemTimer::Instance()->AbsoluteMS();
    bktrace.Rebuild();
    modelRebuildTime = static_cast<uintptr_t>(SystemTimer::Instance()->AbsoluteMS() - begin);
    Init();
}

DumpViewWidget::DumpViewWidget(const DAVA::Vector<uint8>& v, QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , tab(nullptr)
    , symbolTree(nullptr)
    , allocTreeModel(nullptr)
    , backtraceTreeModel(nullptr)
    , symbolTreeModel(nullptr)
    , symbolsFilterModel(nullptr)
    , callstackTreeModel(nullptr)
    , dumpHdr()
{
    setWindowTitle("In-memory dump");
    uint64 begin = SystemTimer::Instance()->AbsoluteMS();
    LoadDump(v);
    //bktrace.Rebuild();
    loadTime = static_cast<uintptr_t>(SystemTimer::Instance()->AbsoluteMS() - begin);
    Init();
}

DumpViewWidget::~DumpViewWidget()
{
    delete allocTreeModel;
    delete backtraceTreeModel;
    delete symbolTreeModel;
    delete symbolsFilterModel;
    delete callstackTreeModel;
}

void DumpViewWidget::Init()
{
    QFont font("Consolas", 10, 500);

    {
        uint64 begin = SystemTimer::Instance()->AbsoluteMS();
        //allocTreeModel = new AllocationTreeModel(blocks, symbolMap, traceMap);
        //backtraceTreeModel = new BacktraceTreeModel(blocks, symbolMap, traceMap);
        symbolTreeModel = new SymbolsTreeModel(bktrace);
        symbolsFilterModel = new SymbolsFilterModel;
        callstackTreeModel = new CallStackTreeModel(blocks, bktrace);
        modelCreateTime = static_cast<uintptr_t>(SystemTimer::Instance()->AbsoluteMS() - begin);
    }
    tab = new QTabWidget;

    {
        QTreeView* tree = new QTreeView;
        tree->setFont(font);
        tree->setModel(allocTreeModel);
        tab->addTab(tree, "Alloc");
    }
    {
        QTreeView* tree = new QTreeView;
        tree->setFont(font);
        tree->setModel(backtraceTreeModel);
        tab->addTab(tree, "Backtrace");
    }
    {
        symbolTree = new QTreeView;
        symbolTree->setFont(font);
        symbolTree->setSelectionMode(QAbstractItemView::ContiguousSelection);

        symbolsFilterModel->setSourceModel(symbolTreeModel);
        symbolTree->setModel(symbolsFilterModel);
        symbolsFilterModel->sort(0);
        //connect(tree, &QTreeView::doubleClicked, this, &DumpViewWidget::OnSymbolDoubleClicked);

        QVBoxLayout* l = new QVBoxLayout;
        QLineEdit* le = new QLineEdit;
        QPushButton* b1 = new QPushButton("Show/hide std::");
        QPushButton* b2 = new QPushButton("Do job");
        connect(b2, &QPushButton::clicked, this, &DumpViewWidget::OnShowCallstack);
        connect(b1, &QPushButton::clicked, symbolsFilterModel, &SymbolsFilterModel::ToggleStd);
        connect(le, &QLineEdit::textChanged, symbolsFilterModel, &SymbolsFilterModel::SetFilterString);
        l->addWidget(le);
        l->addWidget(b1);
        l->addWidget(b2);
        l->addWidget(symbolTree);

        QFrame* frame = new QFrame;
        frame->setLayout(l);
        tab->addTab(frame, "Symbols");
    }
    {
        QTreeView* tree = new QTreeView;
        tree->setFont(font);
        tree->setModel(callstackTreeModel);
        tree->setRootIsDecorated(true);

        QVBoxLayout* l = new QVBoxLayout;
        QPushButton* b = new QPushButton("Reset");
        connect(b, &QPushButton::clicked, this, &DumpViewWidget::OnCallstackReset);
        l->addWidget(b);
        l->addWidget(tree);

        QFrame* frame = new QFrame;
        frame->setLayout(l);

        tab->addTab(frame, "Call stack");
    }

    QVBoxLayout* l = new QVBoxLayout;
    labelpopulate = new QLabel("");
    l->addWidget(tab);
    l->addWidget(new QLabel(QString("load time %1 ms").arg(loadTime)));
    l->addWidget(new QLabel(QString("model create time %1 ms").arg(modelCreateTime)));
    l->addWidget(new QLabel(QString("model rebuild time %1 ms").arg(modelRebuildTime)));
    l->addWidget(labelpopulate);
    setLayout(l);

    setAttribute(Qt::WA_DeleteOnClose);
    show();
}

void DumpViewWidget::OnSymbolDoubleClicked(const QModelIndex& index)
{
    /*uint64 addr = symbolTreeModel->Address(index);
    if (addr)
    {
        callstackTreeModel->Reset(addr);
    }*/
}

void DumpViewWidget::OnCallstackReset()
{
    //callstackTreeModel->Reset();
}

void DumpViewWidget::OnShowCallstack()
{
    QItemSelectionModel* selModel = symbolTree->selectionModel();
    if (selModel->hasSelection())
    {
        Vector<uint64> v;
        QModelIndexList list = selModel->selectedRows(0);
        for (auto x : list)
        {
            QModelIndex index = symbolsFilterModel->mapToSource(x);
            if (index.isValid())
            {
                GenericTreeNode* node = static_cast<GenericTreeNode*>(index.internalPointer());
                if (node->Type() == SymbolsTreeModel::TYPE_NAME)
                {
                    SymbolsTreeModel::NameNode* nnode = static_cast<SymbolsTreeModel::NameNode*>(node);
                    uint64 addr = nnode->Address();
                    v.push_back(addr);
                }
            }
        }
        std::sort(v.begin(), v.end());
        uint64 begin = SystemTimer::Instance()->AbsoluteMS();
        callstackTreeModel->Rebuild(v, false);
        populateTime = uintptr_t(SystemTimer::Instance()->AbsoluteMS() - begin);
        labelpopulate->setNum(int(populateTime));
    }
}

bool DumpViewWidget::LoadDump(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (!file) return false;

    fread(&dumpHdr, sizeof(MMDump), 1, file);

    blocks.reserve(dumpHdr.blockCount);
    for (size_t i = 0, n = dumpHdr.blockCount;i < n;++i)
    {
        MMBlock o;
        size_t c = fread(&o, sizeof(MMBlock), 1, file);
        Q_ASSERT(c == 1);

        blocks.push_back(o);
    }

    for (size_t i = 0, n = dumpHdr.backtraceCount;i < n;++i)
    {
        MMBacktrace o;
        size_t c = fread(&o, sizeof(MMBacktrace), 1, file);
        Q_ASSERT(c == 1);

        bktrace.AddBacktrace(o);
    }
    for (size_t i = 0, n = dumpHdr.symbolCount;i < n;++i)
    {
        MMSymbol o;
        size_t c = fread(&o, sizeof(MMSymbol), 1, file);
        Q_ASSERT(c == 1);

        bktrace.AddSymbol(o.addr, o.name);
    }
    fclose(file);
    return true;
}

template<typename T>
inline T* Offset(void* ptr, size_t byteOffset)
{
    return reinterpret_cast<T*>(static_cast<uint8*>(ptr)+byteOffset);
}

template<typename T>
inline const T* Offset(const void* ptr, size_t byteOffset)
{
    return reinterpret_cast<const T*>(static_cast<const uint8*>(ptr)+byteOffset);
}

bool DumpViewWidget::LoadDump(const DAVA::Vector<uint8>& v)
{
    const MMDump* dump = reinterpret_cast<const MMDump*>(v.data());
    const MMBlock* rawBlocks = Offset<MMBlock>(dump, sizeof(MMDump));
    const MMBacktrace* bt = Offset<MMBacktrace>(rawBlocks, sizeof(MMBlock) * dump->blockCount);
    const MMSymbol* symbols = Offset<MMSymbol>(bt, sizeof(MMBacktrace) * dump->backtraceCount);

    dumpHdr = *dump;

    blocks.reserve(dump->blockCount);
    for (size_t i = 0, n = dump->blockCount;i < n;++i)
    {
        MMBlock o = rawBlocks[i];
        blocks.push_back(o);
    }

    for (size_t i = 0, n = dump->symbolCount;i < n;++i)
        bktrace.AddSymbol(symbols[i].addr, symbols[i].name);
    for (size_t i = 0, n = dump->backtraceCount;i < n;++i)
        bktrace.AddBacktrace(bt[i]);
    return true;
}
