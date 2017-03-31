#pragma once

#include <Base/BaseTypes.h>
#include <MemoryManager/MemoryManagerTypes.h>

#include <QAbstractTableModel>

class ProfilingSession;
class MemoryStatItem;

class GeneralStatModel : public QAbstractTableModel
{
public:
    enum
    {
        CLM_VALUE = 0,
        NCOLUMNS = 1
    };
    enum
    {
        ROW_ALLOC_INTERNAL = 0,
        ROW_ALLOC_INTERNAL_TOTAL,
        ROW_NBLOCKS_INTERNAL,
        ROW_ALLOC_GHOST,
        ROW_NBLOCKS_GHOST,
        ROW_TOTAL_ALLOC_COUNT,
        NROWS
    };

public:
    GeneralStatModel(QObject* parent = nullptr);
    virtual ~GeneralStatModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void BeginNewProfileSession(ProfilingSession* profSession);
    void SetCurrentValues(const MemoryStatItem& item);

private:
    ProfilingSession* profileSession;

    DAVA::uint64 timestamp;
    DAVA::GeneralAllocStat curValues;
};
