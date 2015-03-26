#pragma once

#include <QWidget>

#include "MemoryManager/MemoryManagerTypes.h"

#include "BacktraceSet.h"
#include "BacktraceSymbolTable.h"

class QTabWidget;
class QTreeView;
class QLabel;
class QAbstractItemModel;
class AllocationTreeModel;
class BacktraceTreeModel;
class SymbolsTreeModel;
class SymbolsFilterModel;
class CallStackTreeModel;
class DumpViewWidget : public QWidget
{
    Q_OBJECT

public:
    DumpViewWidget(const char* filename, QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    DumpViewWidget(const DAVA::Vector<DAVA::uint8>& v, QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    ~DumpViewWidget();

public slots:
    void OnSymbolDoubleClicked(const QModelIndex& index);
    void OnCallstackReset();
    void OnShowCallstack();

private:
    void Init();

    bool LoadDump(const char* filename);
    bool LoadDump(const DAVA::Vector<DAVA::uint8>& v);

    void Dump();

private:
    QTabWidget* tab;
    QTreeView* symbolTree;
    AllocationTreeModel* allocTreeModel;
    BacktraceTreeModel* backtraceTreeModel;
    SymbolsTreeModel* symbolTreeModel;
    SymbolsFilterModel* symbolsFilterModel;
    CallStackTreeModel* callstackTreeModel;
    uintptr_t loadTime;
    uintptr_t modelCreateTime;
    uintptr_t modelRebuildTime;

    QLabel* labelpopulate;
    uintptr_t populateTime;

    DAVA::MMDump dumpHdr;
    DAVA::Vector<DAVA::MMBlock> blocks;

    BacktraceSet bktrace;
    BacktraceSymbolTable bktraceTable;
};
