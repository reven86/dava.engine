#ifndef QTTOOLS_CONTROLMAPPER_H
#define QTTOOLS_CONTROLMAPPER_H

#include "UI/UIEvent.h"
#include "QtTools/WarningGuard/QtWarningsHandler.h"

PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
#include <QPointer>
#include <QWindow>
POP_QT_WARNING_SUPRESSOR

class QWindow;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
class QDragMoveEvent;

class ControlMapper
: public QObject
{
    Q_OBJECT

public:
    explicit ControlMapper(QWindow* w);
    ~ControlMapper() = default;

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);

    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void mouseDoubleClickEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);

public slots:
    void releaseKeyboard();

private:
    DAVA::Vector<DAVA::UIEvent>& MapMouseEventToDAVA(const QPoint& pos, const Qt::MouseButtons button = Qt::NoButton, ulong timestamp = 0) const;
    static DAVA::Vector<DAVA::UIEvent::MouseButton>& MapQtButtonToDAVA(const Qt::MouseButtons button);
    QPointer<QWindow> window;
};



#endif // QTTOOLS_CONTROLMAPPER_H