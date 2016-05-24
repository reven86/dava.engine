#ifndef QTTOOLS_SPYSEARCH_H
#define QTTOOLS_SPYSEARCH_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
#include <QPointer>
#include <QPoint>
POP_QT_WARNING_SUPRESSOR

class SpySearchView;
class WidgetListModel;

class SpySearch
: public QObject
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    explicit SpySearch(QObject* parent = nullptr);
    ~SpySearch();

    SpySearchView* GetView() const;

public slots:
    void show();

private slots:
    void trigger(const QPoint& pos);
    void onWidgetSelect(const QModelIndex& index);

private:
    bool isSelf(QWidget* w) const;
    void showWidgetInfo(QWidget* w) const;

    QPointer<SpySearchView> view;
    QPointer<WidgetListModel> resultModel;
};


#endif // QTTOOLS_SPYSEARCH_H
