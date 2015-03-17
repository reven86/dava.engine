#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QAction>

#include "MemoryManager/MemoryManagerTypes.h"

#include "DumpViewWidget.h"
#include "Models/AllocationTreeModel.h"
#include "Models/BacktraceTreeModel.h"
#include "Models/SymbolsTreeModel.h"

using namespace DAVA;

DumpViewWidget::DumpViewWidget(const char* filename, QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , tab(nullptr)
    , allocTreeModel(nullptr)
    , backtraceTreeModel(nullptr)
    , symbolTreeModel(nullptr)
    , dumpHdr()
{
    LoadDump(filename);
    Init();
}

DumpViewWidget::DumpViewWidget(const DAVA::Vector<uint8>& v, QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , tab(nullptr)
    , allocTreeModel(nullptr)
    , backtraceTreeModel(nullptr)
    , symbolTreeModel(nullptr)
    , dumpHdr()
{
    LoadDump(v);
    Init();
}

DumpViewWidget::~DumpViewWidget()
{
    delete allocTreeModel;
    delete backtraceTreeModel;
    delete symbolTreeModel;
}

void DumpViewWidget::Init()
{
    QFont font("Consolas", 10, 500);

    //allocTreeModel = new AllocationTreeModel(blocks, symbolMap, traceMap);
    //backtraceTreeModel = new BacktraceTreeModel(blocks, symbolMap, traceMap);
    symbolTreeModel = new SymbolsTreeModel(symbolMap);
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
        QTreeView* tree = new QTreeView;
        tree->setFont(font);
        tree->setModel(symbolTreeModel);
        tab->addTab(tree, "Symbols");
    }

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(tab);
    setLayout(l);

    setAttribute(Qt::WA_DeleteOnClose);
    show();
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

        uint32 hash = o.hash;
        traceMap.emplace(std::make_pair(hash, o));
    }
    for (size_t i = 0, n = dumpHdr.symbolCount;i < n;++i)
    {
        MMSymbol o;
        size_t c = fread(&o, sizeof(MMSymbol), 1, file);
        Q_ASSERT(c == 1);

        symbolMap.emplace(std::make_pair(o.addr, o.name));
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

    blocks.reserve(dump->blockCount);
    for (size_t i = 0, n = dump->blockCount;i < n;++i)
    {
        MMBlock o = rawBlocks[i];
        blocks.push_back(o);
    }

    for (size_t i = 0, n = dump->symbolCount;i < n;++i)
        symbolMap.emplace(std::make_pair(symbols[i].addr, symbols[i].name));
    for (size_t i = 0, n = dump->backtraceCount;i < n;++i)
    {
        uint32 hash = bt[i].hash;
        auto g = traceMap.emplace(std::make_pair(hash, bt[i]));
        //Q_ASSERT(g.second == true);
    }
    return true;
}
