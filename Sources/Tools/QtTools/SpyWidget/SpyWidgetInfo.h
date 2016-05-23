#ifndef QTTOOLS_SPYWIDGETINFO_H
#define QTTOOLS_SPYWIDGETINFO_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
#include <QPointer>
#include <QModelIndex>
POP_QT_WARNING_SUPRESSOR

class QWidget;
class QTimer;

class SpyWidget;
class WidgetModel;
class WidgetHighlightModel;

class SpyWidgetInfo
: public QObject
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    explicit SpyWidgetInfo(QObject* parent = nullptr);
    ~SpyWidgetInfo();

    void trackWidget(QWidget* w);

    bool eventFilter(QObject* obj, QEvent* e) override;

public slots:
    void show();
    void updateInformation();

private slots:
    void onChangeWidget(const QModelIndex& index);
    void onSelectWidget();

private:
    void selectWidget(QWidget* w);

    QPointer<SpyWidget> view;
    QPointer<QWidget> widget;
    QPointer<QTimer> updateTimer;
    QPointer<WidgetModel> widgetModel;
    QPointer<WidgetHighlightModel> widgetHighlightModel;
};


#endif // QTTOOLS_SPYWIDGETINFO_H
