#ifndef QTTOOLS_SPYDRAGWIDGET_H
#define QTTOOLS_SPYDRAGWIDGET_H


#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QLabel>
#include <QPixmap>
#include <QCursor>
POP_QT_WARNING_SUPRESSOR

class SpyDragWidget
: public QLabel
{
private:
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

signals:
    void mousePressed();
    void mouseReleased(const QPoint& globalPos);

public:
    explicit SpyDragWidget(QWidget* parent = nullptr);
    ~SpyDragWidget();

private:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    void onMousePress();
    void onMouseRelease();

    QPixmap pix;
    QCursor cur;
};


#endif // QTTOOLS_SPYDRAGWIDGET_H
