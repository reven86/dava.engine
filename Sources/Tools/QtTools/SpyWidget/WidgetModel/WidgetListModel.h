#ifndef QTTOOLS_WIDGETLISTMODEL_H
#define QTTOOLS_WIDGETLISTMODEL_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QWidgetList>
POP_QT_WARNING_SUPRESSOR

#include "AbstractWidgetModel.h"

class WidgetListModel
: public AbstractWidgetModel
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    explicit WidgetListModel(QObject* parent = nullptr);
    ~WidgetListModel();

    QWidget* widgetFromIndex(const QModelIndex& index) const override;
    QModelIndex indexFromWidget(QWidget* w) const override;
    void setWidgetList(const QWidgetList& widgetList = QWidgetList());

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private slots:
    void onWidgetDestroyed();

private:
    QWidgetList widgets;
};


#endif // QTTOOLS_WIDGETLISTMODEL_H
