#pragma once


#include "TArc/WindowSubSystem/UI.h"
#include <QWidget>
#include <QMessageBox>

class QPropertyAnimation;
class QTimer;
class QPaintEvent;
class QPoint;
class QPushButton;

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
};

class NotificationWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(float opacity READ windowOpacity WRITE setWindowOpacity)
    Q_PROPERTY(QPoint position READ pos WRITE move)

public:
    explicit NotificationWidget(const NotificationWidgetParams& params, int displayTimeMs, QWidget* parent = 0);

    void SetPosition(const QPoint& point);
    void Init();

private slots:
    void Remove();
    void OnApplicationStateChanged(Qt::ApplicationState state);

private:
    void InitUI(const NotificationWidgetParams& params);
    void InitAnimations();
    void InitTimer();

    void paintEvent(QPaintEvent* event) override;

    QTimer* timer;
    QPushButton* closeButton = nullptr;
    QPushButton* detailsButton = nullptr;
    QPropertyAnimation* opacityAnimation = nullptr;
    QPropertyAnimation* positionAnimation = nullptr;

    int remainTimeMs = 0;
};
} //namespace TArc
} //namespace DAVA
