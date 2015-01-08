#ifndef __DEVICELOGWIDGET_H__
#define __DEVICELOGWIDGET_H__

#include <QWidget>
#include <QScopedPointer>

namespace Ui {
    class DeviceLogWidget;
} // namespace Ui

class DeviceLogWidget : public QWidget
{
    Q_OBJECT

signals:

public:
    explicit DeviceLogWidget(QWidget *parent = NULL);
    ~DeviceLogWidget();

    void AppendText(const QString& text);

private:
    QScopedPointer<Ui::DeviceLogWidget> ui;
};

#endif // __DEVICELOGWIDGET_H__
