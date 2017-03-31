#include "TArc/WindowSubSystem/Private/NotificationWidget.h"

#include <QString>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QPropertyAnimation>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QLabel>
#include <QPushButton>

#include <QPainter>
#include <Qt>

namespace NotificationWidgetDetails
{
QString ColorToHTML(const QColor& color)
{
    QString ret = QString("#%1%2%3%4")
                  .arg(color.alpha(), 2, 16, QChar('0'))
                  .arg(color.red(), 2, 16, QChar('0'))
                  .arg(color.green(), 2, 16, QChar('0'))
                  .arg(color.blue(), 2, 16, QChar('0'));
    return ret;
}

QPoint invalidPos(-1, -1);
} //namespace NotificationWidgetDetails

namespace DAVA
{
namespace TArc
{
NotificationWidget::NotificationWidget(const NotificationWidgetParams& params, QWidget* parent)
    : QWidget(parent)
    , remainTimeMs(params.showTimeMs)

{
    Qt::WindowFlags flags = (Qt::FramelessWindowHint | // Disable window decoration
                             Qt::Tool // Discard display in a separate window
                             );
    setWindowFlags(flags);

    setAttribute(Qt::WA_TranslucentBackground); // Indicates that the background will be transparent
    setAttribute(Qt::WA_ShowWithoutActivating); // At the show, the widget does not get the focus automatically

    InitUI(params);
    InitTimer();
    InitAnimations();

    QDesktopWidget* desktop = QApplication::desktop();
    QRect geometry = desktop->availableGeometry(parent);
    setFixedWidth(geometry.width() / 6);
    setMaximumHeight(geometry.height() / 5);

    connect(qApp, &QApplication::applicationStateChanged, this, &NotificationWidget::OnApplicationStateChanged);
}

void NotificationWidget::SetPosition(const QPoint& point)
{
    positionAnimation->stop();
    if (pos() == NotificationWidgetDetails::invalidPos)
    {
        move(point);
        return;
    }
    positionAnimation->setStartValue(pos());
    positionAnimation->setEndValue(point);
    positionAnimation->start();
}

void NotificationWidget::Add()
{
    show();

    opacityAnimation->setStartValue(0.0);
    opacityAnimation->setEndValue(1.0);
    opacityAnimation->start();
    move(NotificationWidgetDetails::invalidPos);
    if (qApp->applicationState() == Qt::ApplicationActive)
    {
        timer->start(remainTimeMs);
    }
}

void NotificationWidget::OnApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        timer->start(remainTimeMs);
    }
    else
    {
        if (timer->isActive())
        {
            remainTimeMs = timer->remainingTime();
            timer->stop();
        }
    }
}

void NotificationWidget::InitUI(const NotificationWidgetParams& params)
{
    QHBoxLayout* mainLayout = new QHBoxLayout();

    QVBoxLayout* messageLayout = new QVBoxLayout();
    mainLayout->addItem(messageLayout);

    QHBoxLayout* titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(10);
    messageLayout->addItem(titleLayout);

    QPixmap icon = QMessageBox::standardIcon(params.icon);
    QFont currentFont = font();
    QFontMetrics fm(currentFont);
    int fontHeight = fm.height();
    icon = icon.scaled(QSize(fontHeight, fontHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap(icon);
    iconLabel->setScaledContents(false);
    titleLayout->addWidget(iconLabel);

    if (params.title.isEmpty() == false)
    {
        QLabel* labelTitle = new QLabel(params.title);
        labelTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        labelTitle->setAlignment(Qt::AlignLeft);
        labelTitle->setWordWrap(true);
        labelTitle->setStyleSheet("font-weight: bold;");
        titleLayout->addWidget(labelTitle);
    }

    QLabel* labelMessage = new QLabel(params.text);
    messageLayout->addWidget(labelMessage);

    QPalette palette;
    QColor baseColor = palette.color(QPalette::Midlight);
    QColor buttonColor = baseColor;
    buttonColor.setAlpha(0);
    QColor pressedColor = baseColor;
    pressedColor.setAlpha(255);

    QString styleSheet = QString("QPushButton {"
                                 "border-radius: 1px;"
                                 "background-color: " +
                                 NotificationWidgetDetails::ColorToHTML(buttonColor) + ";"
                                                                                       "padding: 5px;"
                                                                                       "}"
                                                                                       "QPushButton:pressed {"
                                                                                       "background-color: " +
                                 NotificationWidgetDetails::ColorToHTML(pressedColor) + ";"
                                                                                        "}");

    QVBoxLayout* buttonsLayout = new QVBoxLayout();
    buttonsLayout->setSpacing(5);
    mainLayout->addItem(buttonsLayout);
    {
        closeButton = new QPushButton(tr("Close"));
        closeButton->setStyleSheet(styleSheet);
        closeButton->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
        buttonsLayout->addWidget(closeButton);
        connect(closeButton, &QPushButton::clicked, this, &NotificationWidget::Remove);
    }
    {
        detailsButton = new QPushButton(tr("Details"));
        detailsButton->setStyleSheet(styleSheet);
        detailsButton->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
        buttonsLayout->addWidget(detailsButton);
        connect(detailsButton, &QPushButton::clicked, params.callBack);
        connect(detailsButton, &QPushButton::clicked, this, &NotificationWidget::Remove);
    }
    setLayout(mainLayout);
}

void NotificationWidget::InitAnimations()
{
    positionAnimation = new QPropertyAnimation(this, "position", this);
    positionAnimation->setEasingCurve(QEasingCurve::OutExpo);
    positionAnimation->setDuration(150);

    opacityAnimation = new QPropertyAnimation(this, "opacity", this);
    opacityAnimation->setDuration(150);
}

void NotificationWidget::InitTimer()
{
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &NotificationWidget::Remove);
}

void NotificationWidget::Remove()
{
    timer->stop();
    opacityAnimation->stop();
    opacityAnimation->setStartValue(1.0);
    opacityAnimation->setEndValue(0.0);
    opacityAnimation->start();
    connect(opacityAnimation, &QAbstractAnimation::finished, this, &NotificationWidget::Removed);
}

void NotificationWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect roundedRect;
    roundedRect.setX(rect().x() + 5);
    roundedRect.setY(rect().y() + 5);
    roundedRect.setWidth(rect().width() - 10);
    roundedRect.setHeight(rect().height() - 10);

    QPalette palette;
    QColor rectColor = palette.color(QPalette::Window);
    painter.setBrush(QBrush(rectColor));
    painter.setPen(Qt::black);

    painter.drawRoundedRect(roundedRect, 10, 10);

    QRect closeButtonGeometry = closeButton->geometry();
    QRect detailsButtonGeometry = detailsButton->geometry();

    int y = (closeButtonGeometry.bottom() + detailsButtonGeometry.top()) / 2;
    QPoint left(qMin(closeButtonGeometry.left(), detailsButtonGeometry.left()), y);
    QPoint right(qMax(closeButtonGeometry.right(), detailsButtonGeometry.right()), y);

    QColor lineColor = palette.color(QPalette::Text);
    painter.setPen(lineColor);
    painter.drawLine(left, right);
}
} //namespace TArc
} //namespace DAVA
