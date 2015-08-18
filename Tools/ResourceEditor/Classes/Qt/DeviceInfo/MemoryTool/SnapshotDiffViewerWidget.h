#ifndef __SNAPSHOTDIFFVIEWERWIDGET_H__
#define __SNAPSHOTDIFFVIEWERWIDGET_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include <QWidget>

class QTabWidget;
class QTreeView;
class QListView;

class SymbolsWidget;

class BranchDiffTreeModel;
class BlockListModel;

class MemorySnapshot;

class SnapshotDiffViewerWidget : public QWidget
{
    Q_OBJECT

public:
    SnapshotDiffViewerWidget(const MemorySnapshot* snapshot1, const MemorySnapshot* snapshot2, QWidget* parent = nullptr);
    virtual ~SnapshotDiffViewerWidget();

public slots:
    void SymbolView_OnBuldTree();

    void BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void BranchBlockView_DoubleClicked(const QModelIndex& current);

private:
    void Init();
    void InitSymbolsView();
    void InitBranchView();

private:
    const MemorySnapshot* snapshot1;
    const MemorySnapshot* snapshot2;

    BranchDiffTreeModel* branchTreeModel = nullptr;
    BlockListModel* blockListModel1 = nullptr;
    BlockListModel* blockListModel2 = nullptr;

    QTabWidget* tab = nullptr;
    SymbolsWidget* symbolWidget = nullptr;
    QTreeView* branchTree = nullptr;
    QListView* blockList1 = nullptr;
    QListView* blockList2 = nullptr;
};

#endif  // __SNAPSHOTDIFFVIEWERWIDGET_H__
