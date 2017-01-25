#ifndef QTTOOLS_WIDGETSTATEHELPER_H
#define QTTOOLS_WIDGETSTATEHELPER_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
#include <QPointer>
POP_QT_WARNING_SUPRESSOR

class QWidget;
class QScreen;

class WidgetStateHelper
: public QObject
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    enum WidgetEvent
    {
        NoFlags = 0,
        MaximizeOnShowOnce = 1 << 0,
        ScaleOnDisplayChange = 1 << 1,
    };
    Q_DECLARE_FLAGS(WidgetEvents, WidgetEvent)

public:
    explicit WidgetStateHelper(QObject* parent = nullptr);
    ~WidgetStateHelper();

    void startTrack(QWidget* w);
    WidgetEvents getTrackedEvents() const;
    void setTrackedEvents(const WidgetEvents& events);

    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void stopTrack();
    void onShowEvent();
    void onScreenChanged(QScreen* screen);

private:
    QPointer<QWidget> trackedWidget;
    WidgetEvents trackedEvents;

    QPointer<QScreen> currentScreen;

public:
    static WidgetStateHelper* create(QWidget* w);
};

Q_DECLARE_METATYPE(WidgetStateHelper::WidgetEvent)
Q_DECLARE_METATYPE(WidgetStateHelper::WidgetEvents)
Q_DECLARE_OPERATORS_FOR_FLAGS(WidgetStateHelper::WidgetEvents)


#endif // QTTOOLS_WIDGETSTATEHELPER_H
