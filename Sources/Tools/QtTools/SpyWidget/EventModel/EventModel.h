#ifndef QTTOOLS_EVENTMODEL_H
#define QTTOOLS_EVENTMODEL_H


#include <QStandardItemModel>
#include <QEvent>




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
    explicit EventModel( QObject *parent = nullptr );
    ~EventModel() = default;

private:

private:
    static void build( QStandardItemModel& model );
};


Q_DECLARE_METATYPE( EventModel::Roles );


#endif // QTTOOLS_EVENTMODEL_H
