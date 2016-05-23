#ifndef QTTOOLS_ABSTRACTWIDGETMODEL_H
#define QTTOOLS_ABSTRACTWIDGETMODEL_H


#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QAbstractItemModel>
#include <QWidget>
POP_QT_WARNING_SUPRESSOR

class AbstractWidgetModel
: public QAbstractItemModel
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    enum Columns
    {
        TITLE,
        CLASSNAME,
        OBJECTNAME,

        COLUMN_COUNT // Last
    };

public:
    explicit AbstractWidgetModel(QObject* parent = nullptr);
    ~AbstractWidgetModel();

    virtual QWidget* widgetFromIndex(const QModelIndex& index) const = 0;
    virtual QModelIndex indexFromWidget(QWidget* w) const = 0;

    int columnCount(const QModelIndex& parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

protected:
    QVariant textDataForColumn(QWidget* w, int column) const;
};

Q_DECLARE_METATYPE(AbstractWidgetModel::Columns);


#endif // QTTOOLS_ABSTRACTWIDGETMODEL_H
