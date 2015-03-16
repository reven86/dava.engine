#pragma once

#include <QWidget>

#include "MemoryManager/MemoryManagerTypes.h"

class QTreeView;
class QAbstractItemModel;
class AllocationTreeModel;
class BacktraceTreeModel;
class DumpViewWidget : public QWidget
{
    Q_OBJECT

public:
    DumpViewWidget(const char* filename, QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    DumpViewWidget(const DAVA::Vector<DAVA::uint8>& v, QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    ~DumpViewWidget();

private:
    bool LoadDump(const char* filename);
    bool LoadDump(const DAVA::Vector<DAVA::uint8>& v);

private:
    QTreeView* treeView;
    AllocationTreeModel* allocTreeModel;
    BacktraceTreeModel* backtraceTreeModel;

    DAVA::MMDump dumpHdr;
    DAVA::Vector<DAVA::MMBlock> blocks;

    DAVA::UnorderedMap<DAVA::uint64, DAVA::String> symbolMap;
    DAVA::UnorderedMap<DAVA::uint32, DAVA::MMBacktrace> traceMap;
};
