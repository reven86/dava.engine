#ifndef __LOGFILTERMODEL_H__
#define __LOGFILTERMODEL_H__

#include "Logger/Logger.h"

#include "QtTools/WarningGuard/QtWarningsHandler.h"

PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
#include <QSortFilterProxyModel>
#include <QFlags>
POP_QT_WARNING_SUPRESSOR

class LogFilterModel
: public QSortFilterProxyModel
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
public:
    explicit LogFilterModel(QObject* parent = nullptr);
    ~LogFilterModel();
public slots:
    void SetFilters(const QVariantList& filters);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    int filters = ~0;
};
#endif // __LOGFILTERMODEL_H__
