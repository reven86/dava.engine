#ifndef __DEVICELISTCONTROLLER_H__
#define __DEVICELISTCONTROLLER_H__


#include <QObject>


class DeviceInfoController
    : public QObject
{
    Q_OBJECT

public:
    explicit DeviceInfoController( QObject *parent = NULL );
    ~DeviceInfoController();

private:
};



#endif // __DEVICELISTCONTROLLER_H__
