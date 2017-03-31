#pragma once

#include <Base/BaseTypes.h>
#include <MemoryManager/MemoryManagerTypes.h>

#include <QColor>
#include <QAbstractTableModel>

class ProfilingSession;
class MemoryStatItem;

class TagModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum
    {
        CLM_NAME = 0,
        CLM_ALLOC_APP,
        CLM_NBLOCKS,
        NCOLUMNS = 3
    };

public:
    TagModel(QObject* parent = nullptr);
    virtual ~TagModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void BeginNewProfileSession(ProfilingSession* profSession);
    void SetCurrentValues(const MemoryStatItem& item);
    void SetTagColors(QColor colorActive, QColor colorInactive);

private:
    ProfilingSession* profileSession;

    DAVA::uint64 timestamp;
    DAVA::uint32 activeTags;
    DAVA::Vector<DAVA::TagAllocStat> curValues;
    QColor colors[2];
};
