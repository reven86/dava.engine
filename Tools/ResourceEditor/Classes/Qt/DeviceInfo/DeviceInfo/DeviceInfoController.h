#ifndef __DEVICEINFOCONTROLLER_H__
#define __DEVICEINFOCONTROLLER_H__


#include <QObject>
#include <QPointer>


class DeviceInfoWidget;


class DeviceInfoController
    : public QObject
{
    Q_OBJECT

public:
    explicit DeviceInfoController( QWidget *parentWidget, QObject *parent = NULL );
    ~DeviceInfoController();

private:
    void InitView();

    void DebugOutput();

    QPointer<DeviceInfoWidget> view;
    QPointer<QWidget> parentWidget;
};



#endif // __DEVICEINFOCONTROLLER_H__
