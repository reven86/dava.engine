#pragma once


#include "TArc/WindowSubSystem/UI.h"
#include <QWidget>
#include <QMessageBox>

class QTimer;
class QPaintEvent;
class QPoint;

namespace DAVA
{
namespace TArc
{
struct NotificationWidgetParams
{
    QString text;
    QString title;
    QMessageBox::Icon icon;
    DAVA::Function<void(void)> callBack;
    int showTimeMs = std::numeric_limits<int>::max();
};

class NotificationWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(float opacity READ windowOpacity WRITE setWindowOpacity)
    Q_PROPERTY(QPoint position READ pos WRITE move)

public:
    explicit NotificationWidget(const NotificationWidgetParams& params, QWidget* parent = 0);

    void SetPosition(const QPoint& point);

signals:
    void Remove();

public slots:
    void Show();

private slots:
    void OnCloseClicked();

private:
    void Hide();

    void paintEvent(QPaintEvent* event);

    QTimer* timer;
};
} //namespace TArc
} //namespace DAVA
