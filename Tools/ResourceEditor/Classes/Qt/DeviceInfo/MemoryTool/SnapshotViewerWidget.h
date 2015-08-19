#ifndef __SNAPSHOTVIEWERWIDGET_H__
#define __SNAPSHOTVIEWERWIDGET_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"

#include <QWidget>

class QTabWidget;
class QTreeView;

class SymbolsWidget;
class MemoryBlocksWidget;

class BranchTreeModel;
class BranchFilterModel;

class ProfilingSession;
class MemorySnapshot;

class SnapshotViewerWidget : public QWidget
{
    Q_OBJECT

public:
    SnapshotViewerWidget(const ProfilingSession* session, size_t snapshotIndex, QWidget* parent = nullptr);
    virtual ~SnapshotViewerWidget();

public slots:
    void SymbolView_OnBuldTree();

    void BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void BranchBlockView_DoubleClicked(const QModelIndex& current);

private:
    void Init();
    void InitSymbolsView();
    void InitBranchView();
    void InitMemoryBlocksView();

private:
    const ProfilingSession* session = nullptr;
    const MemorySnapshot* snapshot = nullptr;

    BlockLink allBlocksLinked;

    DAVA::Vector<DAVA::MMBlock*> branchBlocks;
    BlockLink branchBlockLinked;

    BranchTreeModel* branchTreeModel = nullptr;
    BranchFilterModel* branchFilterModel = nullptr;

    QTabWidget* tab = nullptr;
    SymbolsWidget* symbolWidget = nullptr;
    MemoryBlocksWidget* memoryBlocksWidget = nullptr;
    MemoryBlocksWidget* branchBlocksWidget = nullptr;
    QTreeView* branchTree = nullptr;
};

#endif  // __SNAPSHOTVIEWERWIDGET_H__
