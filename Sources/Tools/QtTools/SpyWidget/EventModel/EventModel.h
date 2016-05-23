#ifndef QTTOOLS_EVENTMODEL_H
#define QTTOOLS_EVENTMODEL_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"

PUSH_QT_WARNING_SUPRESSOR
#include <QStandardItemModel>
#include <QEvent>
POP_QT_WARNING_SUPRESSOR

class EventModel
: public QStandardItemModel
{
    Q_OBJECT

public:
    enum Roles
    {
        EVENT_TYPE = Qt::UserRole + 1,
    };

public:
    explicit EventModel(QObject* parent = nullptr);
    ~EventModel() = default;

private:
private:
    static void build(QStandardItemModel& model);
};

Q_DECLARE_METATYPE(EventModel::Roles);


#endif // QTTOOLS_EVENTMODEL_H
