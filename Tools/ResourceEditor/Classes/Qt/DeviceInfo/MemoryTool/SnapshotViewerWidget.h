#ifndef __SNAPSHOTVIEWERWIDGET_H__
#define __SNAPSHOTVIEWERWIDGET_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"

#include <QWidget>

class QTabWidget;
class QTreeView;
class QListView;
class QComboBox;
class QStandardItem;

class SymbolsWidget;
class MemoryBlocksWidget;

class BranchTreeModel;
class BranchFilterModel;
class BlockListModel;

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

    void AllocPoolComboItemChanged(QStandardItem* item);
    void TagComboItemChanged(QStandardItem* item);

    void ApplyClicked();

private:
    void Init();
    void InitSymbolsView();
    void InitBranchView();
    void InitMemoryBlocksView();

    QComboBox* InitAllocPoolsCombo();
    QComboBox* InitTagsCombo();

private:
    const ProfilingSession* session = nullptr;
    const MemorySnapshot* snapshot = nullptr;

    BlockLink allBlocksLinked;

    BranchTreeModel* branchTreeModel = nullptr;
    BranchFilterModel* branchFilterModel = nullptr;
    BlockListModel* blockListModel = nullptr;

    QTabWidget* tab = nullptr;
    SymbolsWidget* symbolWidget = nullptr;
    MemoryBlocksWidget* memoryBlocksWidget = nullptr;
    QTreeView* branchTree = nullptr;
    QListView* blockList = nullptr;

    DAVA::uint32 poolFilter = 0;
    DAVA::uint32 tagFilter = 0;
};

#endif  // __SNAPSHOTVIEWERWIDGET_H__
