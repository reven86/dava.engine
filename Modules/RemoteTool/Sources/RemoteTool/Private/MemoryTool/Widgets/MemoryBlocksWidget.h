#pragma once

#include "RemoteTool/Private/MemoryTool/BlockLink.h"
#include "RemoteTool/Private/MemoryTool/BlockGroup.h"

#include <Base/BaseTypes.h>

#include <QWidget>

class QTableView;
class QListView;
class QModelIndex;

class ProfilingSession;
class MemorySnapshot;
class MemoryBlocksModel;
class MemoryBlocksFilterModel;
class BlockGroupModel;
class BacktraceListModel;
struct BlockLink;

class MemoryBlocksWidget : public QWidget
{
    Q_OBJECT

public:
    MemoryBlocksWidget(const ProfilingSession* session, const BlockLink* blockLink, bool showBacktrace = true, bool enableGrouping = true, QWidget* parent = nullptr);
    virtual ~MemoryBlocksWidget();

    void SetBlockLink(const BlockLink* blockLink);

signals:
    void MemoryBlockDoubleClicked(const BlockLink::Item& item);

private slots:
    void TableWidget_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void TableWidget_DoubleClicked(const QModelIndex& index);

    void GroupWidget_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);

    void FilterBar_GroupOrderChanged(int order);
    void FilterBar_SortingOrderChanged(int order);
    void FilterBar_FilterChanged(DAVA::uint32 poolMask, DAVA::uint32 tagMask);
    void FilterBar_HideTheSameChanged(bool hide);
    void FilterBar_HideDifferentChanged(bool hide);
    void FilterBar_BlockOrderChanged(DAVA::uint32 minBlockOrder);

private:
    void Init();
    bool Filter(const BlockLink::Item& item);
    void GroupBy();

private:
    const ProfilingSession* session = nullptr;
    const BlockLink* blockLink = nullptr;

    QTableView* tableWidget = nullptr;
    QTableView* groupsWidget = nullptr;
    QListView* backtraceWidget = nullptr;
    bool showBacktrace = true;
    bool enableGrouping = false;

    std::unique_ptr<MemoryBlocksModel> memoryBlocksModel;
    std::unique_ptr<MemoryBlocksFilterModel> memoryBlocksFilterModel;
    std::unique_ptr<BlockGroupModel> blockGroupModel;
    std::unique_ptr<BacktraceListModel> backtraceListModel;

    DAVA::int32 groupOrder = 0;
    DAVA::uint32 filterPoolMask = 0;
    DAVA::uint32 filterTagMask = 0;
    bool hideTheSame = false;
    bool hideDifferent = false;
    DAVA::uint32 minBlockOrder = 0;

    DAVA::Vector<BlockGroup> groups;
};
