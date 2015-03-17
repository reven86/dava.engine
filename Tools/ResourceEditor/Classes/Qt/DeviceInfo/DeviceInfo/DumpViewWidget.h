#pragma once

#include <QWidget>

#include "MemoryManager/MemoryManagerTypes.h"

class QTabWidget;
class QAbstractItemModel;
class AllocationTreeModel;
class BacktraceTreeModel;
class SymbolsTreeModel;
class DumpViewWidget : public QWidget
{
    Q_OBJECT

public:
    DumpViewWidget(const char* filename, QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    DumpViewWidget(const DAVA::Vector<DAVA::uint8>& v, QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    ~DumpViewWidget();

private:
    void Init();

    bool LoadDump(const char* filename);
    bool LoadDump(const DAVA::Vector<DAVA::uint8>& v);

private:
    QTabWidget* tab;
    AllocationTreeModel* allocTreeModel;
    BacktraceTreeModel* backtraceTreeModel;
    SymbolsTreeModel* symbolTreeModel;

    DAVA::MMDump dumpHdr;
    DAVA::Vector<DAVA::MMBlock> blocks;

    DAVA::UnorderedMap<DAVA::uint64, DAVA::String> symbolMap;
    DAVA::UnorderedMap<DAVA::uint32, DAVA::MMBacktrace> traceMap;
};
