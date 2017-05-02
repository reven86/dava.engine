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
class NotificationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPoint position READ pos WRITE move)
    Q_PROPERTY(float opacity READ windowOpacity WRITE setWindowOpacity)

public:
    explicit NotificationWidget(const NotificationParams& params, QWidget* parent = 0);

signals:
    void CloseButtonClicked(NotificationWidget* current);
    void DetailsButtonClicked(NotificationWidget* current);

private slots:
    void OnCloseButtonClicked();
    void OnDetailsButtonClicked();

private:
    void InitUI(const NotificationParams& params);

    void paintEvent(QPaintEvent* event) override;

    QPushButton* closeButton = nullptr;
    QPushButton* detailsButton = nullptr;
    const int32 radius = 10;
};
} //namespace TArc
} //namespace DAVA
