#pragma once
#include <QtGui>
#include <QItemDelegate.h>
#include "MemoryManager/MemoryManagerTypes.h"
struct MemoryProfDataChunk
{
    size_t timestamp;
    DAVA::Vector< DAVA::Vector< DAVA::AllocPoolStat> > stat;
    size_t countMaxTagSize() const 
    {
        size_t maxNum = 0;
        for (const auto & m : stat)
        {
            maxNum = std::max(maxNum, m.size());

        }
        return maxNum;
    }

};
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

class MemProfInfoModel : public QAbstractTableModel
{
public:

    MemProfInfoModel()
    {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const
    {

        return timedData.last().stat.size();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const
    {
        
        return  timedData.last().countMaxTagSize();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
    {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();
        if (timedData.size() == 0)
            return QVariant();
        auto  latestData = timedData.last();
        return latestData.stat[index.row()][index.column()].allocByApp;
    }
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal) {
            return QString("tag_") + std::to_string(section).c_str();
        }
        else
            return QString("tag_") + std::to_string(section).c_str();
        return QVariant();
    }
    virtual ~MemProfInfoModel(){};
    void addMoreData(MemoryProfDataChunk data)
    {
        timedData[data.timestamp] = data;
        dataChanged(createIndex(0, 0, nullptr), createIndex(rowCount(), columnCount(), nullptr));
    }
private:
    QMap<int, MemoryProfDataChunk > timedData;
};

