#pragma once

#include <QtGui>
#include <QItemDelegate.h>
#include "MemoryManager/MemoryManagerTypes.h"
#include <functional>

class MyDelegate : public QItemDelegate
{
public:
    MyDelegate(QObject* parent = 0)
        : QItemDelegate(parent)
    {}
    void paint(QPainter* painter, const QStyleOptionViewItem& opt, const QModelIndex& index) const
    {
        if (index.column() != 0) {
            QItemDelegate::paint(painter, opt, index);
            return;
        }
        // Draw a check box for the status column
        bool enabled = true;
        QStyleOptionViewItem option = opt;
        QRect crect = option.rect;
        crect.moveLeft(15);
        drawCheck(painter, option, crect, enabled ? Qt::Checked : Qt::Unchecked);
    }
};

struct MemData
{
    DAVA::uint32 allocByApp;
    DAVA::uint32 allocTotal;
};

using PoolsStat = DAVA::Vector < MemData >;
using TagsStatVector = DAVA::Vector < PoolsStat >;

struct TagsStat
{
    TagsStatVector statData;
    DAVA::Vector<int> tagNames;
};

class MemProfInfoModel : public QAbstractTableModel
{
public:
    MemProfInfoModel();
    virtual ~MemProfInfoModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const; 
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void addMoreData(const DAVA::MMCurStat * data);
    void setConfig(const DAVA::MMStatConfig* statConfig);
    int getCurrentTimeStamp(){ return timedData.lastKey();}
    const TagsStat getCurrentTagStat(){ return timedData.last(); }
    void forTagStats(std::function<void(size_t, const TagsStat&)> onStat);
    void showDataToClosest(size_t closest);
    void showLatestData();

private:
    bool latestData = true;
    size_t dataToShow = 0;
    QMap<int, TagsStat > timedData;

    size_t allocPoolCount = 0;
    size_t tagCount = 0;
    DAVA::Vector<QString> tagNames;
    DAVA::Vector<QString> poolNames;

    DAVA::Vector<DAVA::uint32> totals;
    DAVA::Vector<DAVA::uint32> allocs;
};
