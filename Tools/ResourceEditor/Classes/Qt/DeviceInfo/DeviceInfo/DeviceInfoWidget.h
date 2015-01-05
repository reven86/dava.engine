#ifndef __DEVICELISTWIDGET_H__
#define __DEVICELISTWIDGET_H__


#include <QWidget>
#include <QScopedPointer>



namespace Ui {
    class DeviceInfoWidget;
} // namespace Ui

class DeviceInfoWidget
    : public QWidget
{
    Q_OBJECT

signals:

public:
    explicit DeviceInfoWidget( QWidget *parent = NULL );
    ~DeviceInfoWidget();

    void AppendText( const QString& text );

private:
    QScopedPointer<Ui::DeviceInfoWidget> ui;
};



#endif // __DEVICELISTWIDGET_H__
