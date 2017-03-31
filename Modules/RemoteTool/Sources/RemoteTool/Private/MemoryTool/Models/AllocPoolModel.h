#pragma once

#include <Base/BaseTypes.h>
#include <MemoryManager/MemoryManagerTypes.h>

#include <QColor>
#include <QAbstractTableModel>

class ProfilingSession;
class MemoryStatItem;

class AllocPoolModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum
    {
        CLM_NAME = 0,
        CLM_ALLOC_APP,
        CLM_ALLOC_TOTAL,
        CLM_NBLOCKS,
        NCOLUMNS = 4
    };

public:
    AllocPoolModel(QObject* parent = nullptr);
    virtual ~AllocPoolModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void BeginNewProfileSession(ProfilingSession* profSession);
    void SetCurrentValues(const MemoryStatItem& item);
    void SetPoolColors(const DAVA::Vector<QColor>& poolColors);

private:
    ProfilingSession* profileSession;

    DAVA::uint64 timestamp;
    DAVA::Vector<DAVA::AllocPoolStat> curValues;
    DAVA::Vector<QColor> poolColors;
};
