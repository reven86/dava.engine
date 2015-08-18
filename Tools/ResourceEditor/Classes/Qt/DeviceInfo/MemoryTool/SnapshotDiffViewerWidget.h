#ifndef __SNAPSHOTDIFFVIEWERWIDGET_H__
#define __SNAPSHOTDIFFVIEWERWIDGET_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"

#include <QWidget>

class QTabWidget;
class QTreeView;
class QListView;

class SymbolsWidget;
class MemoryBlocksWidget;

class BranchDiffTreeModel;
class BlockListModel;

class ProfilingSession;
class MemorySnapshot;

class SnapshotDiffViewerWidget : public QWidget
{
    Q_OBJECT

public:
    SnapshotDiffViewerWidget(const ProfilingSession* session, size_t snapshotIndex1, size_t snapshotIndex2, QWidget* parent = nullptr);
    virtual ~SnapshotDiffViewerWidget();

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
    const MemorySnapshot* snapshot1 = nullptr;
    const MemorySnapshot* snapshot2 = nullptr;

    BlockLink allBlocksLinked;

    BranchDiffTreeModel* branchTreeModel = nullptr;
    BlockListModel* blockListModel1 = nullptr;
    BlockListModel* blockListModel2 = nullptr;

    QTabWidget* tab = nullptr;
    SymbolsWidget* symbolWidget = nullptr;
    MemoryBlocksWidget* memoryBlocksWidget = nullptr;
    QTreeView* branchTree = nullptr;
    QListView* blockList1 = nullptr;
    QListView* blockList2 = nullptr;
};

#endif  // __SNAPSHOTDIFFVIEWERWIDGET_H__
