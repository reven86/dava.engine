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
} //namespace NotificationWidgetDetails

DAVA::TArc::NotificationWidget::NotificationWidget(const DAVA::TArc::NotificationWidgetParams& params, QWidget* parent)
    : QWidget(parent)
{
    Qt::WindowFlags flags = (Qt::FramelessWindowHint | // Disable window decoration
                             Qt::Tool | // Discard display in a separate window
                             Qt::WindowStaysOnTopHint); // Set on top of all windows
    setWindowFlags(flags);

    setAttribute(Qt::WA_TranslucentBackground); // Indicates that the background will be transparent
    setAttribute(Qt::WA_ShowWithoutActivating); // At the show, the widget does not get the focus automatically

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    QVBoxLayout* messageLayout = new QVBoxLayout(this);
    mainLayout->addItem(messageLayout);

    QHBoxLayout* titleLayout = new QHBoxLayout(this);
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

    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    QString styleSheet = QString("QPushButton {"
                                 "border-radius: 5px;"
                                 "border: 1px solid #000000;"
                                 "background-color: " +
                                 NotificationWidgetDetails::ColorToHTML(buttonColor) + ";"
                                                                                       "padding: 5px;"
                                                                                       "}"
                                                                                       "QPushButton:pressed {"
                                                                                       "background-color: " +
                                 NotificationWidgetDetails::ColorToHTML(pressedColor) + ";"
                                                                                        "}");

    QVBoxLayout* buttonsLayout = new QVBoxLayout(this);
    mainLayout->addItem(buttonsLayout);
    {
        QPushButton* button = new QPushButton(tr("Close"));
        button->setStyleSheet(styleSheet);
        button->setSizePolicy(sizePolicy);
        buttonsLayout->addWidget(button);
        connect(button, &QPushButton::clicked, this, &NotificationWidget::OnCloseClicked);
    }
    {
        QPushButton* button = new QPushButton(tr("Details"));
        button->setStyleSheet(styleSheet);
        button->setSizePolicy(sizePolicy);
        buttonsLayout->addWidget(button);
        connect(button, &QPushButton::clicked, params.callBack);
    }

    timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(params.showTimeMs);
    connect(timer, &QTimer::timeout, this, &NotificationWidget::Hide);
    resize(sizeHint());
}

void DAVA::TArc::NotificationWidget::SetPosition(const QPoint& point)
{
    if (pos() == QPoint(-1, -1))
    {
        move(point);
        return;
    }
    QPropertyAnimation* animation = new QPropertyAnimation(this);
    animation->setEasingCurve(QEasingCurve::OutExpo);
    animation->setTargetObject(this);
    animation->setPropertyName("position");

    animation->setDuration(150); // Configuring the duration of the animation
    animation->setStartValue(pos()); // The start value is 0 (fully transparent widget)
    animation->setEndValue(point); // End - completely opaque widget
    animation->start();
}

void DAVA::TArc::NotificationWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect roundedRect;
    roundedRect.setX(rect().x() + 5);
    roundedRect.setY(rect().y() + 5);
    roundedRect.setWidth(rect().width() - 10);
    roundedRect.setHeight(rect().height() - 10);

    QPalette palette;
    QColor baseColor = palette.color(QPalette::Window);
    painter.setBrush(QBrush(baseColor));
    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(roundedRect, 10, 10);
}

void DAVA::TArc::NotificationWidget::Show()
{
    setWindowOpacity(1.0); // Set the transparency to zero

    QPropertyAnimation* animation = new QPropertyAnimation(this);
    animation->setTargetObject(this);
    animation->setPropertyName("opacity");

    animation->setDuration(150); // Configuring the duration of the animation
    animation->setStartValue(0.0); // The start value is 0 (fully transparent widget)
    animation->setEndValue(1.0); // End - completely opaque widget

    connect(animation, &QAbstractAnimation::finished, animation, &QObject::deleteLater);

    //     setGeometry(QApplication::desktop()->availableGeometry().width() - 36 - width() + QApplication::desktop()->availableGeometry().x(),
    //         QApplication::desktop()->availableGeometry().height() - 36 - height() + QApplication::desktop()->availableGeometry().y(),
    //         width(),
    //         height());
    QWidget::show();

    animation->start();
    timer->start();
    move(-1, -1);
}

void DAVA::TArc::NotificationWidget::OnCloseClicked()
{
    timer->stop();
    Hide();
}

void DAVA::TArc::NotificationWidget::Hide()
{
    timer->stop();

    QPropertyAnimation* animation = new QPropertyAnimation(this);
    animation->setTargetObject(this);
    animation->setPropertyName("opacity");

    animation->setDuration(150);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->start();
    connect(animation, &QAbstractAnimation::finished, this, &NotificationWidget::Remove);
}
