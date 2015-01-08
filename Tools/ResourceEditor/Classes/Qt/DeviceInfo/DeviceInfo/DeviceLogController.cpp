#include "DeviceLogWidget.h"
#include "DeviceLogController.h"

using namespace DAVA;
using namespace DAVA::Net;

DeviceLogController::DeviceLogController(const DAVA::Net::PeerDescription& peerDescr, QWidget *_parentWidget, QObject* parent)
    : QObject(parent)
    , parentWidget(_parentWidget)
    , peer(peerDescr)
{
    ShowView();
}

DeviceLogController::~DeviceLogController() {}

void DeviceLogController::ShowView()
{
    if (NULL == view)
    {
        const QString title = QString("%1 (%2 %3)")
            .arg(peer.GetName().c_str())
            .arg(peer.GetPlatformString().c_str())
            .arg(peer.GetVersion().c_str());

        view = new DeviceLogWidget(parentWidget);
        view->setWindowFlags(Qt::Window);
        view->setWindowTitle(title);

        connect(this, &QObject::destroyed, view, &QObject::deleteLater);
    }
    if (!view->isVisible())
        view->show();
}

void DeviceLogController::ChannelOpen()
{
    Output("************* Channel open");
}

void DeviceLogController::ChannelClosed()
{
    Output("************ Channel closed");
}

void DeviceLogController::PacketReceived(const void* packet, size_t length)
{
    String msg(static_cast<const char8*>(packet), length);
    Output(msg);
}

void DeviceLogController::Output(const String& msg)
{
    view->AppendText(msg.c_str());
}
