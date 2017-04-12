#pragma once

#include <QWidget>
#include <QScopedPointer>

class QTreeView;

namespace Ui
{
class DeviceListWidget;
} // namespace Ui

class DeviceListWidget
: public QWidget
{
    Q_OBJECT
signals:

    void connectClicked();
    void disconnectClicked();
    void showLogClicked();
    void deviceDiscoverClicked(const QString& addr);

public slots:
    void OnViewDump();
    void OnDeviceDiscover();

public:
    explicit DeviceListWidget(QWidget* parent = NULL);
    ~DeviceListWidget();

    QTreeView* ItemView();

    QScopedPointer<Ui::DeviceListWidget> ui;
};
