#ifndef __DEVICELISTWIDGET_H__
#define __DEVICELISTWIDGET_H__


#include <QWidget>
#include <QScopedPointer>

class QTreeView;


namespace Ui {
    class DeviceListWidget;
} // namespace Ui

class DeviceListWidget
    : public QWidget
{
    Q_OBJECT

signals:
    void connectClicked();
    void disconnectClicked();
    void showInfoClicked();

public:
    explicit DeviceListWidget( QWidget *parent = NULL );
    ~DeviceListWidget();

    QTreeView *ItemView();

private:
    QScopedPointer<Ui::DeviceListWidget> ui;
};



#endif // __DEVICELISTWIDGET_H__
