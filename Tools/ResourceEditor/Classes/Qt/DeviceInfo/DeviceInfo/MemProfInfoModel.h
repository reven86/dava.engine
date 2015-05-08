#pragma once
#include <QtGui>
#include <QItemDelegate.h>
#include "MemoryManager/MemoryManagerTypes.h"

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
    MemProfInfoModel();
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const; 
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    virtual ~MemProfInfoModel();
    void addMoreData(const DAVA::MMStat * data);
    void setConfig(const DAVA::MMStatConfig* statConfig);
private:
    QString formatMemoryData(int memoryData);
    struct MemData
    {
        DAVA::uint32 allocByApp;
        DAVA::uint32 allocTotal;
    };
    using PoolsStat = DAVA::Vector < MemData > ;
    using TagsStatVector = DAVA::Vector < PoolsStat >;
    struct TagsStat
    {
        TagsStatVector statData;
        DAVA::Vector<int> tagNames;
    };
    
    QMap<int, TagsStat > timedData;
    DAVA::Vector<QString> tagNames;
    DAVA::Vector<QString> poolNames;
};

