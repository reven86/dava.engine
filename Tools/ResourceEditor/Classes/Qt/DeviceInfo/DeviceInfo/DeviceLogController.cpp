#include <Utils/UTF8Utils.h>

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
    view->show();
    view->activateWindow();
    view->raise();
}

void DeviceLogController::ChannelOpen()
{
    Output("************* Connection open");
}

void DeviceLogController::ChannelClosed(const char8* message)
{
    String s("************ Connection closed: ");
    s += message;
    Output(s);
}

void DeviceLogController::PacketReceived(const void* packet, size_t length)
{
    String msg(static_cast<const char8*>(packet), length);
    Output(msg);
}

void DeviceLogController::Output(const String& msg)
{
    // Temporal workaround to extract log level from message
    QStringList list = QString(msg.c_str()).split(" ");
    Logger::eLogLevel ll = Logger::LEVEL_WARNING;
    // Current message format: <date> <time> <level> <text>
    if (list.size() > 3)
    {
        if (list[2] == "framwork")
            ll = Logger::LEVEL_FRAMEWORK;
        else if (list[2] == "debug")
            ll = Logger::LEVEL_DEBUG;
        else if (list[2] == "info")
            ll = Logger::LEVEL_INFO;
        else if (list[2] == "warning")
            ll = Logger::LEVEL_WARNING;
        else if (list[2] == "error")
            ll = Logger::LEVEL_ERROR;
    }
    view->AppendText(msg.c_str(), ll);
}
