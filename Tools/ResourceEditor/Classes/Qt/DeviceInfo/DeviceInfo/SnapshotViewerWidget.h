#ifndef __SNAPSHOTVIEWERWIDGET_H__
#define __SNAPSHOTVIEWERWIDGET_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include <QWidget>

class QTabWidget;
class QTreeView;
class QListView;

class SymbolsTreeModel;
class SymbolsFilterModel;

class BranchTreeModel;
class BlockListModel;

class MemorySnapshot;

class SnapshotViewerWidget : public QWidget
{
    Q_OBJECT

public:
    SnapshotViewerWidget(const MemorySnapshot* snapshot, QWidget* parent = nullptr);
    virtual ~SnapshotViewerWidget();

public slots:
    void SymbolView_OnBuldTree();

    void BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void BranchBlockView_DoubleClicked(const QModelIndex& current);

private:
    void Init();
    void InitSymbolsView();
    void InitBranchView();

    DAVA::Vector<const char*> GetSelectedSymbols();

private:
    const MemorySnapshot* snapshot;

    SymbolsTreeModel* symbolsTreeModel = nullptr;
    SymbolsFilterModel* symbolsFilterModel = nullptr;

    BranchTreeModel* branchTreeModel = nullptr;
    BlockListModel* blockListModel = nullptr;

    QTabWidget* tab = nullptr;
    QTreeView* symbolsTree = nullptr;
    QTreeView* branchTree = nullptr;
    QListView* blockList = nullptr;
};

#endif  // __SNAPSHOTVIEWERWIDGET_H__
