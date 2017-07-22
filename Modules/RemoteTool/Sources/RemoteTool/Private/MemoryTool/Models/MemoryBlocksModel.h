#pragma once

#include <functional>

#include "RemoteTool/Private/MemoryTool/BlockLink.h"

#include <Base/BaseTypes.h>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

class ProfilingSession;
struct BlockLink;

class MemoryBlocksModel : public QAbstractTableModel
{
public:
    enum
    {
        ROLE_LINKITEM_POINTER = Qt::UserRole + 1
    };

public:
    MemoryBlocksModel(const ProfilingSession* session, QObject* parent = nullptr);
    virtual ~MemoryBlocksModel();

    void SetBlockLink(const BlockLink* blockLink);

    // reimplemented QAbstractTableModel methods
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QString TagsToString(DAVA::uint32 tags) const;

private:
    const ProfilingSession* session = nullptr;
    const BlockLink* blockLink = nullptr;
};

//////////////////////////////////////////////////////////////////////////
class MemoryBlocksFilterModel : public QSortFilterProxyModel
{
public:
    MemoryBlocksFilterModel(MemoryBlocksModel* model, QObject* parent = nullptr);
    virtual ~MemoryBlocksFilterModel();

    template <typename F>
    void SetFilter(F fn);
    void ClearFilter();

    template <typename F>
    void SortBy(F fn);

    template <typename F>
    void IterateOverElements(F fn);

    // reimplemented QSortFilterProxyModel methods
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    // TODO: replace with DAVA::Function
    std::function<bool(const BlockLink::Item&, const BlockLink::Item&)> lessThanPredicate;
    std::function<bool(const BlockLink::Item&)> filterPredicate;

    bool dontFilter = true;
};

//////////////////////////////////////////////////////////////////////////
template <typename F>
void MemoryBlocksFilterModel::SetFilter(F fn)
{
    dontFilter = false;
    filterPredicate = fn;
    invalidateFilter();
}

template <typename F>
void MemoryBlocksFilterModel::SortBy(F fn)
{
    lessThanPredicate = fn;
    sort(0);
}

template <typename F>
void MemoryBlocksFilterModel::IterateOverElements(F fn)
{
    for (int i = 0, n = rowCount(); i < n; ++i)
    {
        QVariant v = data(index(i, 0), MemoryBlocksModel::ROLE_LINKITEM_POINTER);
        const BlockLink::Item* item = static_cast<const BlockLink::Item*>(v.value<void*>());
        fn(*item);
    }
}
