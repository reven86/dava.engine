#ifndef QTTOOLS_WIDGETMODEL_H
#define QTTOOLS_WIDGETMODEL_H


#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QPointer>
#include <QWidget>
#include <QSharedPointer>
POP_QT_WARNING_SUPRESSOR

#include "AbstractWidgetModel.h"

class WidgetItem;

class WidgetModel
: public AbstractWidgetModel
{
    friend class WidgetItem;

    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

private:
    using ItemCache = QMap<QWidget*, QSharedPointer<WidgetItem>>;

public:
    explicit WidgetModel(QWidget* w);
    ~WidgetModel();

    QWidget* widgetFromIndex(const QModelIndex& index) const override;
    QModelIndex indexFromWidget(QWidget* w) const override;

    // QAbstractItemModel
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    void rebuildCache();

    QSharedPointer<WidgetItem> root;
    ItemCache cache;

private:
    static void rebuildCacheRecursive(ItemCache& cache, const QSharedPointer<WidgetItem>& item);
};


#endif // QTTOOLS_WIDGETMODEL_H
